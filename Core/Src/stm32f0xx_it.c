/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_it.h"

extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart1;


void NMI_Handler(void)
{

  while (1)
  {
  }
}


void HardFault_Handler(void)
{
 
  while (1)
  {
  
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  
}



void PendSV_Handler(void)
{
  
}


void SysTick_Handler(void)
{
  
  HAL_IncTick();
 
}


void TIM14_IRQHandler(void) 
{
		HAL_TIM_IRQHandler(&htim14);
}

void TIM16_IRQHandler(void) 
{
		HAL_TIM_IRQHandler(&htim16);
}

void TIM17_IRQHandler(void) 
{
		HAL_TIM_IRQHandler(&htim17);
}


void USART1_IRQHandler(void) 
{
	HAL_UART_IRQHandler(&huart1);
}


void EXTI0_1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(PWR_CTR_Pin);
}


void EXTI2_3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(K_AZ_2_Pin);
}

void EXTI4_15_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(K_EL_1_Pin);
	HAL_GPIO_EXTI_IRQHandler(K_EL_2_Pin);
	HAL_GPIO_EXTI_IRQHandler(K_AZ_1_Pin);
}



/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
