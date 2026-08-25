/**
 * @file    rtos.c
 * @brief   FreeRTOS мигалка для STM32F103C8T6 (BluePill)
 * @details Одна задача мигает светодиодом на PC13 с периодом 500 мс.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x.h"

/**
 * @brief   Задача мигания светодиодом
 * @param   pvParameters  Не используется
 */
static void vLED_Task(void *pvParameters) {
    (void)pvParameters;  // Подавляем предупреждение о неиспользуемом параметре

    for (;;) {
        // Включить светодиод (низкий уровень на PC13)
        GPIOC->BRR = GPIO_BRR_BR13;

        // Задержка 500 мс (используем системный тик FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(100));

        // Выключить светодиод (высокий уровень на PC13)
        GPIOC->BSRR = GPIO_BSRR_BS13;

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief   Инициализация тактирования и GPIO
 */
static void System_Init(void) {
    // Включаем тактирование порта C
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // Настраиваем PC13 как выход (push-pull, 2 MHz)
    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;  // MODE13 = 01 (2MHz, выход)

    // Изначально светодиод выключен
    GPIOC->BSRR = GPIO_BSRR_BS13;
}

/**
 * @brief   Главная функция
 */
int main(void) {
    // Инициализация железа
    System_Init();

    // Создаём задачу мигания
    // Стек 128 слов (~512 байт) — достаточно для простой задачи
    xTaskCreate(
        vLED_Task,       // Указатель на функцию задачи
        "LED",           // Имя задачи (для отладки)
        128,             // Размер стека (в словах)
        NULL,            // Параметр задачи
        1,               // Приоритет (1 — низкий)
        NULL             // Хэндл задачи (не нужен)
    );

    // Запускаем планировщик FreeRTOS
    vTaskStartScheduler();

    // Сюда мы никогда не попадём (если планировщик запустился)
    while (1) {
        // Бесконечный цикл на случай ошибки
    }
}
