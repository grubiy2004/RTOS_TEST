/**
 * @file ssd1306.c
 * @author Me
 * @brief Functions for work with LCD SSD1306 via I2C
 */

#include "ssd1306.h"
#include "usart.h"
#include "i2c.h"
#include "delay.h"

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/** @brief Transmit command
 * @param cmd Command (1 byte), see in @ref ssd1306.h
*/
static void SSD1306_WriteCmd(uint8_t cmd) {
    I2C_Start();
    I2C_Select(SSD1306_ADDR, 0); // Запись
    I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
    I2C_Write(cmd);
    I2C_Stop();
}

/** @brief Transmit data
 * @param data data buffer
 * @param len length of buffer data
 */
static void SSD1306_WriteData(uint8_t *data, uint16_t len) {
    I2C_Start();
    I2C_Select(SSD1306_ADDR, 0); // Запись
    I2C_Write(0x40);             // Control Byte: Co=0, D/C#=1 (Data)
    while(len--) {
        I2C_Write(*data++);
    }
    I2C_Stop();
}

/** @brief Initialization of display */
void SSD1306_Init(void) {
    // Ждем стабилизации питания (в твоем стиле, delay уже реализован)
    delay(10000);
    // Последовательность инициализации согласно даташиту
	I2C_Start();
	I2C_Select(SSD1306_ADDR, 0); // Запись
	I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
	I2C_Write(SSD1306_CMD_DISPLAY_OFF);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_MUX_RATIO);
	I2C_Write(0x3F);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_DISP_OFFSET);
	I2C_Write(0x00);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_START_LINE | 0x00);
	I2C_Write(0x00);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SEG_REMAP_1);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_COM_SCAN_DEC);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_COM_PINS);
	I2C_Write(0x12);

    	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_CONTRAST);
	I2C_Write(0xFF);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_NORMAL_DISPLAY);

    	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_CLK_DIV);
	I2C_Write(0x80);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_CHARGE_PUMP);
	I2C_Write(0x14);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_MEM_MODE);
	I2C_Write(0x00);

	SSD1306_Clear();

	I2C_Start();
	I2C_Select(SSD1306_ADDR, 0); // Запись
	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_DISPLAY_ON);

	I2C_Stop();
}
/** @brief Clear buffer and display */
void SSD1306_Clear(void) {
    for(uint16_t i = 0; i < sizeof(ssd1306_buffer); i++) {
        ssd1306_buffer[i] = 0x00;
    }

    // Устанавливаем окно на весь экран
	I2C_Start();
	I2C_Select(SSD1306_ADDR, 0); // Запись
	I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
	I2C_Write(SSD1306_CMD_SET_COL_ADDR);
	I2C_Write(0x00);
	I2C_Write(0x7F);

    	I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
	I2C_Write(SSD1306_CMD_SET_PAGE_ADDR);
	I2C_Write(0x00);
	I2C_Write(0x07);

    // Отправляем пустые данные
    SSD1306_WriteData(ssd1306_buffer, sizeof(ssd1306_buffer));
}

/** @brief Full display updating from buffer
 * @param buffer Array of pixels
*/
void SSD1306_UpdateFull(uint8_t *buffer) {
    // Копируем во внутренний буфер
    for(uint16_t i = 0; i < sizeof(ssd1306_buffer); i++) {
        ssd1306_buffer[i] = buffer[i];
    }
    	I2C_Start();
	I2C_Select(SSD1306_ADDR, 0); // Запись

	I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
	I2C_Write(SSD1306_CMD_SET_COL_ADDR);
	I2C_Write(0x00);
	I2C_Write(0x7F);

	I2C_Write(0x00);
	I2C_Write(SSD1306_CMD_SET_PAGE_ADDR);
	I2C_Write(0x00);
	I2C_Write(0x07);

    // Отправляем данные
    SSD1306_WriteData(ssd1306_buffer, sizeof(ssd1306_buffer));
}

/** @brief Enable display */
void SSD1306_DisplayOn(void) {
    SSD1306_WriteCmd(SSD1306_CMD_DISPLAY_ON);
}

/** @brief Disable display */
void SSD1306_DisplayOff(void) {
    SSD1306_WriteCmd(SSD1306_CMD_DISPLAY_OFF);
}
