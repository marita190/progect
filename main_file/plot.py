"""
Создаёт график сравнения истинной траектории, измерений и оценки фильтра
"""

import pandas as pd
import matplotlib.pyplot as plt
import os

def main():
    # Путь к файлу с результатами
    csv_path = "output/output.csv"
    
    if not os.path.exists(csv_path):
        print(f"Ошибка: файл {csv_path} не найден")
        print("Сначала запустите симуляцию: make run")
        return
    
    # Чтение данных
    df = pd.read_csv(csv_path)
    
    # Создание графика
    plt.figure(figsize=(14, 8))
    
    # Истинная траектория (чёрная линия)
    plt.plot(df['step'], df['true_position'], 
             'k-', linewidth=2, label='Истинное положение')
    
    # Шумные измерения (красные точки)
    plt.scatter(df['step'], df['measurement'], 
                c='red', s=20, alpha=0.5, marker='x', 
                label='Измерения (шумные)')
    
    # Оценка фильтра (синяя линия)
    plt.plot(df['step'], df['estimate'], 
             'b-', linewidth=2, label='Оценка фильтра частиц')
    
    # Оформление
    plt.xlabel('Время (шаги)', fontsize=12)
    plt.ylabel('Положение (метры)', fontsize=12)
    plt.title('Фильтр частиц: отслеживание робота в одномерном пространстве', 
              fontsize=14, fontweight='bold')
    plt.legend(fontsize=11)
    plt.grid(True, alpha=0.3)
    
    # Добавление текста с параметрами
    plt.text(0.02, 0.98, 
             f'Частиц: 100\nШум движения Q = 0.5\nШум измерений R = 3.0',
             transform=plt.gca().transAxes,
             verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.7))
    
    plt.tight_layout()
    
    # Сохранение графика
    plt.savefig('output/filter_result.png', dpi=150, bbox_inches='tight')
    print(f"✅ График сохранён в output/filter_result.png")
    
    # Показ графика
    plt.show()

if __name__ == "__main__":
    main()