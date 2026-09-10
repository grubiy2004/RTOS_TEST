/**
 * @file    rtos.c
 * @brief   FreeRTOS мигалка для STM32F103C8T6 (BluePill)
 * @details Одна задача мигает светодиодом на PC13 с периодом 500 мс.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x.h"
#include "usart_tx.h"

#define BIT_BAND(address,offset,bit) *((volatile uint32_t *) (((address) & 0xF0000000) + 0x02000000 + (((address) & 0x000FFFFF) + offset)*32 + bit*4))

uint8_t flag = 0;

/**
 * @brief   Задача мигания светодиодом
 * @param   pvParameters  Не используется
 */
static void vLED_Task(void *pvParameters) {
    (void)pvParameters;  // Подавляем предупреждение о неиспользуемом параметре

    for (;;) {
        // Включить светодиод (низкий уровень на PC13)
        //GPIOC->BRR = GPIO_BRR_BR13;
        BIT_BAND(GPIOC_BASE,0x0C,13) = 0;
        // Задержка 500 мс (используем системный тик FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(1000));
        // for(int i=0;i<10000000;i++) {
        //     __ASM volatile("nop");
        // }

        // Выключить светодиод (высокий уровень на PC13)
        BIT_BAND(GPIOC_BASE,0x0C,13) = 1;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationIdleHook(void) {
    IWDG->KR = 0xAAAA;
    WWDG->CR |= 0x7F;
    __WFI();
}

/**
 * @brief   Инициализация тактирования и GPIO
 */
static void System_Init(void) {
    // Включаем тактирование порта C
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB1ENR |= RCC_APB1ENR_WWDGEN;

    // Настраиваем PC13 как выход (push-pull, 2 MHz)
    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;  // MODE13 = 01 (2MHz, выход)

    // Изначально светодиод выключен
    GPIOC->BSRR = GPIO_BSRR_BS13;

    IWDG->KR = 0x5555;
    IWDG->PR = 2;
    IWDG->KR = 0xAAAA;
    IWDG->KR = 0xCCCC;

    WWDG->CR |= WWDG_CR_WDGA;
    WWDG->CFR |= WWDG_CFR_EWI;

    NVIC_EnableIRQ(WWDG_IRQn);
}

void WWDG_IRQHandler(void) {
    flag = 1;
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
