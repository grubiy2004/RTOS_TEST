#include "ssd1306.h"
#include "usart_tx.h"

// Внутренний буфер кадра (1024 байта = 128 * 64 / 8)
static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Статическая функция для отправки пакета "команда"
static void SSD1306_WriteCmd(uint8_t cmd) {
    I2C_Start();
    I2C_Select(SSD1306_ADDR, 0); // Запись
    I2C_Write(0x00);             // Control Byte: Co=0, D/C#=0 (Command)
    I2C_Write(cmd);
    I2C_Stop();
}

// Статическая функция для отправки пакета "данные"
static void SSD1306_WriteData(uint8_t *data, uint16_t len) {
    I2C_Start();
    I2C_Select(SSD1306_ADDR, 0); // Запись
    I2C_Write(0x40);             // Control Byte: Co=0, D/C#=1 (Data)
    while(len--) {
        I2C_Write(*data++);
    }
    I2C_Stop();
}

// Инициализация дисплея
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

// Очистка буфера и экрана
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

// Полное обновление экрана из буфера
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

// Включение дисплея
void SSD1306_DisplayOn(void) {
    SSD1306_WriteCmd(SSD1306_CMD_DISPLAY_ON);
}

// Выключение дисплея
void SSD1306_DisplayOff(void) {
    SSD1306_WriteCmd(SSD1306_CMD_DISPLAY_OFF);
}
