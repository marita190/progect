#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "filter.h"

int main() {
    srand(time(NULL));
    
    //генерация чисел и считаем среднее
    double sum = 0.0;
    for (int i = 0; i < 1000; i++) {
        sum += gaussian_noise(0.0, 1.0);
    }
    double mean = sum / 1000.0;
    printf("%f\n", mean);
    
    return 0;
}