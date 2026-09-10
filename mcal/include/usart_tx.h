#ifndef USART_H
#define USART_H

#include <stdint.h>

// sdjvjdsfkjdsfdskjfdsalkjfksajdfnkdsajnkdsajnds,vnds,vnds,fvj
//sdfdsfdsdsaf/asdf
//adfsadgsdgdsdfsfds

void delay(uint32_t time);
void USART1_TX (uint8_t* dt);
void I2C_Reset(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Select(uint8_t address, uint8_t readBit);
void I2C_Write(uint8_t data);
uint8_t I2C_Read(uint8_t * byte, uint8_t ack);
void I2C1_Busy_Errata(void);

#endif
