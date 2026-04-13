#include "filter.h"
#include <stdlib.h>
#include <math.h>

Filter* pf_create(int N, double x_min, double x_max){

    return NULL;
}

void pf_free(Filter* pf){
}

double gaussian_noise(double mean, double sigma) {
    static int has_spare = 0;
    static double spare;
    
    if (has_spare) {
        has_spare = 0;
        return mean + sigma * spare;
    }
    
    double u1 = ((double)rand() / RAND_MAX);
    double u2 = ((double)rand() / RAND_MAX);
    
    if (u1 < 1e-10) u1 = 1e-10;
    
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    double z1 = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);
    
    spare = z1;
    has_spare = 1;
    
    return mean + sigma * z0;
}