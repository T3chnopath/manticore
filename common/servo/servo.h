#ifndef __SERVO_H
#define __SERVO_H

#include <stdint.h>
#include "stm32h5xx_hal.h"  

/* List of Macros */
#define DESIRED_SERVO_FREQ  (50U)           //In Hz
#define DESIRED_ACT_FREQ    (50U)           //In Hz

//Servo Motor Error Enum
typedef enum{

	SERVO_OK 		 		= 0x00U,
	SERVO_FREQ_ERROR 		= 0x01U,
	SERVO_RANGE_ERROR_MIN	= 0x02U,
	SERVO_RANGE_ERROR_MAX	= 0x03U,
	SERVO_INSTANCE_ERROR	= 0x04U

} Servo_Error;

//Actuator Enum
typedef enum{
    ACTUATOR_OK             = 0x00U,
    ACTUATOR_UNDER_RANGE    = 0x01U,
    ACTUATOR_ABOVE_RANGE    = 0x02U,
    ACTUATOR_INSTANCE_ERROR = 0x03U,
    ACTUATOR_FREQ_ERROR     = 0x04U
} Actuator_Error;

//Servo Motor Struct
typedef struct{

	float minDuty;
	float maxDuty;
	int8_t minAngle;
	int8_t maxAngle;

} Servo_Config_t;

typedef struct{

	TIM_HandleTypeDef*	htim;
	uint8_t				channel;
	Servo_Config_t*		config;

	/* These will be set in the INIT function */
	uint32_t 			minCnt;
	uint32_t 			maxCnt;

} Servo_Instance_t;


//Actuator Struct
typedef struct{
    uint16_t 	Min_Pulse;                     //uS Based
    uint16_t 	Max_Pulse;                     //uS Based
    uint8_t 	Min_Length;                    //mm Based
    uint8_t 	Max_Length;                    //mm Based
	uint8_t		Desired_Min_Length;			   //mm Based
	uint8_t 	Desired_Max_Length;			   //mm Based
} Actuator_Config_t;

typedef struct 
{
	TIM_HandleTypeDef* Act_Timer;
	uint8_t Channel;
    Actuator_Config_t* config;

    /* Will be Initialized by function */
	uint16_t Min_Cnt;                     
	uint16_t Max_Cnt;                     
} Actuator_Instance_t;



//Servo Functions
Servo_Error Servo_Init(Servo_Instance_t* servo);
Servo_Error Drive_Servo(const Servo_Instance_t* servo, const int8_t angle);

//Actuator Functions
Actuator_Error Actuator_Init(Actuator_Instance_t* act);
Actuator_Error Drive_Actuator(const Actuator_Instance_t* act, const uint8_t length);

#endif

