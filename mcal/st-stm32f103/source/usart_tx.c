#include "usart_tx.h"
#include "stm32f10x.h"

// Передача по UART посимвольно
void USART1_TX (uint8_t* dt)		// Создаем указатель на переменную типа uint8 (в контексте строки - указатель - адрес 1-го символа)
{
    while (*dt)  // Отправляем пока не встретим нулевой символ (разыменование)
    {
        while (!(USART1->SR & USART_SR_TXE)) {} // ждем когда выставится флаг полной передачи (сдвигоый регистр очистился)
			USART1->DR = *dt++; // вот здесь пиздец. сначала разыменование и передача в DR а только потом инкрементирование (без разыменования), потому что в постфиксе  
    }
}

void delay(uint32_t time) {
    volatile uint32_t count = time;
    while(count--) {
        __NOP();  // No Operation - предотвращает оптимизацию
    }
}

// Программный сброс I2C
void I2C_Reset(void) {
	I2C1->CR1 |= I2C_CR1_SWRST; 	// Установка и сброс SWRST
	I2C1->CR1 &= ~I2C_CR1_SWRST;	// для перезапуска I2C
	 uint8_t temp __attribute__((unused)) = I2C1->SR1 | I2C1->SR2;
}

// Передача события Start I2C
void I2C_Start(void) {
	I2C1->CR1 |= I2C_CR1_START;
	delay(100);
  // Ждать окончания события START
	while(!(I2C1->SR1 & I2C_SR1_SB)) {}
}

// Передача события Stop I2C
void I2C_Stop(void) {
  I2C1->CR1 |= I2C_CR1_STOP; 
  // Ждать завершения события STOP
  while(!(I2C1->CR1 & I2C_CR1_STOP)) {}  
}

// Передача адреса I2C (выбор адресата)
void I2C_Select(uint8_t address, uint8_t readBit) {
  // Поместить адрес и бит чтения/записи в регистр DR (R или ~W)
  I2C1->DR = (address << 1) | (1 & readBit); 
  // Ждать окончания передачи адреса
	uint16_t timing = 10000;
  while(!(I2C1->SR1 & I2C_SR1_ADDR)) {
	if(!(timing--)) break;
  }
  // Прочесть SR1 и SR2 для стирания бита ADDR
  uint8_t temp __attribute__((unused)) = I2C1->SR1 | I2C1->SR2;
}

// Запись байта по I2C
void I2C_Write(uint8_t data) { 
  // Дождаться освобождения регистра данных
  while(!(I2C1->SR1 & I2C_SR1_TXE)) {}
  // Загрузка байта в регистр DR
  I2C1->DR = data;                        
  // Дождаться окончания передачи
  while(!(I2C1->SR1 & I2C_SR1_BTF)) {}
}

// Прием байта по I2C (с ожиданием бита ACK и без него)
uint8_t I2C_Read(uint8_t * byte, uint8_t ack) {
	if (ack) I2C1->CR1 |= I2C_CR1_ACK; 	// Если нужно подтверждение
	else I2C1->CR1 &= ~(I2C_CR1_ACK); 	// без подтверждения
	uint32_t timeout = 5000000;
	while(!(I2C1->SR1 & (I2C_SR1_RXNE))) { // Дождаться окончания приема байта
    if (!timeout--) {
	I2C_Reset();
	return 1; // Ошибка: нет ответа
		}
	}
	*byte = I2C1->DR; // Забрать прочитанный байт

	return 0; // Без ошибок
}

void I2C1_Busy_Errata(void) {		// Перезагрузка I2C настоящая, из Errata
	I2C1->CR1 &= ~I2C_CR1_PE;
	GPIOB->CRL = 0x77444444;
	GPIOB->ODR = 1<<6 | 1<<7;
	(void) GPIOB->IDR;
	GPIOB->ODR = 0<<7;
	(void) GPIOB->IDR;
	GPIOB->ODR = 0<<6;
	(void) GPIOB->IDR;
	GPIOB->ODR = 1<<6;
	(void) GPIOB->IDR;
	GPIOB->ODR = 1<<7;
	(void) GPIOB->IDR;
	GPIOB->CRL = 0xFF444444;
	I2C1->CR1 |= I2C_CR1_SWRST;
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	I2C1->SR1 = 0;
	I2C1->SR2 = 0;
	I2C1->CR1 |= I2C_CR1_PE;
}