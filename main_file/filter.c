#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "filter.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int check_filter(const Filter *pf, const char *func_name) {
    if (pf == NULL) {
        fprintf(stderr, "Ошибка: %s() получила NULL указатель на фильтр\n", 
                func_name);
        return 0;
    }
    if (pf->particles == NULL || pf->weights == NULL) {
        fprintf(stderr, "Ошибка: %s() - повреждённый фильтр (NULL массивы)\n", 
                func_name);
        return 0;
    }
    if (pf->N <= 0) {
        fprintf(stderr, "Ошибка: %s() - некорректное количество частиц %d\n", 
                func_name, pf->N);
        return 0;
    }
    return 1;
}

double gaussian_noise(double mean, double sigma) {
    static int has_spare = 0;
    static double spare;
    
    if (has_spare) {
        has_spare = 0;
        return mean + sigma * spare;
    }
    
    //преобразование Бокса-Мюллера
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    
    //(log(0) = -inf)
    if (u1 < 1e-10) {
        u1 = 1e-10;
    }
    
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    double z1 = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);
    
    spare = z1;
    has_spare = 1;
    
    return mean + sigma * z0;
}

Filter* pf_create(int N, double x_min, double x_max) {
    if (N <= 0) {
        fprintf(stderr, "Ошибка pf_create(): N=%d должно быть >0\n", N);
        return NULL;
    }
    if (x_min >= x_max) {
        fprintf(stderr, "Ошибка pf_create(): x_min=%.2f >= x_max=%.2f\n", 
                x_min, x_max);
        return NULL;
    }
    Filter *pf = (Filter*)malloc(sizeof(Filter));
    if (pf == NULL) {
        fprintf(stderr, "Ошибка pf_create()\n");
        return NULL;
    }
    
    pf->N = N;
    
    //памятm под массив частиц
    pf->particles = (double*)malloc(N * sizeof(double));
    if (pf->particles == NULL) {
        fprintf(stderr, "Ошибка pf_create(): не удалось выделить для particles\n");
        free(pf);
        return NULL;
    }
    
    pf->weights = (double*)malloc(N * sizeof(double));
    if (pf->weights == NULL) {
        fprintf(stderr, "Ошибка pf_create(): не удалось выделить для weights\n");
        free(pf->particles);
        free(pf);
        return NULL;
    }
    
    for (int i = 0; i < N; i++) {
        double r = (double)rand() / RAND_MAX;
        pf->particles[i] = x_min + r * (x_max - x_min);
        pf->weights[i] = 1.0 / N;
    }
    
    return pf;
}

void pf_free(Filter *pf) {
    if (pf == NULL) {
        return;
    }
    
    free(pf->particles);
    free(pf->weights);
    free(pf);
}


void predict(Filter *pf, double Q) {
    if (!check_filter(pf, "predict")) {
        return;
    }
    
    double sigma = sqrt(Q);  //cтандартное отклонение шума
    
    //модель движения: x_k = x_{k-1} + 1.0 + η_k где η_k ~ N(0, Q)
    for (int i = 0; i < pf->N; i++) {
        double noise = gaussian_noise(0.0, sigma);
        pf->particles[i] = pf->particles[i] + 1.0 + noise;
    }
}

void update_weights(Filter *pf, double z, double R) {
    if (!check_filter(pf, "update_weights")) {
        return;
    }
    
    //модель измерения: z = x + ε, где ε ~ N(0, R)
    //правдоподобие p(z|x) = exp(-0.5 * (z-x)² / R)
    for (int i = 0; i < pf->N; i++) {
        double diff = z - pf->particles[i];
        double likelihood = exp(-0.5 * diff * diff / R);
        pf->weights[i] = pf->weights[i] * likelihood;
    }
}

void normalize_weights(Filter *pf) {
    if (!check_filter(pf, "normalize_weights")) {
        return;
    }
    
    double sum = 0.0;
    for (int i = 0; i < pf->N; i++) {
        sum += pf->weights[i];
    }
    
    if (sum < 1e-10) {
        //если все веса практически нулевые, сбрасываем к равномерным
        for (int i = 0; i < pf->N; i++) {
            pf->weights[i] = 1.0 / pf->N;
        }
        return;
    }
    
    for (int i = 0; i < pf->N; i++) {
        pf->weights[i] /= sum;
    }
}

double estimate_position(const Filter *pf) {
    if (!check_filter(pf, "estimate_position")) {
        return 0.0;
    }
    
    double estimate = 0.0;
    for (int i = 0; i < pf->N; i++) {
        estimate += pf->particles[i] * pf->weights[i];
    }
    return estimate;
}

void resample(Filter *pf) {
    if (!check_filter(pf, "resample")) {
        return;
    }
    double *new_particles = (double*)malloc(pf->N * sizeof(double));
    double *new_weights = (double*)malloc(pf->N * sizeof(double));
    
    if (new_particles == NULL || new_weights == NULL) {
        fprintf(stderr, "Ошибка resample(): не удалось выделить временную память\n");
        free(new_particles);
        free(new_weights);
        return;
    }
    
    //сумма весов
    double *cumulative = (double*)malloc(pf->N * sizeof(double));
    if (cumulative == NULL) {
        fprintf(stderr, "Ошибка resample(): не удалось выделить память для cumulative\n");
        free(new_particles);
        free(new_weights);
        return;
    }
    
    cumulative[0] = pf->weights[0];
    for (int i = 1; i < pf->N; i++) {
        cumulative[i] = cumulative[i-1] + pf->weights[i];
    }
    
    //начальная случайная точка
    double u = ((double)rand() / RAND_MAX) / pf->N;
    int j = 0;
    
    // выборка
    for (int i = 0; i < pf->N; i++) {
        double threshold = u + (double)i / pf->N;
        while (threshold > cumulative[j] && j < pf->N - 1) {
            j++;
        }
        new_particles[i] = pf->particles[j];
        new_weights[i] = 1.0 / pf->N;
    }
    
    //замена старых массивов новыми
    free(pf->particles);
    free(pf->weights);
    free(cumulative);
    
    pf->particles = new_particles;
    pf->weights = new_weights;
}


int is_filter_valid(const Filter *pf) {
    if (pf == NULL) return 0;
    if (pf->particles == NULL || pf->weights == NULL) return 0;
    if (pf->N <= 0) return 0;
    
    for (int i = 0; i < pf->N; i++) {
        if (pf->weights[i] < 0 || isnan(pf->weights[i])) {
            return 0;
        }
    }
    
    return 1;
}

void save_to_csv(FILE *fp, int step, double true_pos, 
                 double measurement, double estimate) {
    if (fp == NULL) {
        fprintf(stderr, "Ошибка save_to_csv(): NULL указатель на файл\n");
        return;
    }
    fprintf(fp, "%d,%.6f,%.6f,%.6f\n", step, true_pos, measurement, estimate);
}

double compute_ess(const Filter *pf) {
    if (!check_filter(pf, "compute_ess")) {
        return 0.0;
    }
    
    double sum_sq = 0.0;
    for (int i = 0; i < pf->N; i++) {
        sum_sq += pf->weights[i] * pf->weights[i];
    }
    
    if (sum_sq < 1e-10) {
        return pf->N;
    }
    
    return 1.0 / sum_sq;
}