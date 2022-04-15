/*
	Библиотека для внешней микросхемы EEPROM "m24c01-wmn6p"
*/

#ifndef EEPROM_H_
#define EEPROM_H_

#include "main.h"

#define I2C1_DEVICE_ADDRESS      0x50   /* A0 = A1 = A2 = 0; т.к. в протоколе I2C 7-ми битный адрес устройстве */  
#define MEMORY_ADDRESS           0x01         

extern I2C_HandleTypeDef hi2c1;


void mem_write32(uint32_t value, uint16_t address);
uint32_t mem_read32(uint16_t address);
void EEPROM_float_write(float value,uint16_t address);
float EEPROM_float_read(uint16_t address);
_Bool memtest(void);


#endif /*EEPROM_H_*/
