#include "usart.h"
#include "stm32f10x.h"

/** @brief Initialization of USART
 *  @note Only for USART1 and 9600 baudrate, for other configurations to change function
 */
void USART_Init(void) {
    USART1->BRR = 7500;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

// Передача по UART посимвольно
void USART_TX (uint8_t* dt, uint8_t usart_unit_by_user)		// Создаем указатель на переменную типа uint8 (в контексте строки - указатель - адрес 1-го символа)
{
    USART_TypeDef * usart_unit;

    switch (usart_unit_by_user) {
        case USART_UNIT_1:
            usart_unit = USART1;
            break;
        case USART_UNIT_2:
            usart_unit = USART2;
            break;
        case USART_UNIT_3:
            usart_unit = USART3;
            break;
        default:
            return;
    }

    while (*dt)     // Отправляем пока не встретим нулевой символ (разыменование)
    {
        while (!(usart_unit->SR & USART_SR_TXE)) {} // ждем когда выставится флаг полной передачи (сдвигоый регистр очистился)
			usart_unit->DR = *dt++; // вот здесь пиздец. сначала разыменование и передача в DR а только потом инкрементирование (без разыменования), потому что в постфиксе
    }
}

// Передача по UART посимвольно
void USART_RX (uint8_t* dt, uint8_t usart_unit_by_user)		// Создаем указатель на переменную типа uint8 (в контексте строки - указатель - адрес 1-го символа)
{
    USART_TypeDef * usart_unit;

    switch (usart_unit_by_user) {
        case USART_UNIT_1:
            usart_unit = USART1;
            break;
        case USART_UNIT_2:
            usart_unit = USART2;
            break;
        case USART_UNIT_3:
            usart_unit = USART3;
            break;
        default:
            return;
    }

    do {
        while (!(usart_unit->SR & USART_SR_RXNE)) {} // ждем когда выставится флаг полной передачи (сдвигоый регистр очистился)
		*dt = (uint8_t)usart_unit->DR ; // вот здесь пиздец. сначала разыменование и передача в DR а только потом инкрементирование (без разыменования), потому что в постфиксе
    } while (*dt++ != '\n');
}

