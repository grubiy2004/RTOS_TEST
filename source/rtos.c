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
#include "delay.h"

#define BIT_BAND(address,offset,bit) *((volatile uint32_t *) (((address) & 0xF0000000) + 0x02000000 + (((address) & 0x000FFFFF) + offset)*32 + bit*4))
#define BUF_ADC_SIZE    512

uint16_t buffer_adc[BUF_ADC_SIZE];
uint16_t second_buffer_adc[BUF_ADC_SIZE];
q15_t buffer_fft[BUF_ADC_SIZE*2];

TaskHandle_t fft_task, usart_task = NULL;

arm_rfft_instance_q15 * S;

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
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        //USART_TX(buffer_fft, USART_UNIT_1);
}

static void vFFT_Task(void *pvParameters) {
    (void) pvParameters;
    for(;;) {
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        for(int i = 0; i<BUF_ADC_SIZE; i++) {
            second_buffer_adc[i] = (second_buffer_adc[i] - 2048) << 4;
        }
        arm_rfft_q15(S,(q15_t *)second_buffer_adc,buffer_fft);
        xTaskNotifyGive(usart_task);
    }

}

void vApplicationIdleHook(void) {
    IWDG->KR = 0xAAAA;
    __WFI();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName ) {
    (void)xTask;
    (void)pcTaskName;
    for(;;) {
    }
}
/**
 * @brief   Инициализация тактирования и GPIO
 */

void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & DMA_ISR_HTIF1) {
        DMA1_Channel2->CCR |= DMA_CCR1_EN;
        DMA1->IFCR |= DMA_IFCR_CHTIF1;
    }
    else if (DMA1->ISR & DMA_ISR_TCIF1) {
        DMA1_Channel2->CNDTR = BUF_ADC_SIZE/2;
        DMA1_Channel2->CCR |= DMA_CCR2_EN | DMA_CCR2_TCIE;
        DMA1->IFCR |= DMA_IFCR_CTCIF1;
    }
    else if (DMA1->ISR & DMA_ISR_TEIF1) {
        //NVIC_SystemReset();
        //DMA1->IFCR |= DMA_IFCR_CTEIF1;
    }
}

void DMA1_Channel2_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    DMA1->IFCR |= DMA_IFCR_CTCIF2;
    vTaskNotifyGiveFromISR(fft_task,&xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void System_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_ADC1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_MODE13_1; // AF-output, 2 MHz, p-p (альт. выход для USART1 TX)
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF10_1; // input push-pull (вод для USART1 RX)

    GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
    GPIOA->CRL |= GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0_1; // AF-output, 2 MHz, p-p (альт. выход для TIM2 CH1)

    GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOB->CRH |= GPIO_CRH_CNF10_1 | GPIO_CRH_MODE13_1;  // // AF-output, 2 MHz (альт. выход (TIM3-CC1))
    GPIOB->BSRR = GPIO_BSRR_BR13;    // Изначально светодиод выключен

    GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);// Настраиваем PC13 как выход (push-pull, 2 MHz)
    GPIOC->CRH |= GPIO_CRH_MODE13_0;  // MODE13 = 01 (2MHz, выход)
    GPIOC->BSRR = GPIO_BSRR_BS13;    // Изначально светодиод выключен

    // ШИМ
    TIM2->PSC = 8000 - 1;
    TIM2->ARR = 10;
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;     // Очистили биты режима
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    TIM2->CCR1 = 5;                  // 50% заполнение
    TIM2->CCER |= TIM_CCER_CC1E;        // Включить выход
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;

    // Таймер 3 для триггера АЦП
    TIM3->CR2 &= ~TIM_CR2_MMS;  //The update event is selected as trigger output (TRGO).
    TIM3->CR2 |= TIM_CR2_MMS_1;
    TIM3->ARR = 3599;      // Update event 20 kHz
    TIM3->PSC = 0;
    TIM3->EGR |= TIM_EGR_UG;
    TIM3->SR &= ~TIM_SR_UIF;

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
    DMA1_Channel1->CNDTR = BUF_ADC_SIZE;
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR = (uint32_t)buffer_adc;
    DMA1_Channel1->CCR |= DMA_CCR1_EN;

    DMA1_Channel2->CCR &= ~DMA_CCR2_MSIZE;
    DMA1_Channel2->CCR |= DMA_CCR2_MSIZE_0; //16 bits
    DMA1_Channel2->CCR &= ~DMA_CCR2_PSIZE;
    DMA1_Channel2->CCR |= DMA_CCR2_PSIZE_0; //16 bits
    DMA1_Channel2->CCR |= DMA_CCR2_MINC | DMA_CCR2_PINC | DMA_CCR2_MEM2MEM | DMA_CCR2_DIR | DMA_CCR2_TCIE ;
    DMA1_Channel2->CNDTR = BUF_ADC_SIZE/2;
    DMA1_Channel2->CPAR = (uint32_t)buffer_adc;
    DMA1_Channel2->CMAR = (uint32_t)second_buffer_adc;

    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    arm_rfft_init_512_q15(S,0,1);

    ADC1->CR2 |= ADC_CR2_ADON;
    delay(100);
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL) {}
    __DMB();
    GPIOC->BSRR |= GPIO_BRR_BR13;
    TIM2->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief   Главная функция
 */
int main(void) {
    // Инициализация железа
    System_Init();
    USART_Init();
    xTaskCreate(
        vLED_Task,       // Указатель на функцию задачи
        "LED",           // Имя задачи (для отладки)
        128,             // Размер стека (в словах)
        NULL,            // Параметр задачи
        1,               // Приоритет (1 — низкий)
        NULL             // Хэндл задачи (не нужен)
    );

    xTaskCreate(vUSART_Task,"USART1",128,NULL,2,&usart_task);
    xTaskCreate(vFFT_Task,"FFT",1024, NULL, 3, &fft_task);
    vTaskStartScheduler();  // Запускаем планировщик FreeRTOS
    while (1) {
        // Бесконечный цикл на случай ошибки
    }
}
