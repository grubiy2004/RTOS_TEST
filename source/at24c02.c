#include "stm32f10x.h"
#include "usart_tx.h"
#include "at24c02.h"

// 2 Кбит. 32 страницы по 8 байт. Если записывается сверх одной страницы за раз, то байты перезаписываются по кругу (9-й вместо 1-го, 10-й вместо 2-го и т.д.)
// Между циклами записи НУЖНО подождать минимум 5 мс, иначе EEPROM не ответит ACKом

#define ADDR_BM 0x50

void AT24_Write_Byte(uint8_t word_address,uint8_t *data) { // Записать 1 байт по указанному адресу EEPROM (0x00..0xFF)
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(word_address);
	I2C_Write(*data);
	I2C_Stop();
}

void AT24_Write_Page(uint8_t word_address,uint8_t *buf) {	// Записать страницу (8 байт), нужно указать начальный адрес 1-го байта страницы (0x00,0x08 и т.д.). неиспользуемые будут 0x00
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(word_address);
	uint8_t count = 8;
	while(count--) {
		I2C_Write(*buf++);
	}
	I2C_Stop();	
}
	
void AT24_Read_Current_Byte(uint8_t *data) {			// Прочитать байт, следующий после байта, к которому в последний раз было обращение (в прошлом цикле читался один байт по адресу 0x01, значит здесь будет читаться сразу байт 0x02)
	I2C_Start();										// Не проверял
	I2C_Select(ADDR_BM,1);
	I2C_Read(data,0);
	I2C_Stop();
}

void AT24_Read_Byte(uint8_t word_address,uint8_t *data) {	// Прочитать 1 байт по указанному адресу
	I2C_Start();											// Не проверял
	I2C_Select(ADDR_BM,0);
	I2C_Write(word_address);
	I2C_Start();
	I2C_Select(ADDR_BM,1);
	I2C_Read(data,0);
	I2C_Stop();
}

void AT24_Read_Sequent(uint8_t word_address,uint8_t *buf, uint8_t count){	// Прочитать указанное количество байтов подряд при указании адреса первого байта. Чтение даже между страницами
	I2C_Start();
	I2C_Select(ADDR_BM,0);
	I2C_Write(word_address);
	I2C_Start();
	I2C_Select(ADDR_BM,1);
	while(count>1) {
		I2C_Read(buf++,1);
		count--; 
	}
	I2C_Read(buf,0);
	I2C_Stop();	
}
