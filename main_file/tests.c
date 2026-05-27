#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "filter.h"

#define EPS 1e-6

void test_pf_create() {
    Filter *pf = pf_create(100, 0.0, 10.0);

    assert(pf != NULL);
    assert(pf->particles != NULL);
    assert(pf->weights != NULL);
    assert(pf->N == 100);

    for (int i = 0; i < pf->N; i++) {
        assert(pf->weights[i] > 0.0);
    }

    pf_free(pf);

    printf("test_pf_create passed\n");
}

void test_gaussian_noise() {
    for (int i = 0; i < 1000; i++) {
        double x = gaussian_noise(0.0, 1.0);

        assert(!isnan(x));
        assert(!isinf(x));
    }

    printf("test_gaussian_noise passed\n");
}

void test_normalize_weights() {
    Filter *pf = pf_create(3, 0.0, 1.0);

    pf->weights[0] = 2.0;
    pf->weights[1] = 3.0;
    pf->weights[2] = 5.0;

    normalize_weights(pf);

    double sum = 0.0;

    for (int i = 0; i < pf->N; i++) {
        sum += pf->weights[i];
    }

    assert(fabs(sum - 1.0) < EPS);

    pf_free(pf);

    printf("test_normalize_weights passed\n");
}

void test_estimate_position() {
    Filter *pf = pf_create(2, 0.0, 1.0);

    pf->particles[0] = 0.0;
    pf->particles[1] = 10.0;

    pf->weights[0] = 0.5;
    pf->weights[1] = 0.5;

    double est = estimate_position(pf);

    assert(fabs(est - 5.0) < EPS);

    pf_free(pf);

    printf("test_estimate_position passed\n");
}

void test_compute_ess() {
    Filter *pf = pf_create(4, 0.0, 1.0);

    pf->weights[0] = 0.25;
    pf->weights[1] = 0.25;
    pf->weights[2] = 0.25;
    pf->weights[3] = 0.25;

    double ess = compute_ess(pf);

    assert(fabs(ess - 4.0) < EPS);

    pf_free(pf);

    printf("test_compute_ess passed\n");
}

void test_predict() {
    Filter *pf = pf_create(10, 0.0, 1.0);

    double before = pf->particles[0];

    predict(pf, 0.1);

    double after = pf->particles[0];

    assert(before != after);

    pf_free(pf);

    printf("test_predict passed\n");
}

void test_resample() {
    Filter *pf = pf_create(4, 0.0, 1.0);

    pf->weights[0] = 0.9;
    pf->weights[1] = 0.05;
    pf->weights[2] = 0.03;
    pf->weights[3] = 0.02;

    normalize_weights(pf);

    resample(pf);

    for (int i = 0; i < pf->N; i++) {
        assert(fabs(pf->weights[i] - 0.25) < EPS);
    }

    pf_free(pf);

    printf("test_resample passed\n");
}

void test_is_filter_valid() {
    Filter *pf = pf_create(10, 0.0, 1.0);

    assert(is_filter_valid(pf) == 1);

    pf_free(pf);

    printf("test_is_filter_valid passed\n");
}
int main() {
    srand(time(NULL));

    test_pf_create();
    test_gaussian_noise();
    test_normalize_weights();
    test_estimate_position();
    test_compute_ess();
    test_predict();
    test_resample();
    test_is_filter_valid();

    printf("\nAll tests passed successfully!\n");

    return 0;
}
