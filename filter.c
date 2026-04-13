#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//гауссовский шум
double gaussian_noise(double mean, double sigma) {
    static int has_spare = 0;
    static double spare;
    
    if (has_spare) {
        has_spare = 0;
        return mean + sigma * spare;
    }
    
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    
    //от нуля
    if (u1 < 1e-10) u1 = 1e-10;
    
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    double z1 = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);
    
    spare = z1;
    has_spare = 1;
    
    return mean + sigma * z0;
}

//создание фильтра
Filter* pf_create(int N, double x_min, double x_max) {
    Filter* pf = (Filter*)malloc(sizeof(Filter));
    if (pf == NULL) {
        printf("ошибка памяти\n");
        return NULL;
    }
    
    pf->N = N;
    //память под массив частиц
    pf->particles = (double*)malloc(N * sizeof(double));
    if (pf->particles == NULL) {
        printf("ошибка: память для частиц\n");
        free(pf);
        return NULL;
    }
    
    //память под массив весов
    pf->weights = (double*)malloc(N * sizeof(double));
    if (pf->weights == NULL) {
        printf("ошибка: память для весов\n");
        free(pf->particles);
        free(pf);
        return NULL;
    }
    
    for (int i = 0; i < N; i++) {
        double r = (double)rand() / RAND_MAX;  //случайное число от 0 до 1
        pf->particles[i] = x_min + r * (x_max - x_min);
        pf->weights[i] = 1.0 / N;  //все веса равны
    }
    
    return pf;
}
void pf_free(Filter* pf) {
    if (pf == NULL) return;
    free(pf->particles);
    free(pf->weights);
    free(pf);
}

//шаг предсказания
void predict(Filter* pf, double Q) {
    double sigma = sqrt(Q);  //стандартное отклонение шума
    for (int i = 0; i < pf->N; i++) { //шум движения
        double noise = gaussian_noise(0.0, sigma);
        pf->particles[i] = pf->particles[i] + 1.0 + noise;//модель движения: робот едет 1 метр + шум
    }
}

//обновление весов
void update_weights(Filter* pf, double z, double R) {
}

//нормализация
void normalize_weights(Filter* pf) {
}

//оценка положения
double estimate_position(Filter* pf) {
    return 0.0;
}

//ресемплирование
void resample(Filter* pf) {
}