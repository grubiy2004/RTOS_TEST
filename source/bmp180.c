#include "stm32f10x.h"
#include "usart_tx.h"

#define ADDR_BM 0x76
#define REG_DIG_T1 0x88
#define REG_DIG_T2 0x8A
#define REG_DIG_T3 0x8C
#define REG_MSB_T 0xFA
#define REG_LSB_T 0xFB
#define REG_XLSB_T 0xFC
#define CONFIG 0xF5
#define CTRL_MEAS 0xF4
#define STATUS 0xF3
#define RESET 0xE0

typedef struct {			// Для калибровки
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
} BMP280_Calib;

BMP280_Calib cal;

void BM280_Init(uint8_t osrs_t,			// Оверсемплинг (сколько измерений усредняется) для температуры
				uint8_t osrs_p,			// Для давления
				uint8_t mode,			// Режим работы (11 - нормальный)
				uint8_t t_sb,			// Время измерения
				uint8_t filter,			// Кол-во коэфф. БИХ фильтра
				uint8_t spi3w_en) {		// SPI1 вкл/выкл
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(CTRL_MEAS);
					I2C_Write(osrs_t<<5 | osrs_p<<2 | mode);			// Записываем все установки микросхемы в два регисра ее
	I2C_Stop();
	
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(CONFIG);
	I2C_Write(t_sb<<5 | filter<<2 | spi3w_en);
	I2C_Stop();
}

void BMP280_Get_Calib(void) {			// Получить калибровочные числа (3 регистра по 2 байта (6 байт)) для точности вычислений температуры
    uint8_t buf[6];
    
    I2C_Start();
    I2C_Select(ADDR_BM, 0);
    I2C_Write(REG_DIG_T1);
    I2C_Start();
    I2C_Select(ADDR_BM, 1);
    I2C_Read(&buf[0], 1);
    I2C_Read(&buf[1], 1);
    I2C_Read(&buf[2], 1);
    I2C_Read(&buf[3], 1);
    I2C_Read(&buf[4], 1);
    I2C_Read(&buf[5], 0);
    I2C_Stop();
    
    cal.dig_T1 = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    cal.dig_T2 = (int16_t)((uint16_t)buf[2] | ((uint16_t)buf[3] << 8));
    cal.dig_T3 = (int16_t)((uint16_t)buf[4] | ((uint16_t)buf[5] << 8));
}

double  BM280_Get_Temp(void) {				// Получить значение температуры (3 байта)
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(REG_MSB_T);
	I2C_Start();
	I2C_Select(ADDR_BM,1);
	uint8_t temp_regs[3];
	I2C_Read(&temp_regs[0],1);
	I2C_Read(&temp_regs[1],1);
	I2C_Read(&temp_regs[2],0);
	I2C_Stop();
	
	int32_t adc_T = (int32_t)temp_regs[0]<<12 | (int32_t)temp_regs[1]<<4 | (int32_t)temp_regs[2];	// Вычисляем температуру с учетом калибровочных чисел
	double var1 = (((double)adc_T)/16384.0 - ((double)cal.dig_T1)/1024.0)*((double)cal.dig_T2);
	double var2 = ((((double)adc_T)/131072.0 - ((double)cal.dig_T1)/8192.0)*(((double)adc_T)/131072.0 - ((double)cal.dig_T1)/8192.0))*((double)cal.dig_T3);
	return (var1+var2)/5120.0;
}

