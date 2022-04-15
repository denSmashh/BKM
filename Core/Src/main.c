/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;
IWDG_HandleTypeDef hiwdg;


void SystemClock_Config(void);
static void GPIO_Init(void);
static void I2C1_Init(void);
static void USART1_UART_Init(void);
static void TIM14_Init(void);
static void TIM16_Init(void);
static void TIM17_Init(void);
static void IWDG_Init(void);


int main(void)
{
  
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
  I2C1_Init();
  USART1_UART_Init();
	TIM14_Init();
	TIM16_Init();
	TIM17_Init();
	IWDG_Init();
	
  superloop();
	
}

void SystemClock_Config(void)	 // тактовая частота - 48 MHz
{
	
	__HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
	
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0}; 
	
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;					// для WatchDog таймера
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
	
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
	
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	//PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


static void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, I1_1_Pin|I1_2_Pin|I1_3_Pin|I1_4_Pin
                          |I2_2_Pin|I2_1_Pin|I2_3_Pin|I2_4_Pin
                           , GPIO_PIN_RESET);


  /*Configure GPIO pins : I1_1_Pin I1_2_Pin I1_3_Pin I1_4_Pin
                           I2_2_Pin I2_1_Pin I2_3_Pin I2_4_Pin
                            */
  GPIO_InitStruct.Pin = I1_1_Pin|I1_2_Pin|I1_3_Pin|I1_4_Pin
                          |I2_2_Pin|I2_1_Pin|I2_3_Pin|I2_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_CTR_Pin K_AZ_2_Pin K_AZ_1_Pin */
  GPIO_InitStruct.Pin = K_AZ_2_Pin|K_AZ_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = PWR_CTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  /*Configure GPIO pins : LED_Pin RS485_DE_Pin */
  GPIO_InitStruct.Pin = LED_Pin|RS485_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, LED_Pin|RS485_DE_Pin, GPIO_PIN_RESET);

	
  /*Configure GPIO pins : K_EL_2_Pin K_EL_1_Pin */
  GPIO_InitStruct.Pin = K_EL_2_Pin|K_EL_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*Configure pin I1_E for PWM*/	
	GPIO_InitStruct.Pin = I1_E_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
	HAL_GPIO_Init(I1_E_GPIO_Port,&GPIO_InitStruct);
	
	/*Configure pin I2_E for PWM*/
	GPIO_InitStruct.Pin = I2_E_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
	HAL_GPIO_Init(I2_E_GPIO_Port,&GPIO_InitStruct);
	
	/*Configure USART1 for RS-485*/
	 GPIO_InitStruct.Pin = RS485_TX_Pin|RS485_RX_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure I2C for EEPROM*/
	GPIO_InitStruct.Pin = EEPROM_SCL_Pin|EEPROM_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	
	//настройка EXTI модуля для прерываний
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	HAL_NVIC_SetPriority(EXTI0_1_IRQn,0,0);
	
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
	HAL_NVIC_SetPriority(EXTI2_3_IRQn,0,0);
	
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,0,0);
}


static void I2C1_Init(void)
{
	__HAL_RCC_I2C1_CLK_ENABLE();
	
  hi2c1.Instance = I2C1;
	//hi2c1.Init.Timing = 0x00000212; //300 kHz
	//hi2c1.Init.Timing = 0x0000020B; //400 kHz with analog filter
	hi2c1.Init.Timing  = 0x0000020C;  //400 kHz without analog filter
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
 /*
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  */
}


static void USART1_UART_Init(void)
{

	__HAL_RCC_USART1_CLK_ENABLE();
	
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
	
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_NVIC_SetPriority(USART1_IRQn,0,0);
  
}
static void TIM14_Init(void)
{
	__HAL_RCC_TIM14_CLK_ENABLE();
	
	htim14.Instance = TIM14;
  //htim14.Init.Prescaler = 48000 - 1; // для мс задержек
	htim14.Init.Prescaler = 48 - 1; 	// для мкс задержек
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	//htim14.Init.Period = 20 - 1; // задержка указывается в мс
  htim14.Init.Period = MIN_HF_DELAY_AZ; // задержка указывается в мкс
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim14);
	
	HAL_NVIC_EnableIRQ(TIM14_IRQn);
	HAL_NVIC_SetPriority(TIM14_IRQn,0,0);
	
}

static void TIM16_Init(void)
{
	__HAL_RCC_TIM16_CLK_ENABLE();
	
	htim16.Instance = TIM16;
  //htim16.Init.Prescaler = 48000 - 1; // для мс интервалов
	htim16.Init.Prescaler = 48 - 1; 	// для мкс интервалов
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = MIN_HF_DELAY_EL;  // задержка указывается в мкс
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim16);
	
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
	HAL_NVIC_SetPriority(TIM16_IRQn,0,0);
	
}

static void TIM17_Init(void)
{
	__HAL_RCC_TIM17_CLK_ENABLE();
	
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 48 - 1; 	// для мкс интервалов
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 50 - 1;  // задержка указывается в мкс
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim17);
	
	HAL_NVIC_EnableIRQ(TIM17_IRQn);
	HAL_NVIC_SetPriority(TIM17_IRQn,0,0);
}

static void IWDG_Init(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 3999;  // ~ 2.5 c
	hiwdg.Init.Window = 3999;
	HAL_IWDG_Init(&hiwdg);
}

void Error_Handler(void)
{
  
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
