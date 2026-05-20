#include <stdio.h>
#ifndef FILTER_H
#define FILTER_H

typedef struct{
    double* particles;  //массив частиц
    double* weights;    //массив весов
    int N;              //количество частиц
} Filter;

Filter* pf_create(int N, double x_min, double x_max);

void pf_free(Filter *pf);

double gaussian_noise(double mean, double sigma);

void predict(Filter *pf, double Q);

void update_weights(Filter *pf, double z, double R);

void normalize_weights(Filter *pf);

double estimate_position(const Filter *pf);

void resample(Filter *pf);

int is_filter_valid(const Filter *pf);

void save_to_csv(FILE *fp, int step, double true_pos, 
                 double measurement, double estimate);

double compute_ess(const Filter *pf);

#endif 