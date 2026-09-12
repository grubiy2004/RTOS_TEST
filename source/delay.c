#ifndef DELAY_H
#define DELAY_H

#include "stm32f10x.h"
#include "delay.h"

void delay(uint32_t time) {
    volatile uint32_t count = time;
    while(count--) {
        __NOP();  // No Operation - предотвращает оптимизацию
    }
}

#endif
