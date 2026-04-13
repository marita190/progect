#ifndef FILTER_H
#define FILTER_H

typedef struct{
    double* particles;  //массив частиц
    double* weights;    //массив весов
    int N;              //количество частиц
} Filter;

Filter* pf_create(int N, double x_min, double x_max);
void pf_free(Filter* pf);

double gaussian_noise(double mean, double sigma);

#endif