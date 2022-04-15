#include "loop.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern IWDG_HandleTypeDef hiwdg;

//функции состояний
void idle(void);
void error(void);
void recieve_from_ppu(void);
void transmit_to_ppu(void);
void calculation(void);
void write_in_eeprom(void); 
void limiter_break_az(void);
void limiter_break_el(void);

//вспомогательные функции
void auto_mode_az_calculation(void);
void auto_mode_el_calculation(void);
void create_message(uint8_t* buffer);
void read_message(uint8_t* buffer);
void ParseToInt(uint8_t* orbitron_buffer); 
void ParseToIntXY(uint8_t* orbitron_buffer);

//структуры для работы с моторами
Stepper_motor_t motor_AZ = {GPIOA,I1_E_Pin,I1_1_Pin,I1_2_Pin,I1_3_Pin,I1_4_Pin,&htim14,TIM14,AZIMUTH};
Stepper_motor_t motor_EL = {GPIOA,I2_E_Pin,I2_1_Pin,I2_2_Pin,I2_3_Pin,I2_4_Pin,&htim16,TIM16,ELEVATION};

//структуры для хранения данных о положении ПУ и объекта из орбитрона
RotatorData_t RotatorCoords;
OrbitronData_t ObjectCoords;
JoystickData_t JoystickCoords;

uint8_t rx_buffer[10] = {0};                  // буфер для приема с ппу
_Bool usart1_recieve_from_ppu_complete = 0; 

int braking_in_auto_mode = 0;
uint32_t braking_timer = 0;
const uint32_t braking_timeout = 3000;

// флаги
volatile _Bool auto_mode = 0;
volatile _Bool manual_mode = 0;

_Bool move_AZ_enable = 0;
_Bool move_EL_enable = 0;

volatile _Bool move_auto_mode_az = 0;
volatile _Bool move_auto_mode_el = 0;

volatile _Bool rotator_update_data_az = 0;
volatile _Bool rotator_update_data_el = 0;

volatile _Bool power_off_interrupt = 0;

volatile int limiter_az_pressed = 0;
volatile int limiter_el_pressed = 0;

extern volatile _Bool motor_joystick_move_on_az;
extern volatile _Bool motor_joystick_move_on_el;

// состояния системы
volatile State_t current_state = STATE_RX;   // начальное состояние при запуске
volatile Event_t current_event = EVENT_NONE;
volatile State_t prev_state = STATE_RX;	


TRANSITION_FUNC_PTR_t transition_table[STATE_MAX][EVENT_MAX] = {
			
		[STATE_POWER_OFF][EVENT_NONE] = write_in_eeprom,		
	
		[STATE_RX][EVENT_NONE] = recieve_from_ppu,
		[STATE_RX][EVENT_OK] = transmit_to_ppu,
	
		[STATE_TX][EVENT_NONE] = error, 
		[STATE_TX][EVENT_OK] = calculation,
	
		[STATE_CALCULATION][EVENT_NONE] = error,
		[STATE_CALCULATION][EVENT_OK] = recieve_from_ppu,	
	
		[STATE_LIMITER_AZ][EVENT_NONE] =  limiter_break_az,
		[STATE_LIMITER_AZ][EVENT_OK] =  limiter_break_az,
		
		[STATE_LIMITER_EL][EVENT_NONE] =  limiter_break_el,
		[STATE_LIMITER_EL][EVENT_OK] =  limiter_break_el,
};


void superloop(void)
{
	// Читаем данные из eeprom 
	RotatorCoords.azimuth = mem_read32(1); 
	HAL_Delay(10);
	RotatorCoords.elevation = mem_read32(10); 
	
	motor_set_stepping_mode(&motor_AZ,HALF_DRIVEN_STEP);	// установка режима работы мотора
	motor_set_stepping_mode(&motor_EL,HALF_DRIVEN_STEP);
	
	while(1)
	{
		transition_table[current_state][current_event]();
	}
}


void error(void)       // ошибка перехода! 
{
	motor_stop(&motor_AZ);
	motor_stop(&motor_EL);

	__disable_irq();
	while(1) {}   
}


void recieve_from_ppu(void)
{
	if(!power_off_interrupt)
	{
		current_state = STATE_RX;
		current_event = EVENT_NONE;
	}

	HAL_IWDG_Refresh(&hiwdg);				// обновляем сторожевой таймер

	HAL_GPIO_WritePin(RS485_DE_GPIO_Port,RS485_DE_Pin,GPIO_PIN_RESET);    // устанавливаем rs485 на приём
	HAL_UART_Receive_IT(&huart1,rx_buffer,10);	   
		
	if(usart1_recieve_from_ppu_complete)
	{
		manual_mode = 0;
		auto_mode = 0;
		usart1_recieve_from_ppu_complete = 0;
		braking_timer = HAL_GetTick();
		
		read_message(rx_buffer);                  
		
		HAL_UART_AbortReceive_IT(&huart1);
		
		//current_event = EVENT_OK; 	
		/* переменная current_event меняется в функции read_message();
			 мы отталкиваемся оттого какого формата(режим) пришло сообщение */		
	}
	
	// разрешаем авто-режим после экстренного торможения( необходимо заново включить драйвер DDE_To_Serial в орбитроне)
	if(braking_in_auto_mode && (HAL_GetTick() - braking_timer > braking_timeout))  
	{
		braking_in_auto_mode = 0;
	}
}

void transmit_to_ppu(void) 
{
		if(!power_off_interrupt)
		{
			current_state = STATE_TX;
			current_event = EVENT_NONE;
		}
		
		HAL_GPIO_WritePin(RS485_DE_GPIO_Port,RS485_DE_Pin,GPIO_PIN_SET);   // устанавливаем rs485 на передачу

		uint8_t tx_buffer[11] = {0};
		create_message(tx_buffer);										                     //готовим сообщение для отправки
		
		if(huart1.gState == HAL_UART_STATE_READY) 
		{
			HAL_UART_Transmit(&huart1,tx_buffer,11,0xFFFF); // отправляем данные
			current_event = EVENT_OK;
		}
}

void calculation(void)                // считаем градусы для поворота
{
	if(!power_off_interrupt)
	{
		current_state = STATE_CALCULATION;
		current_event = EVENT_NONE;
	}	
	
	if(auto_mode && !braking_in_auto_mode) 
	{
			// поворачиваем ПУ согласно координатам
			if(ObjectCoords.elevation >= 12 && ObjectCoords.azimuth <= 350)  // 12 градусов - допустимо возможное отклонение по элевации
			{
					if(move_auto_mode_az == 0)
					{
						auto_mode_az_calculation();
					}
					
					if(move_auto_mode_el == 0) 
					{
						auto_mode_el_calculation();
					}
			}	
	}
	
	else if(manual_mode)          // ручной режим(джойстик)
	{
			if(JoystickCoords.X >= 230) 															 { motor_move_joystick_AZ(&motor_AZ,DIR_CW,3); }  		
			else if(JoystickCoords.X < 40)  													 { motor_move_joystick_AZ(&motor_AZ,DIR_CCW,3);} 
			else if(JoystickCoords.X >= 190 && JoystickCoords.X < 230) { motor_move_joystick_AZ(&motor_AZ,DIR_CW,2); }  
			else if(JoystickCoords.X >= 40  && JoystickCoords.X < 80)  { motor_move_joystick_AZ(&motor_AZ,DIR_CCW,2);} 
			else if(JoystickCoords.X >= 145 && JoystickCoords.X < 190) { motor_move_joystick_AZ(&motor_AZ,DIR_CW,1); } 
			else if(JoystickCoords.X >= 80  && JoystickCoords.X < 100) { motor_move_joystick_AZ(&motor_AZ,DIR_CCW,1);}  
			else 																											 { motor_move_joystick_AZ(&motor_AZ,DIR_CW,0); } //джойстик отпущен 
			
			
			if(JoystickCoords.Y >= 230)                                { motor_move_joystick_EL(&motor_EL,DIR_CCW,3);} 
			else if(JoystickCoords.Y < 40)  													 { motor_move_joystick_EL(&motor_EL,DIR_CW,3); }   
			else if(JoystickCoords.Y >= 190 && JoystickCoords.Y < 230) { motor_move_joystick_EL(&motor_EL,DIR_CCW,2);} 
			else if(JoystickCoords.Y >= 40  && JoystickCoords.Y < 80)  { motor_move_joystick_EL(&motor_EL,DIR_CW,2); } 
			else if(JoystickCoords.Y >= 145 && JoystickCoords.Y < 190) { motor_move_joystick_EL(&motor_EL,DIR_CCW,1);} 
			else if(JoystickCoords.Y >= 80  && JoystickCoords.Y < 100) { motor_move_joystick_EL(&motor_EL,DIR_CW,1); } 
			else																											 { motor_move_joystick_EL(&motor_EL,DIR_CCW,0);}	
	}
	
	current_event = EVENT_OK;	
}

void write_in_eeprom(void)          // выполняется после отключения питания 
{
	if(power_off_interrupt)
	{
		mem_write32(RotatorCoords.azimuth,1);
		HAL_Delay(6);
		mem_write32(RotatorCoords.elevation,10);		
	}	
}


void limiter_break_az(void)              // наехали на ограничитель, остановка мотора по азимуту
{
	current_state = STATE_LIMITER_AZ;
	current_event = EVENT_NONE;
	
	motor_stop(&motor_AZ);
	
	if(prev_state == STATE_RX) current_state = STATE_RX;
	else current_state = STATE_TX;
	
	limiter_az_pressed = 0;
	HAL_Delay(1000);
}

void limiter_break_el(void)             // наехали на ограничитель, остановка мотора по элевации
{
	current_state = STATE_LIMITER_EL;
	current_event = EVENT_NONE;
	
	motor_stop(&motor_EL);
	
	if(prev_state == STATE_RX) current_state = STATE_RX;
	else current_state = STATE_TX;
	
	limiter_el_pressed = 0;
	HAL_Delay(1000);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)         // обработчик прерывания приема
{
	usart1_recieve_from_ppu_complete = 1;	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)     // обработчик для моторов 
{
	if(htim == &htim14) 
	{
		Motor_Timer_Handler(&motor_AZ);    // выполняем 1 шаг мотором 
	}
	else if(htim == &htim16) 
	{
		Motor_Timer_Handler(&motor_EL);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case PWR_CTR_Pin: power_off_interrupt = 1; current_state = STATE_POWER_OFF; current_event = EVENT_NONE; // отключили питание
										 break;
		
		case K_AZ_1_Pin: HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
										 limiter_az_pressed++;
										 if(limiter_az_pressed == 1)
										 {
											 prev_state = current_state;
											 current_state = STATE_LIMITER_AZ; 
											 current_event = EVENT_NONE; 
										 }
										 break;
		
		case K_AZ_2_Pin: HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
										 limiter_az_pressed++;
										 if(limiter_az_pressed == 1)
										 {
											 prev_state = current_state;
											 current_state = STATE_LIMITER_AZ; 
											 current_event = EVENT_NONE; 
										 }
										 break;					
	
		case K_EL_1_Pin: HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
										 limiter_el_pressed++;
										 if(limiter_el_pressed == 1)
										 {
											 prev_state = current_state;
											 current_state = STATE_LIMITER_EL; 
											 current_event = EVENT_NONE; 
										 }
										 break;
		
		case K_EL_2_Pin: HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin); 
										 limiter_el_pressed++;
										 if(limiter_el_pressed == 1)
										 {
											 prev_state = current_state;
											 current_state = STATE_LIMITER_EL; 
											 current_event = EVENT_NONE; 
										 }
										 break;	
	}
}

void auto_mode_az_calculation(void)                // считаем градусы для авто режима по азимуту
{
		if(ObjectCoords.azimuth - RotatorCoords.azimuth != 0)
		{				
			if(RotatorCoords.azimuth - ObjectCoords.azimuth < 0)
			{
				motor_set_direction(&motor_AZ,DIR_CW);
			}
			else 
			{
				motor_set_direction(&motor_AZ,DIR_CCW);
			}
					
			int distanse_az = abs(ObjectCoords.azimuth - RotatorCoords.azimuth);
			motor_move(&motor_AZ,distanse_az);
			move_auto_mode_az = 1;
		}
}

void auto_mode_el_calculation(void)                 // считаем градусы для авто режима по элевации
{ 
	if(ObjectCoords.elevation - RotatorCoords.elevation != 0)
		{				
			if(RotatorCoords.elevation - ObjectCoords.elevation < 0)
			{
				motor_set_direction(&motor_EL,DIR_CW);
			}
			else 
			{
				motor_set_direction(&motor_EL,DIR_CCW);
			}
			
			int distanse_el = abs(ObjectCoords.elevation - RotatorCoords.elevation);
			motor_move(&motor_EL,distanse_el);
			move_auto_mode_el = 1;
		}
}

void create_message(uint8_t* buffer)	// формат: $[AZ] [EL] [D]!  , D - показывает движется или нет ПУ, $ - начало, ! - конец
{	
		char AZ_rotator_string[4] = {'*','*','*'};
		char EL_rotator_string[4] = {'*','*','*'};
	
		sprintf(AZ_rotator_string,"%d",RotatorCoords.azimuth);
		sprintf(EL_rotator_string,"%d",RotatorCoords.elevation);
	
		buffer[0] = '$';
		
		int bf_position = 1;
		for(int i = 0; ( i < 3 && AZ_rotator_string[i] != '\0'); i++, bf_position++)
		{
			buffer[bf_position] = (uint8_t)AZ_rotator_string[i];
		}
		
		buffer[bf_position] = ' ';
		bf_position++;
		
		for(int i = 0; (i < 3 && EL_rotator_string[i] != '\0'); i++, bf_position++)
		{
			buffer[bf_position] = (uint8_t)EL_rotator_string[i];
		}
		
		buffer[bf_position] = ' ';
		bf_position++;
		
		if(move_auto_mode_az || move_auto_mode_el || motor_joystick_move_on_az || motor_joystick_move_on_el)
		{
			buffer[bf_position] = 'D';				// ПУ движется
		}
		else buffer[bf_position] = 'S';     // ПУ не движется
		
		bf_position++;	
		buffer[bf_position] = '!';	
}


void read_message(uint8_t* buffer)
{
	if(buffer[0] == '$')
	{
		switch(buffer[1])
		{
			case 'a':											// автоматический режим
				auto_mode = 1;
				ParseToInt(buffer);
				current_event = EVENT_OK;
				break;
			
			case 'm':												// ручной режим
				manual_mode = 1;
				ParseToIntXY(buffer);
				current_event = EVENT_OK;
				break;
			
			case 'z':												// устанавливаем нулевые значения по удержанию кнопки
				RotatorCoords.azimuth = 0; 				
				RotatorCoords.elevation = 12;
				current_event = EVENT_NONE;
				break;
			
			case 'b':												// экстренное торможение по кнопке
				motor_stop(&motor_AZ);
				motor_stop(&motor_EL);
				current_event = EVENT_NONE;
				if(buffer[2] == 'a')
				{
					braking_in_auto_mode = 1; 
					move_auto_mode_az = 0;
					move_auto_mode_el = 0;
				}
				else if(buffer[2] == 'm')
			  {}
				break;
			
			//default:       // обработать ошибку приёма второго байта!  
				
		}		
	}
	
	else {}  //обработать ошибку приема первого байта!
	
}

void ParseToInt(uint8_t* orbitron_buffer) 
{
	int buffer_position = 0; // для движения по буфферу
	
	ObjectCoords.azimuth = 0;
	ObjectCoords.elevation = 0;
	
	for (buffer_position = 2; orbitron_buffer[buffer_position] != ' '; buffer_position++);
	buffer_position--;
	
	//парсим и записываем значение в поле azimuth 
	for (int k = 1; (orbitron_buffer[buffer_position] != 'a' && orbitron_buffer[buffer_position] != 'm') ; buffer_position--, k *= 10) 
	{
		ObjectCoords.azimuth += (orbitron_buffer[buffer_position] - '0') * k;
	}
	
	for ( ;orbitron_buffer[buffer_position] != '!'; buffer_position++);
	buffer_position--;

	//парсим и записываем значение в поле elevation 
	for (int k = 1; orbitron_buffer[buffer_position] != ' ' ; buffer_position--, k *= 10) //считаем единицы, десятки, сотни значения
	{
		if (orbitron_buffer[buffer_position] == '-')
		{
			ObjectCoords.elevation *= -1;
			break;
		}
		ObjectCoords.elevation += (orbitron_buffer[buffer_position] - '0') * k;
	}
}

void ParseToIntXY(uint8_t* orbitron_buffer)
{
	JoystickCoords.X = 0;
	JoystickCoords.Y = 0;
	
	int buffer_position = 0; // индекс элемента для движения по буфферу
	
	for (buffer_position = 2; orbitron_buffer[buffer_position] != ' '; buffer_position++);
	buffer_position--;
	
	//парсим и записываем значение в поле azimuth 
	for (int k = 1; (orbitron_buffer[buffer_position] != 'a' && orbitron_buffer[buffer_position] != 'm') ; buffer_position--, k *= 10) 
	{
		JoystickCoords.X += (orbitron_buffer[buffer_position] - '0') * k;
	}
	
	for ( ;orbitron_buffer[buffer_position] != '!'; buffer_position++);
	buffer_position--;

	//парсим и записываем значение в поле elevation 
	for (int k = 1; orbitron_buffer[buffer_position] != ' ' ; buffer_position--, k *= 10) //считаем единицы, десятки, сотни значения
	{
		JoystickCoords.Y += (orbitron_buffer[buffer_position] - '0') * k;
	}

}
