#ifndef USART_H
#define USART_H

#include <stdint.h>

void USART_Init(void);
void USART_TX (uint8_t* dt, uint8_t usart_unit_by_user)	;
void USART_RX (uint8_t* dt, uint8_t usart_unit_by_user)	;

#define USART_UNIT_1 1
#define USART_UNIT_2 2
#define USART_UNIT_3 3

#define TX 1
#define RX 2

#endif
