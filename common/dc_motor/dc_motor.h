/*
 *  motor.h
 *
 *  V1: Provide DC Motor & Actuator control functions
 *  V2: Provide Servo Motor control function
 * 
 *  Created on: Jan 11, 2023
 *      Author: Jackie Huynh & Michelle Tran
 * 
 */

#ifndef MOTOR_H_
#define MOTOR_H

#include <stdint.h>
#include "stm32h5xx_hal.h"

//DC Motor Error Enum
typedef enum{
    DC_MOTOR_OK             = 0x00U,
	DC_MOTOR_UNDER_RANGE	= 0x01U,
	DC_MOTOR_ABOVE_RANGE	= 0x02U,
    DC_MOTOR_INSTANCE_ERR   = 0x03U
} DCMotor_Error;

//DC Motor Direction Enum
typedef enum{
    CLOCKWISE               = 0x00U,
    COUNTER_CLOCKWISE       = 0x01u
} DCMotor_Direction;

//DC Motor Struct
typedef struct{
	uint8_t Min_Speed;						//Percentage Based
	uint8_t Max_Speed;						//Percentage Based
} DCMotor_Config_t;

typedef struct{
	TIM_HandleTypeDef* 	DC_Timer;
	uint8_t 			IN1_Channel;
	uint8_t 			IN2_Channel;
	uint16_t			Min_Cnt;                      
	uint16_t			Max_Cnt;   
	DCMotor_Config_t* 	config;                   
} DCMotor_Instance_t;

//DC Motor W Encoder Structs
typedef struct{
	uint32_t 	Default_Counter;
	float 		Degree_Per_Pulse;
	int16_t 	Current_Angle;
	int16_t 	Min_Angle;
	int16_t 	Max_Angle;
} DCMotor_Encoder_Config_t;

typedef struct{
	TIM_HandleTypeDef*			Encoder_Timer;
	DCMotor_Encoder_Config_t* 	encConfig;
	DCMotor_Instance_t* 	  	motorInstance;
} DCMotor_Encoder_Instance_t;

//DC Motor Functions
DCMotor_Error DCMotor_Init(DCMotor_Instance_t* dcMotor);
DCMotor_Error Drive_DCMotor(const DCMotor_Instance_t* dcMotor, const uint8_t speed, DCMotor_Direction dir);
DCMotor_Error Stop_DCMotor(const DCMotor_Instance_t* dcMotor);

//DC Motor With Encoder Function
DCMotor_Error DCMotor_Encoder_Init(DCMotor_Encoder_Instance_t* encMotor);
DCMotor_Error Drive_DCMotor_Angle(const DCMotor_Encoder_Instance_t* encMotor, int16_t angle);

#endif