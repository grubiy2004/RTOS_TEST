#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

// Структуры, доступные другим модулям
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    int32_t t_fine;
} BMP280_Calib;

// Прототипы функций (что умеет этот модуль)
void BM280_Init(uint8_t osrs_t,uint8_t osrs_p,uint8_t mode,uint8_t t_sb,uint8_t filter,uint8_t spi3w_en);
double  BM280_Get_Temp(void);
void BMP280_Get_Calib(void);

#endif
