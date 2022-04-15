#include "EEPROM.h"


void mem_write32(uint32_t value, uint16_t address)
{
      uint8_t Buffer[4];
      Buffer[3] =  value&0xFF;
      Buffer[2] = (value >> 8)&0xFF;
      Buffer[1] = (value >> 16)&0xFF;
      Buffer[0] = (value >> 24)&0xFF;
			HAL_I2C_Mem_Write(&hi2c1, (uint16_t) I2C1_DEVICE_ADDRESS<<1, address, I2C_MEMADD_SIZE_8BIT, (uint8_t*)Buffer, sizeof(Buffer), 5);
		
}

uint32_t mem_read32(uint16_t address)
{
	  uint8_t Buffer[4];
	  HAL_I2C_Mem_Read(&hi2c1, (uint16_t) I2C1_DEVICE_ADDRESS<<1, address, I2C_MEMADD_SIZE_8BIT, (uint8_t*)Buffer, sizeof(Buffer), 5);
	  uint32_t value = Buffer[3]|(Buffer[2] << 8)|(Buffer[1] << 16)|(Buffer[0] << 24);
	return value;
}

	uint32_t i = 100;

_Bool mem_test(void)
{
	i = 455;
	mem_write32(i, 20);
	i=0;
	HAL_Delay(10);
	i = mem_read32(20);
	if (i == 455) return 0;
	else return 1;
}

typedef union FloatToBytes
    {
        float asFloat;
        char asBytes[4];
    } FloatToBytes;


void EEPROM_float_write(float value,uint16_t address){
    FloatToBytes Data;
    Data.asFloat = value;
    HAL_I2C_Mem_Write(&hi2c1, (uint16_t) I2C1_DEVICE_ADDRESS<<1, address, I2C_MEMADD_SIZE_8BIT, Data.asBytes, 4, HAL_MAX_DELAY);
}

float EEPROM_float_read(uint16_t address){
    FloatToBytes Data;
    while(HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t) I2C1_DEVICE_ADDRESS<<1, 1, HAL_MAX_DELAY) != HAL_OK);
    HAL_I2C_Mem_Read(&hi2c1, (uint16_t) I2C1_DEVICE_ADDRESS<<1, address, I2C_MEMADD_SIZE_8BIT, Data.asBytes, 4, HAL_MAX_DELAY);
    return Data.asFloat;
}
