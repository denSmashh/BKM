#include "StepperDriver.h"

#define INCREASE_SPEED_AZ(value) 				if(Motor->TIM->ARR > MAX_HF_DELAY_AZ) Motor->TIM->ARR -= (value)
#define DECREASE_SPEED_AZ(value) 				if(Motor->TIM->ARR < MIN_HF_DELAY_AZ) Motor->TIM->ARR += (value)

#define INCREASE_SPEED_EL(value) 				if(Motor->TIM->ARR > MAX_HF_DELAY_EL) Motor->TIM->ARR -= (value)
#define DECREASE_SPEED_EL(value) 				if(Motor->TIM->ARR < MIN_HF_DELAY_EL) Motor->TIM->ARR += (value)

#define DEC_ROT_DATA_AZ()			   				if(RotatorCoords.azimuth - 1 < 0) RotatorCoords.azimuth = 359;	 	\
																				else RotatorCoords.azimuth -= 1
																	 
#define INC_ROT_DATA_AZ() 			 				if(RotatorCoords.azimuth + 1 > 359) RotatorCoords.azimuth = 0;     \
																				else RotatorCoords.azimuth += 1

#define CONSTRAIN_TOP(value,limit)   		value = ((value) > (limit)) ? (limit) : (value)
#define CONSTRAIN_BOTTOM(value,limit)   value = ((value) < (limit)) ? (limit) : (value)		

uint32_t AZ_degree; 
uint32_t EL_degree;

volatile _Bool acceleration_AZ_enable;
volatile _Bool acceleration_EL_enable;

extern volatile _Bool move_auto_mode_az;
extern volatile _Bool move_auto_mode_el;

static uint32_t step_az = 0;	
static uint32_t step_el = 0; 

extern volatile _Bool manual_mode;
extern volatile _Bool rotator_update_data_az;
extern volatile _Bool rotator_update_data_el;

extern volatile _Bool auto_mode;
volatile _Bool motor_joystick_move_on_az = 0;
volatile _Bool motor_joystick_move_on_el = 0;

volatile _Bool move_one_degree_az = 0;
volatile _Bool move_one_degree_el = 0;

uint8_t prev_dir_az = 0;
uint8_t prev_dir_el = 0;

extern RotatorData_t RotatorCoords;														 
																 
void motor_AZ_acceleration(Stepper_motor_t* Motor,int degree);
void motor_EL_acceleration(Stepper_motor_t* Motor,int degree);
void update_data_auto_mode_az(Stepper_motor_t* Motor);
void update_data_auto_mode_el(Stepper_motor_t* Motor);


static uint8_t BIPOLAR_WD_PATTERN[4][4] = {
	
				{1,0,0,0},
				{0,0,1,0},
				{0,1,0,0},
				{0,0,0,1},			
};

static uint8_t BIPOLAR_HD_PATTERN[8][8] = {
				
				{1,0,0,0},
				{1,0,1,0},
				{0,0,1,0},
				{0,1,1,0},
				{0,1,0,0},
				{0,1,0,1},
				{0,0,0,1},
				{1,0,0,1},
};


void motor_one_step(Stepper_motor_t* Motor)          // делаем шаг мотором
{
	 if(Motor->Stepping_mode == HALF_DRIVEN_STEP)
	{
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input1,BIPOLAR_HD_PATTERN[Motor->step_index][0]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input2,BIPOLAR_HD_PATTERN[Motor->step_index][1]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input3,BIPOLAR_HD_PATTERN[Motor->step_index][2]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input4,BIPOLAR_HD_PATTERN[Motor->step_index][3]);
	}
	
	else if(Motor->Stepping_mode == WAVE_DRIVEN_STEP)
	{
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input1,BIPOLAR_WD_PATTERN[Motor->step_index][0]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input2,BIPOLAR_WD_PATTERN[Motor->step_index][1]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input3,BIPOLAR_WD_PATTERN[Motor->step_index][2]);
		HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input4,BIPOLAR_WD_PATTERN[Motor->step_index][3]);
	}	
		
	if(Motor->direction == DIR_CW)
	{
		if(Motor->step_index == Motor->max_step_index) Motor->step_index = 0;
		else Motor->step_index++;
	}
	
	else if(Motor->direction == DIR_CCW)
	{
		if(Motor->step_index == 0) Motor->step_index = Motor->max_step_index;
		else Motor->step_index--;
	}
}

/* Выбираем мотор и количество градусов для поворота
   Для выбора направления вызвать функцию void motor_set_direction(Stepper_motor_t* Motor, uint8_t direction)
*/
void motor_move(Stepper_motor_t* Motor, uint32_t degree)
{
	if(auto_mode && Motor->target == AZIMUTH) 
	{
		AZ_degree = degree;
		if (AZ_degree >= MIN_DEGREE_ACCEL_AZ) acceleration_AZ_enable = 1;
	}
	
	else if(auto_mode && Motor->target == ELEVATION) 
	{
		EL_degree = degree;
		if (EL_degree >= MIN_DEGREE_ACCEL_EL) acceleration_EL_enable = 1;
	}
	
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_SET); // включаем разрещающий пин
	HAL_TIM_Base_Start_IT(Motor->TIM_Handler);                        // заводим таймер с опредленной задержкой	

}

//совершает поворот на 1 градус 
void Motor_Timer_Handler(Stepper_motor_t* Motor)                // вызывается в обработчике прерывания таймера
{
	
	motor_one_step(Motor);
	
	switch(Motor->target)
	{
		case AZIMUTH: 
			step_az++;
		
			if(auto_mode && (step_az % 26 == 0))  // ускорение в HF режиме AZ
			{
				update_data_auto_mode_az(Motor);
				motor_AZ_acceleration(Motor,step_az/26);
			}				
			
		  if(manual_mode && (step_az % 26 == 0)) move_one_degree_az = 1; 
		
			if(Motor->Stepping_mode == HALF_DRIVEN_STEP && step_az == AZ_degree * 26) 
			{
				HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
				HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); //выключаем разрещающий пин
				step_az  = 0;
				AZ_degree = 0;
				acceleration_AZ_enable = 0;
				move_auto_mode_az = 0;
			}
			
			else if(Motor->Stepping_mode == WAVE_DRIVEN_STEP && step_az == AZ_degree* 13) 
			{
				HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
				HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); 
				step_az  = 0;
				AZ_degree = 0;
				acceleration_AZ_enable = 0;
				move_auto_mode_az = 0;
			}
		break;
		
		
		case ELEVATION:	
			step_el++;
		
			if(auto_mode && (step_el % 176 == 0)) 
			{
				update_data_auto_mode_el(Motor);
				motor_EL_acceleration(Motor,step_el/176); // ускорение в HF режиме EL
			}
			
			if(manual_mode && (step_el % 176 == 0)) move_one_degree_el = 1;
		
			if(Motor->Stepping_mode == HALF_DRIVEN_STEP && step_el == EL_degree * 176) 
			{
				HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
				HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); //выключаем разрещающий пин
				step_el = 0;
				EL_degree = 0;
				acceleration_EL_enable = 0;
				move_auto_mode_el = 0;
			}
	
			else if(Motor->Stepping_mode == WAVE_DRIVEN_STEP && step_el == EL_degree * 88) // перемещение на x градусов по элевации в WAVE режиме
			{
				HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
				HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); //выключаем разрещающий пин
				step_el = 0;
				EL_degree = 0;
				acceleration_EL_enable = 0;
				move_auto_mode_el = 0;
			}
		break;
			
	}
}


void update_data_auto_mode_az(Stepper_motor_t* Motor)            // обновление данных о положение ПУ в структуре данных
{
		if(Motor->direction == DIR_CCW) 
		{
				if(RotatorCoords.azimuth - 1 < 0) RotatorCoords.azimuth = 359;	
				else RotatorCoords.azimuth -= 1;
		}
		else if(Motor->direction == DIR_CW)
		{
			 if(RotatorCoords.azimuth + 1 > 359) RotatorCoords.azimuth = 0;     
				else RotatorCoords.azimuth += 1;
		}	
		rotator_update_data_az = 0;

}

void update_data_auto_mode_el(Stepper_motor_t* Motor)
{
			if(Motor->direction == DIR_CCW)	RotatorCoords.elevation -= 1;
			else if(Motor->direction == DIR_CW) RotatorCoords.elevation += 1;
			
			rotator_update_data_el = 0;

}


/* обработка поворота джойстика по оси Y, в соответствии с отклонением 
   принцип работы: запускаем движение при первом отклонении джойстика, 
                   как только мы отпускаем джойстик происходит торможение.
                   Изменение скорости происходит когда мотор сделал один градус 
*/
void motor_move_joystick_AZ(Stepper_motor_t* Motor, uint8_t dir, uint8_t speed)
{
	if(motor_joystick_move_on_az == 0 && speed != 0)   // начинаем движение при первом отклонение джойстика
	{
		motor_set_direction(Motor,dir);
		motor_move(Motor,360);						
	
		 if(dir == DIR_CW)
		 {
			 INC_ROT_DATA_AZ();
		 }			
		 else if(dir == DIR_CCW) 
		 {
			 DEC_ROT_DATA_AZ();
		 }

		motor_joystick_move_on_az = 1;
		prev_dir_az = dir;               // храним данные о направлении для торможения
	}
	
	if(dir != prev_dir_az)					// обработка резкой смены направления
	{
			if(Motor->TIM->ARR <= MIN_HF_DELAY_AZ) speed = 0;
	}

	switch(speed) 
	{
		case 0: if(move_one_degree_az)
						{					
							if(Motor->TIM->ARR < MIN_HF_DELAY_AZ)               // плавное томожение
							{
								if(Motor->TIM->ARR < LVL2_DELAY_JOYSTICK_AZ) Motor->TIM->ARR += 4000;
								else Motor->TIM->ARR += 8000;
								CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_AZ);
								
								if(prev_dir_az == DIR_CCW) 
								{
									DEC_ROT_DATA_AZ();
								}
								else if(prev_dir_az == DIR_CW)
								{
									INC_ROT_DATA_AZ();
								}	
									
							}
							else if(motor_joystick_move_on_az && Motor->TIM->ARR == MIN_HF_DELAY_AZ) // если скорость минимальна, останавливаемся
							{
									HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
									HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); 
									step_az = 0;
									motor_joystick_move_on_az = 0;
									prev_dir_az = 100;			// любое число кроме 0 и 1 для сброса направления
							}						
							move_one_degree_az = 0;
						}											 								 
			break;
		
		case 1: if(move_one_degree_az)                             // первый уровень отклонения
						{
							if(Motor->TIM->ARR < LVL1_DELAY_JOYSTICK_AZ) Motor->TIM->ARR += 4000; 
			
							if(dir == DIR_CCW) 
							{
								DEC_ROT_DATA_AZ();
							}
							else if(dir == DIR_CW)
							{
								INC_ROT_DATA_AZ();
							}	
							
							move_one_degree_az = 0;
						}
				break;
		
		case 2: if(move_one_degree_az)                              // второй уровень отклонения
						{
							if(Motor->TIM->ARR > LVL2_DELAY_JOYSTICK_AZ) Motor->TIM->ARR -= 6000;
							else if(Motor->TIM->ARR < LVL2_DELAY_JOYSTICK_AZ) Motor->TIM->ARR += 4000; 
								
							if(dir == DIR_CCW) 
							{
								DEC_ROT_DATA_AZ();
							}
							else if(dir == DIR_CW)
							{
								INC_ROT_DATA_AZ();
							}								
								
							move_one_degree_az = 0;					
						}
				break;
		
		case 3: if(move_one_degree_az)                               // третий уровень отклонения (максимум)
						{
							if(Motor->TIM->ARR > LVL2_DELAY_JOYSTICK_AZ) Motor->TIM->ARR -= 6000;
							else if(Motor->TIM->ARR > LVL3_DELAY_JOYSTICK_AZ) 
							{
								if(Motor->TIM->ARR < 6000 - 1) Motor->TIM->ARR -= 500;
								else Motor->TIM->ARR -= 1000;  
							}
								
							if(dir == DIR_CCW) 
							{
								DEC_ROT_DATA_AZ();
							}
							else if(dir == DIR_CW)
							{
								INC_ROT_DATA_AZ();
							}	
							
							move_one_degree_az = 0;
					 } 	
				break;
	}		
}


void motor_move_joystick_EL(Stepper_motor_t* Motor, uint8_t dir, uint8_t speed)  
{
	if(motor_joystick_move_on_el == 0 && speed != 0)       
	{
			motor_set_direction(Motor,dir);
			motor_move(Motor,160);
			
			if(dir == DIR_CCW) RotatorCoords.elevation -= 1;
			else if(dir == DIR_CW) RotatorCoords.elevation += 1;
			
			motor_joystick_move_on_el = 1;
			prev_dir_el = dir;
	}

	if(dir != prev_dir_el)					
	{
		if(Motor->TIM->ARR <= MIN_HF_DELAY_EL) speed = 0;
	}
	
	switch(speed) 
	{
		case 0: if(move_one_degree_el)
						{
							if(Motor->TIM->ARR < MIN_HF_DELAY_EL)                   
							{
								if(Motor->TIM->ARR < LVL2_DELAY_JOYSTICK_EL) Motor->TIM->ARR += 2000;
								else Motor->TIM->ARR += 4000;
								CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_EL);
								 
								if(prev_dir_el == DIR_CCW) RotatorCoords.elevation -= 1;
								else if(prev_dir_el == DIR_CW) RotatorCoords.elevation += 1;							 
							}			
							else if(motor_joystick_move_on_el && Motor->TIM->ARR == MIN_HF_DELAY_EL)    
							{
									HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);
									HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET); 
									step_el = 0;
									motor_joystick_move_on_el = 0;
									prev_dir_el = 100;			// любое число кроме 0 и 1 для сброса направления
							}
							 
							 move_one_degree_el = 0;
						}				 
			break;
		
		case 1:  if(move_one_degree_el)                    
						 {
								if(Motor->TIM->ARR < LVL1_DELAY_JOYSTICK_EL) Motor->TIM->ARR += 2000; 
							 
								if(dir == DIR_CCW) RotatorCoords.elevation -= 1;
							  else if(dir == DIR_CW) RotatorCoords.elevation += 1;
							 
							 move_one_degree_el = 0;
						 }
				break;
		
		case 2:	if(move_one_degree_el)        
						{	
								if(Motor->TIM->ARR > LVL2_DELAY_JOYSTICK_EL) Motor->TIM->ARR -= 4000;
								else if(Motor->TIM->ARR < LVL2_DELAY_JOYSTICK_EL) Motor->TIM->ARR += 2000; 
							
								if(dir == DIR_CCW) RotatorCoords.elevation -= 1;
								else if(dir == DIR_CW) RotatorCoords.elevation += 1;
							
								move_one_degree_el = 0;
						}
				break;
		
		case 3: if(move_one_degree_el)          
						{	
							if(Motor->TIM->ARR > LVL2_DELAY_JOYSTICK_EL) Motor->TIM->ARR -= 4000;
							else if(Motor->TIM->ARR > LVL3_DELAY_JOYSTICK_EL) Motor->TIM->ARR -= 1000;
							
							if(dir == DIR_CCW) RotatorCoords.elevation -= 1;
							else if(dir == DIR_CW) RotatorCoords.elevation += 1;
							
							move_one_degree_el = 0;
						}
				break;
	}
}


void motor_AZ_acceleration(Stepper_motor_t* Motor , int degree)  // ускорение по азимуту в авто режиме (линейное ускорение)
{
	if(acceleration_AZ_enable)
	{	
		if(AZ_degree % 3 == 0) 
		{
			if(degree <= (AZ_degree / 3))
			{
				INCREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_AZ);
			}
			else if(degree > (AZ_degree * 2 / 3))
			{
				DECREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_AZ);
			}
		}
			
		else if((AZ_degree - 1) % 3 == 0)
		{
			if(degree <= (AZ_degree / 3))
			{
				INCREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_AZ);
			}
			else if(degree > ((AZ_degree * 2 / 3) + 1))
			{
				DECREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_AZ);
			}
		}
				
		else if((AZ_degree + 1) % 3 == 0)
		{
			if(degree <= ((AZ_degree / 3) + 1))
			{
				INCREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_AZ);
			}
			else if(degree > (AZ_degree * 2 / 3))
			{
				DECREASE_SPEED_AZ(ACCEL_SPEED_AZ);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_AZ);
			}
		}

	}
}

void motor_EL_acceleration(Stepper_motor_t* Motor , int degree)   // ускорение по элевации в авто режиме (линейное ускорение)
{
	if(acceleration_EL_enable)
	{
		if(EL_degree % 3 == 0) 
		{
			if(degree <= (EL_degree / 3))
			{
				INCREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_EL);
			}
			else if(degree > (EL_degree * 2 / 3))
			{
				DECREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_EL);
			}
		}
			
		else if((EL_degree - 1) % 3 == 0)
		{
			if(degree <= (EL_degree / 3))
			{
				INCREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_EL);
			}
			else if(degree > ((EL_degree * 2 / 3) + 1))
			{
				DECREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_EL);
			}
		}
				
		else if((EL_degree + 1) % 3 == 0)
		{
			if(degree <= ((EL_degree / 3) + 1))
			{
				INCREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_BOTTOM(Motor->TIM->ARR,MAX_HF_DELAY_EL);
			}
			else if(degree > (EL_degree  * 2 / 3))
			{
				DECREASE_SPEED_EL(ACCEL_SPEED_EL);
				CONSTRAIN_TOP(Motor->TIM->ARR,MIN_HF_DELAY_EL);
			}
		}
		
	}
}

void motor_set_stepping_mode(Stepper_motor_t* Motor, uint8_t mode)      // установка режима шагового двигателя
{
		if(mode == WAVE_DRIVEN_STEP) 
		{
			Motor->Stepping_mode = WAVE_DRIVEN_STEP;
			Motor->max_step_index = 3;
		}
		else if(mode == HALF_DRIVEN_STEP)
		{
			Motor->Stepping_mode = HALF_DRIVEN_STEP;
			Motor->max_step_index = 7;
		}
}


void motor_set_direction(Stepper_motor_t* Motor, uint8_t direction)     // установка направления движения
{
  if(direction == DIR_CW)
	{
		Motor->direction = DIR_CW;
		Motor->step_index = 0;
	}
	
	else if(direction == DIR_CCW)
	{
		Motor->direction = DIR_CCW;
		if(Motor->Stepping_mode == WAVE_DRIVEN_STEP) Motor->step_index = 3;
		else if(Motor->Stepping_mode == HALF_DRIVEN_STEP) Motor->step_index = 7;
	}
}


void motor_set_speed(Stepper_motor_t* Motor, int delay_mcs)     // установка задержки между шагами (скорость) 
{
	if(Motor->target  == AZIMUTH)
	{
		if(delay_mcs> MAX_HF_DELAY_AZ) Motor->TIM->ARR = delay_mcs;  
	}
	
	else if(Motor->target  == ELEVATION)
	{
		if(delay_mcs > MAX_HF_DELAY_EL) Motor->TIM->ARR = delay_mcs;
	}
} 


void motor_stop(Stepper_motor_t* Motor)                     // остановка мотора
{
	HAL_TIM_Base_Stop_IT(Motor->TIM_Handler);                 // остановка таймера
	
	//снимаем напряжение с обмоток и выключаем управляющий выход
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Enable_Pin,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input1,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input2,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input3,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Motor->IN_GPIO,Motor->Input4,GPIO_PIN_RESET);	
	
	if(Motor->target == AZIMUTH)
	{
		step_az = 0;
		AZ_degree = 0;
		acceleration_AZ_enable = 0;
		move_auto_mode_az = 0;
		motor_joystick_move_on_az = 0;
		Motor->TIM->ARR = MIN_HF_DELAY_AZ;
	}
	
	else 
	{
		step_el = 0;
		EL_degree = 0;
		acceleration_EL_enable = 0;
		move_auto_mode_el = 0;
		motor_joystick_move_on_el = 0;
		Motor->TIM->ARR = MIN_HF_DELAY_EL;
	}
	
}
