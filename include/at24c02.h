#ifndef AT24
#define AT24

#include <stdint.h>

void AT24_Write_Byte(uint8_t word_address,uint8_t *data);
void AT24_Write_Page(uint8_t word_address,uint8_t *buf);
void AT24_Read_Current_Byte(uint8_t *data);
void AT24_Read_Byte(uint8_t word_address,uint8_t *data);
void AT24_Read_Sequent(uint8_t word_address,uint8_t *buf, uint8_t count);

#endif