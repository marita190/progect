#ifndef FILTER_H
#define FILTER_H

//фильтр частиц
typedef struct {
    double* particles;   //массив положений частиц
    double* weights;     //массив весов частиц
    int N;               //количество частиц
} Filter;


//шенерация гауссовского шума
double gaussian_noise(double mean, double sigma);

Filter* pf_create(int N, double x_min, double x_max);
void pf_free(Filter* pf);

//сам алгоритм
void predict(Filter* pf, double Q);
void update_weights(Filter* pf, double z, double R);
void normalize_weights(Filter* pf);
double estimate_position(Filter* pf);
void resample(Filter* pf);

#endif