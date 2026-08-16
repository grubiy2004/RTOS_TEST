/**
 * @file    rtos.c
 * @brief   FreeRTOS application for STM32F103C8T6
 * @details This file contains the main application entry point and all RTOS tasks.
 *          It demonstrates usage of FreeRTOS queues, task notifications, and
 *          peripheral drivers for temperature sensor, I2C EEPROM, OLED display,
 *          and USART communicatio
 * 
 * @author  Your Name
 * @date    August 2026
 * @version 1.0
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h" 
#include "stm32f10x.h"                
#include <stdio.h>
#include <string.h>
#include "usart_tx.h"
#include "bmp180.h"
#include "at24c02.h"
#include "ssd1306.h"
#include "EventRecorder.h"
#include "EventRecorderConf.h"

/**
 * @defgroup Queues Queue Handles
 * @brief Global queue handles used for inter-task communication
 * @{
 */

/** @brief Queue handle for temperature data from BMP280 task to USART task */
QueueHandle_t xQueue_Temp;

/** @brief Task handle for LED task notification from EXTI interrupt */
TaskHandle_t xNotify_EXTI = NULL;

/** @brief Variable to store minimum free heap size for debugging */
uint32_t HeapSize;

/** @} */

/**
 * @brief   Monitor task - displays task list via USART
 * @param   pvParameters Unused parameter
 * @details This task periodically calls vTaskList() to get information about
 *          all running tasks and displays it. It runs at the lowest priority (0).
 * @note    Runs every 500ms
 */
void vMonitorTask(void *pvParameters) {
    (void)pvParameters;
    static char cBuffer[1024];
	
    for(;;) {
        memset(cBuffer, 0, 1024);   /**< Clear buffer before writing task list */
        vTaskList(cBuffer);        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief   BMP280/BMP180 temperature sensor task
 * @param   pvParameters Unused parameter
 * @details Reads temperature from BMP280 sensor every 2ms and sends the result
 *          to the USART task via queue. Also updates the minimum free heap size.
 * @note    Priority 2
 */
void vBM280Task(void *pvParameters) {
    (void)pvParameters;

    for(;;) {
		double res = BM280_Get_Temp();
		xQueueSend(xQueue_Temp,&res,0);
		HeapSize = xPortGetMinimumEverFreeHeapSize();
		vTaskDelay(pdMS_TO_TICKS(2));
    }
}

/**
 * @brief   USART communication task
 * @param   pvParameters Unused parameter
 * @details Receives temperature data from queue and sends it via USART1.
 *          Uses Event Recorder for performance measurement.
 * @note    Priority 1
 */
void vUSARTTask(void *pvParameters) {
	(void)pvParameters;
	double ext_temp;
	char buf[30];
	for(;;) {
		EventStartA(0);                                 /**< Start Event Recorder measurement */
		xQueueReceive(xQueue_Temp,&ext_temp,0);
		sprintf(buf,"%.2f",ext_temp);
		USART1_TX((uint8_t*)0xFF);
		while(!(USART1->SR & USART_SR_TC));            /**< Wait for transmission complete */
		EventStopA(0);                                  /**< Stop Event Recorder measurement */
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

/**
 * @brief   LED control task triggered by external interrupt
 * @param   pvParameters Unused parameter
 * @details Waits for task notification from EXTI0 interrupt handler.
 *          Toggles PC13 LED with debounce protection (10ms).
 * @note    Priority 3 - highest among application tasks
 */
void vLED_EXTITask(void *pvParameters) {
	(void)pvParameters;
	static TickType_t last_call_time = 0;
    TickType_t current_time;
	for(;;) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        current_time = xTaskGetTickCount();
        /** @brief Debounce: ignore interrupts if less than 10ms since last toggle */
        if ((current_time - last_call_time) >= pdMS_TO_TICKS(10)) {
            GPIOC->ODR ^= GPIO_ODR_ODR13;   /**< Toggle LED on PC13 */
            last_call_time = current_time;
        }
	}
}

/**
 * @brief   Initialize system clocks and peripherals
 * @details Configures PLL (72 MHz), enables clocks for GPIOA, GPIOB, GPIOC,
 *          USART1 and I2C1. Sets up HSE as system clock.
 * @note    Called once at system startup
 */
static void RCC_Init(void) {
	RCC->CR = RCC_CR_PLLON | 0x83;      /**< Enable HSE and PLL */
	RCC->CFGR = 7<<18 | RCC_CFGR_PPRE1_2 | 2;  /**< PLL x9, APB1 prescaler = /2 */
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_USART1EN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
}

/**
 * @brief   Initialize LED on PC13
 * @details Configures PC13 as push-pull output and sets LED to OFF state.
 * @note    PC13 is connected to the built-in LED on most STM32F103 boards
 */
static void LED_Init(void) {
	GPIOC->CRH = 0x44144444;	
    GPIOC->BSRR = GPIO_BSRR_BS13;       /**< Set PC13 high (LED OFF on many boards) */
}

/**
 * @brief   Initialize USART1 for serial communication
 * @details Configures PA9 as TX (alternate function output) and PA10 as RX (input).
 *          Sets baud rate to 9600 (with 72 MHz system clock).
 * @note    Baud rate calculation: 72000000 / 16 / 9600 = 468.75 ≈ 469
 */
static void USART_Init(void) {
	GPIOA->CRH = 0x444448B4;            /**< PA9: AF push-pull, PA10: input floating */
	USART1->CR1 = USART_CR1_UE;         /**< Enable USART1 */
	USART1->BRR = 7500;                 /**< 9600 baud @ 72MHz (7500 = 72000000 / 16 / 9600) */
	USART1->CR1 |= USART_CR1_TE;        /**< Enable transmitter */
	USART1->CR2 = 0;
	USART1->CR3 = 0;
}

/**
 * @brief   Initialize I2C1 for communication with peripherals
 * @details Configures PB6 as SCL and PB7 as SDA for I2C1.
 *          Sets up Fast Mode (400 kHz) with 36 MHz APB1 clock.
 * @note    I2C clock = 400 kHz, calculated from APB1 (36 MHz)
 */
static void I2C_Init(void) {
	GPIOB->CRL = 0xFF444444;            /**< PB6 - SCL, PB7 - SDA (AF open-drain) */
	I2C1->CR2 = 36;                     /**< APB1 frequency = 36 MHz */
	I2C1->TRISE = 12;                   /**< 300ns rise time max */
	I2C1->CCR |= 1<<15 | 30;            /**< Fast Mode (400kHz) */
	I2C1->OAR1 = 0;
	I2C1->OAR2 = 0;					
	I2C1->CR1 |= I2C_CR1_PE;            /**< Enable I2C1 */
	delay(10000);                       /**< Wait for I2C to stabilize */
}

/**
 * @brief   Initialize EXTI0 interrupt on PA0
 * @details Configures PA0 as input with falling edge trigger.
 *          Enables EXTI0 interrupt in NVIC.
 * @note    Used to trigger LED task via task notification
 */
static void EXTI0_Init(void) {
	GPIOA->CRL = (GPIOA->CRL & ~0xF) | 0x4;  /**< PA0 as input floating */
	EXTI->IMR |= EXTI_IMR_MR0;              /**< Enable interrupt on line 0 */
	EXTI->FTSR |= EXTI_FTSR_TR0;            /**< Falling edge trigger */
	NVIC_EnableIRQ(EXTI0_IRQn);             /**< Enable EXTI0 interrupt in NVIC */
}

/**
 * @brief   Main entry point
 * @details Initializes hardware peripherals, creates RTOS tasks,
 *          and starts the FreeRTOS scheduler.
 * @note    Should never return - infinite loop if scheduler fails
 */
int main(void) {
	RCC_Init();
	LED_Init();
	USART_Init();
	I2C_Init();
	EXTI0_Init();
	BM280_Init(1,0,3,0,0,0);
	EventRecorderInitialize(EventRecordAll,1);
	BMP280_Get_Calib();
	
	/** @brief Create queue for temperature data (1 item of double size) */
	xQueue_Temp = xQueueCreate(1, sizeof(double));
	
	/** @brief Create RTOS tasks */
	xTaskCreate(vBM280Task, "BMP280", 100, NULL, 2, NULL);       /**< Temperature sensor task */
    xTaskCreate(vUSARTTask, "USART1", 256, NULL, 1, NULL);       /**< USART communication task */
	xTaskCreate(vLED_EXTITask, "LED", 128, NULL, 3, &xNotify_EXTI); /**< LED control task */
	xTaskCreate(vMonitorTask, "Monitor", 256, NULL, 0, NULL);    /**< Debug monitor task */
	
    /** @brief Start RTOS scheduler */
    vTaskStartScheduler();
    
    /** @brief Should never reach this point */
    for(;;);
}

/**
 * @brief   EXTI0 interrupt handler
 * @details Clears the interrupt flag and notifies the LED task.
 *          Uses task notification from ISR to wake up vLED_EXTITask.
 * @note    Called automatically on falling edge of PA0
 */
void EXTI0_IRQHandler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	EXTI->PR = EXTI_PR_PR0;     /**< Clear interrupt pending bit */
	vTaskNotifyGiveFromISR(xNotify_EXTI, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}