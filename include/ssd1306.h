#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f10x.h"

// Адрес SSD1306 на шине I2C (обычно 0x3C или 0x3D)
// SA0 = 0 -> 0x78 (с учетом сдвига, как у тебя в стиле)
// В твоей функции Select адрес сдвигается: (address << 1)
// Поэтому передаем адрес без сдвига: 0x3C << 1 = 0x78. В функцию нужно передать 0x3C.
#define SSD1306_ADDR 0x3C

// Размеры дисплея
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGES  8

// Команды управления
#define SSD1306_CMD_DISPLAY_OFF      0xAE
#define SSD1306_CMD_DISPLAY_ON       0xAF
#define SSD1306_CMD_SET_CONTRAST     0x81
#define SSD1306_CMD_NORMAL_DISPLAY   0xA6
#define SSD1306_CMD_INVERT_DISPLAY   0xA7
#define SSD1306_CMD_SET_MEM_MODE     0x20
#define SSD1306_CMD_SET_COL_ADDR     0x21
#define SSD1306_CMD_SET_PAGE_ADDR    0x22
#define SSD1306_CMD_SET_START_LINE   0x40
#define SSD1306_CMD_SEG_REMAP_0      0xA0
#define SSD1306_CMD_SEG_REMAP_1      0xA1
#define SSD1306_CMD_COM_SCAN_DEC     0xC8
#define SSD1306_CMD_COM_SCAN_INC     0xC0
#define SSD1306_CMD_SET_COM_PINS     0xDA
#define SSD1306_CMD_SET_MUX_RATIO    0xA8
#define SSD1306_CMD_SET_DISP_OFFSET  0xD3
#define SSD1306_CMD_SET_CLK_DIV      0xD5
#define SSD1306_CMD_SET_PRECHARGE    0xD9
#define SSD1306_CMD_SET_VCOMH        0xDB
#define SSD1306_CMD_CHARGE_PUMP      0x8D

// Функции
void SSD1306_Init(void);
void SSD1306_SendCommand(uint8_t cmd);
void SSD1306_SendData(uint8_t data);
void SSD1306_SetCursor(uint8_t page, uint8_t col);
void SSD1306_Clear(void);
void SSD1306_UpdateFull(uint8_t *buffer);
void SSD1306_SetContrast(uint8_t value);
void SSD1306_DisplayOn(void);
void SSD1306_DisplayOff(void);

#endif