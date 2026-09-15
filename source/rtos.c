/**
 * @file    rtos.c
 * @brief   FreeRTOS мигалка для STM32F103C8T6 (BluePill)
 * @details Одна задача мигает светодиодом на PC13 с периодом 500 мс.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x.h"
#include "usart.h"
#include "arm_math.h"

#define BIT_BAND(address,offset,bit) *((volatile uint32_t *) (((address) & 0xF0000000) + 0x02000000 + (((address) & 0x000FFFFF) + offset)*32 + bit*4))

uint16_t * buffer_adc;

/**
 * @brief   Задача мигания светодиодом
 * @param   pvParameters  Не используется
 */
static void vLED_Task(void *pvParameters) {
    (void)pvParameters;  // Подавляем предупреждение о неиспользуемом параметре

    for (;;) {
        // Включить светодиод (низкий уровень на PC13)
        BIT_BAND(GPIOC_BASE,0x0C,13) = 0;
        // Задержка 500 мс (используем системный тик FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(1000));
        // Выключить светодиод (высокий уровень на PC13)
        BIT_BAND(GPIOC_BASE,0x0C,13) = 1;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vUSART_Task(void *pvParameters) {
    (void) pvParameters;
    uint8_t buffer_receive[40];
    for(;;){
        USART_TX(buffer_receive, USART_UNIT_1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationIdleHook(void) {
    IWDG->KR = 0xAAAA;
    __WFI();
}

/**
 * @brief   Инициализация тактирования и GPIO
 */
static void System_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_USART1EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_MODE13_1; // AF-output, 2 MHz (альт. выход для USART1 TX)
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF10_1; // input push-pull (вод для USART1 RX)

    GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOB->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_MODE13_1;  // // AF-output, 2 MHz (альт. выход (TIM1-CC1))
    GPIOB->BSRR = GPIO_BSRR_BR13;    // Изначально светодиод выключен

    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);// Настраиваем PC13 как выход (push-pull, 2 MHz)
    GPIOC->CRH |= GPIO_CRH_MODE13_0;  // MODE13 = 01 (2MHz, выход)
    GPIOC->BSRR = GPIO_BSRR_BS13;    // Изначально светодиод выключен

    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM1->CCR1 = 36;
    TIM1->ARR = 72;
    TIM1->PSC = 10000;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CR1 |= TIM_CR1_CEN;

    IWDG->KR = 0x5555;
    IWDG->PR = 2;
    IWDG->KR = 0xAAAA;
    IWDG->KR = 0xCCCC;

    ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_DMA;
    ADC1->CR2 &= ~ADC_CR2_EXTSEL;
    ADC1->CR2 |= ADC_CR2_EXTSEL_2;  //Timer 3 TRGO event
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP2;    //PA2 ADC12_IN2/
    ADC1->SMPR2 |= ADC_SMPR2_SMP2_1;    // 13.5 cycles
    ADC1->SQR1 &= ~ADC_SQR1_L;  // 1 conversation
    ADC1->SQR3 |= 2; //1st conversion in regular sequence is channel 2

    DMA1_Channel1->CCR &= ~DMA_CCR1_MSIZE;
    DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0; //16 bits
    DMA1_Channel1->CCR &= ~DMA_CCR1_PSIZE;
    DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0; //16 bits
    DMA1_Channel1->CCR |= DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_HTIE | DMA_CCR1_TCIE ;
    DMA1_Channel1->CNDTR = 512;
    DMA1_Channel1->CPAR = ADC1->DR;
    DMA1_Channel1->CMAR = buffer_adc;
    DMA1_Channel1->CCR |= DMA_CCR1_EN;

    ADC1->CR2 |= ADC_CR2_CAL;
    while(!(ADC1->CR2 & ADC_CR2_CAL)) {}
    GPIOC->BSRR |= GPIO_BRR_BR13;
    ADC1->CR2 |= ADC_CR2_ADON;
}

void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & DMA_ISR_HTIF1) {

        DMA1->IFCR &= ~DMA_IFCR_CHTIF1;
    }
    else if (DMA1->ISR & DMA_ISR_TCIF1) {

        DMA1->IFCR &= ~DMA_IFCR_CTCIF1;
    }
    else if (DMA1->ISR & DMA_ISR_TEIF1) {

        DMA1->IFCR &= ~DMA_IFCR_CTEIF1;
    }
}

/**
 * @brief   Главная функция
 */
int main(void) {
    // Инициализация железа
    System_Init();
    USART_Init();
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

    xTaskCreate(
        vUSART_Task,
        "USART1",
        128,
        NULL,
        2,
        NULL
    );

    // Запускаем планировщик FreeRTOS
    vTaskStartScheduler();

    // Сюда мы никогда не попадём (если планировщик запустился)
    while (1) {
        // Бесконечный цикл на случай ошибки
    }
}
