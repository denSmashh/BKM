/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

#include "loop.h"
#include "StepperDriver.h"
#include "EEPROM.h"
#include "stdlib.h"

void Error_Handler(void);


#define I1_1_Pin GPIO_PIN_0
#define I1_1_GPIO_Port GPIOA
#define I1_2_Pin GPIO_PIN_1
#define I1_2_GPIO_Port GPIOA
#define I1_3_Pin GPIO_PIN_2
#define I1_3_GPIO_Port GPIOA
#define I1_4_Pin GPIO_PIN_3
#define I1_4_GPIO_Port GPIOA
#define I2_2_Pin GPIO_PIN_4
#define I2_2_GPIO_Port GPIOA
#define I2_1_Pin GPIO_PIN_5
#define I2_1_GPIO_Port GPIOA
#define I2_3_Pin GPIO_PIN_6
#define I2_3_GPIO_Port GPIOA
#define I2_4_Pin GPIO_PIN_7
#define I2_4_GPIO_Port GPIOA
#define PWR_CTR_Pin GPIO_PIN_0
#define PWR_CTR_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOB
#define I2_E_Pin GPIO_PIN_8
#define I2_E_GPIO_Port GPIOA
#define EEPROM_SCL_Pin GPIO_PIN_9
#define EEPROM_SCL_GPIO_Port GPIOA
#define EEPROM_SDA_Pin GPIO_PIN_10
#define EEPROM_SDA_GPIO_Port GPIOA
#define I1_E_Pin GPIO_PIN_11
#define I1_E_GPIO_Port GPIOA
#define K_EL_2_Pin GPIO_PIN_12
#define K_EL_2_GPIO_Port GPIOA
#define K_EL_1_Pin GPIO_PIN_15
#define K_EL_1_GPIO_Port GPIOA
#define K_AZ_2_Pin GPIO_PIN_3
#define K_AZ_2_GPIO_Port GPIOB
#define K_AZ_1_Pin GPIO_PIN_4
#define K_AZ_1_GPIO_Port GPIOB
#define RS485_DE_Pin GPIO_PIN_5
#define RS485_DE_GPIO_Port GPIOB
#define RS485_TX_Pin GPIO_PIN_6
#define RS485_TX_GPIO_Port GPIOB
#define RS485_RX_Pin GPIO_PIN_7
#define RS485_RX_GPIO_Port GPIOB




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
