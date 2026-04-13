#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "filter.h"

int main() {
    srand(time(NULL));
    
    
    //фильтр 5 частиц от 1 до 100 метров
    Filter* pf = pf_create(5, 0.0, 100.0);
    int temp[6];
    //до предсказания
    for (int i = 1; i < pf->N; i++) {
        temp[i] = pf->particles[i];
        printf("частица %d: положение = %.2f м\n", i, pf->particles[i]);
    }
    
    //шум движения Q = 0.5
    printf("\nприменение predict() с Q = 0.5\n");
    predict(pf, 0.5);
    
    //частицы после предсказания
    for (int i = 0; i < pf->N; i++) {
        printf("частица %d: положение = %.2f м\n", i, pf->particles[i]);
    }
    
    //сдвиг какой
    for (int i = 0; i < pf->N; i++) {
        printf("частица %d изменила положение на %f\n", i, pf->particles[i] - temp[i]);
    }
    pf_free(pf);
    
    return 0;
}