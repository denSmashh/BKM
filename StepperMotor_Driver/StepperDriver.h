/*
ƒрайвер шагового мотора на базе микросхемы L293DD дл€ бипол€рного
шагового двигател€ 50BYJ2662
*/

#ifndef STEPPER_DRIVER_H_
#define STEPPER_DRIVER_H_

#include "main.h"


#define DIR_CW  								((uint8_t)0)  // по часовой стрелке 
#define DIR_CCW 								((uint8_t)1)  // против часовой стрелки 

#define WAVE_DRIVEN_STEP 				((uint8_t)0)  // режим активации одной фазы
#define HALF_DRIVEN_STEP 				((uint8_t)1)  // полушаговый режим

#define AZIMUTH 				 				((uint8_t)0) 
#define ELEVATION  						 	((uint8_t)1)


#define MIN_HF_DELAY_AZ 				((uint32_t)(22000 - 1))			// минимальна€ задержка дл€ AZ
#define MAX_HF_DELAY_AZ 				((uint32_t)(3000 - 1))

//дл€ PSC = 48
#define MIN_HF_DELAY_EL  				((uint32_t)(10000 - 1)) 		// минимальна€ задержка дл€ EL 
#define MAX_HF_DELAY_EL 				((uint32_t)(4000 - 1))

#define MIN_DEGREE_ACCEL_AZ  		10
#define MIN_DEGREE_ACCEL_EL  		10

#define ACCEL_SPEED_AZ					2000
#define ACCEL_SPEED_EL					2000

#define LVL1_DELAY_JOYSTICK_AZ  MIN_HF_DELAY_AZ   
#define LVL2_DELAY_JOYSTICK_AZ  ((uint32_t)(10000 - 1))   
#define LVL3_DELAY_JOYSTICK_AZ  ((uint32_t)(4000 - 1))

#define LVL1_DELAY_JOYSTICK_EL  MIN_HF_DELAY_EL   
#define LVL2_DELAY_JOYSTICK_EL  ((uint32_t)(6000 - 1))   
#define LVL3_DELAY_JOYSTICK_EL  ((uint32_t)(3000 - 1))



typedef struct 
{
	  GPIO_TypeDef* IN_GPIO;
		uint16_t Enable_Pin;
		uint16_t Input1;
		uint16_t Input2;
		uint16_t Input3;
		uint16_t Input4;		

		TIM_HandleTypeDef* TIM_Handler;
		TIM_TypeDef* TIM;
	
		uint8_t target;  				// AZIMUTH или ELEVATION
		uint8_t Stepping_mode;	//WAVE_DRIVEN_STEP или WAVE_DRIVEN_STEP
		uint8_t direction;  		//DIR_CW или DIR_CCW
	
		uint8_t step_index;
		uint8_t max_step_index;
			
} Stepper_motor_t;


//void motor_one_step(Stepper_motor_t* Motor);
void motor_move(Stepper_motor_t* Motor, uint32_t degree);
void motor_stop(Stepper_motor_t* Motor);
void motor_set_stepping_mode(Stepper_motor_t* Motor, uint8_t mode);
void motor_set_direction(Stepper_motor_t* Motor,uint8_t direction);
void motor_set_speed(Stepper_motor_t* Motor, int timer_delay_mcs);
void motor_move_joystick_AZ(Stepper_motor_t* Motor, uint8_t direction, uint8_t speed);
void motor_move_joystick_EL(Stepper_motor_t* Motor, uint8_t dir, uint8_t speed);
void Motor_Timer_Handler(Stepper_motor_t* Motor);



#endif /*STEPPER_DRIVER_H_*/
