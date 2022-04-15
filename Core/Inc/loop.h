#ifndef LOOP_H_
#define LOOP_H_

#include "main.h"

typedef struct 
{
	int azimuth;
	int elevation;
	
} RotatorData_t;


typedef struct 
{
	int azimuth;
	int elevation;
	
} OrbitronData_t;

typedef struct 
{
	int X;
	int Y;
	
} JoystickData_t;

//для таблицы переходов
typedef void (*TRANSITION_FUNC_PTR_t)(void);

// состояния системы
typedef enum
{
	STATE_IDLE,
	STATE_RX,
	STATE_TX, 
	STATE_CALCULATION,
	STATE_POWER_OFF,
	STATE_LIMITER_AZ,
	STATE_LIMITER_EL,
	STATE_MAX
} State_t;

typedef enum
{
	EVENT_NONE,
	EVENT_OK,
	EVENT_MAX
} Event_t;


void superloop(void);

	
#endif /*LOOP_H_*/
