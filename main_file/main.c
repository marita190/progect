//  Симулирует движение робота по коридору от 0 до 100 метров.
//  Робот движется со скоростью ~1 м/с с шумом (проскальзывание колёс)
//  Датчик даёт зашумлённые измерения.
//  Фильтр частиц оценивает истинное положение робота.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "filter.h"


#define N_PARTICLES     100     /**< Количество частиц */
#define Q_PROCESS       0.5     /**< Дисперсия шума движения */
#define R_MEASUREMENT   3.0     /**< Дисперсия шума измерений */
#define X_MIN           0.0     /**< Минимальное начальное положение */
#define X_MAX           100.0   /**< Максимальное начальное положение */
#define TIME_STEPS      50      /**< Количество шагов симуляции */
#define RESAMPLE_THRESHOLD 0.5  /**< Порог ESS для ресемплирования (N/2) */

/*===========================================================================
 * ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ВЫВОДА
 *===========================================================================*/

/**
 * @brief Печатает заголовок таблицы результатов
 */
static void print_table_header(void) {
    printf("\n");
    printf("+------+--------+-----------+--------+----------+\n");
    printf("| Step | Truth  | Measured  | Filter | Error    |\n");
    printf("|      | (м)    | (м)       | (м)    | (м)      |\n");
    printf("+------+--------+-----------+--------+----------+\n");
}

/**
 * @brief Печатает строку таблицы результатов
 */
static void print_table_row(int step, double truth, double meas, 
                            double est, double error) {
    printf("| %4d | %6.2f | %9.2f | %6.2f | %8.2f |\n",
           step, truth, meas, est, error);
}

/**
 * @brief Печатает нижнюю границу таблицы
 */
static void print_table_footer(void) {
    printf("+------+--------+-----------+--------+----------+\n");
}

/**
 * @brief Вычисляет среднюю ошибку
 */
static double compute_average_error(double *errors, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += errors[i];
    }
    return sum / n;
}

/*===========================================================================
 * ГЛАВНАЯ ФУНКЦИЯ
 *===========================================================================*/

int main(int argc, char *argv[]) {
    /* Инициализация генератора случайных чисел */
    srand(time(NULL));
    
    printf("\n");
    printf("========================================================\n");
    printf("     ФИЛЬТР ЧАСТИЦ ДЛЯ ОТСЛЕЖИВАНИЯ РОБОТА\n");
    printf("========================================================\n");
    printf("\nПараметры симуляции:\n");
    printf("  - Частиц: %d\n", N_PARTICLES);
    printf("  - Шум движения (Q): %.1f\n", Q_PROCESS);
    printf("  - Шум измерений (R): %.1f\n", R_MEASUREMENT);
    printf("  - Шагов: %d\n", TIME_STEPS);
    
    Filter *pf = pf_create(N_PARTICLES, X_MIN, X_MAX);
    if (pf == NULL) {
        fprintf(stderr, "Ошибка: не удалось создать фильтр\n");
        return EXIT_FAILURE;
    }
    
    /* Открытие файла для сохранения результатов */
    FILE *output_file = fopen("output/output.csv", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Ошибка: не удалось создать output/output.csv\n");
        pf_free(pf);
        return EXIT_FAILURE;
    }
    
    /* Заголовок CSV файла */
    fprintf(output_file, "step,true_position,measurement,estimate\n");
    
    /* Переменные для симуляции */
    double x_true = 0.0;                    /* Истинное положение */
    double *errors = (double*)malloc(TIME_STEPS * sizeof(double));
    if (errors == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для errors\n");
        fclose(output_file);
        pf_free(pf);
        return EXIT_FAILURE;
    }
    
    print_table_header();
    
    /* Основной цикл симуляции */
    for (int t = 1; t <= TIME_STEPS; t++) {
        /* ===== СИМУЛЯЦИЯ РЕАЛЬНОГО МИРА ===== */
        /* Движение робота с шумом процесса */
        double process_noise = gaussian_noise(0.0, sqrt(Q_PROCESS));
        x_true = x_true + 1.0 + process_noise;
        
        /* Измерение датчика с шумом */
        double measurement_noise = gaussian_noise(0.0, sqrt(R_MEASUREMENT));
        double z = x_true + measurement_noise;
        
        /* ===== РАБОТА ФИЛЬТРА ===== */
        predict(pf, Q_PROCESS);                    /* Шаг 1: Предсказание */
        update_weights(pf, z, R_MEASUREMENT);      /* Шаг 2: Обновление весов */
        normalize_weights(pf);                      /* Шаг 3: Нормализация */
        double estimate = estimate_position(pf);   /* Шаг 4: Оценка */
        
        /* Адаптивное ресемплирование по ESS */
        double ess = compute_ess(pf);
        if (ess < N_PARTICLES * RESAMPLE_THRESHOLD) {
            resample(pf);
        }
        
        /* Сохранение результатов */
        double error = fabs(estimate - x_true);
        errors[t-1] = error;
        save_to_csv(output_file, t, x_true, z, estimate);
        
        /* Вывод таблицы (каждые 5 шагов) */
        if (t % 5 == 0 || t == 1) {
            print_table_row(t, x_true, z, estimate, error);
        }
    }
    
    print_table_footer();
    
    /* Статистика */
    double avg_error = compute_average_error(errors, TIME_STEPS);
    printf("\n");
    printf("========================================================\n");
    printf("СТАТИСТИКА:\n");
    printf("  - Средняя ошибка: %.3f м\n", avg_error);
    printf("  - Шум измерений:  σ = %.1f м (95%% ошибка ~ ±%.1f м)\n", 
           sqrt(R_MEASUREMENT), 2.0 * sqrt(R_MEASUREMENT));
    printf("\nРезультаты сохранены в output/output.csv\n");
    printf("Для визуализации запустите: python3 scripts/plot.py\n");
    printf("========================================================\n");
    
    /* Очистка ресурсов */
    free(errors);
    fclose(output_file);
    pf_free(pf);
    
    return EXIT_SUCCESS;
}