/*
 *  motor.c
 *
 *  V1: Main implementation of DC Motor & Actuator control functions
 *  V2: Main implementation of Servo Motor control function
 * 
 *  Created on: Jan 11, 2023
 *      Author: Jackie Huynh & Michelle Tran
 * 
 */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "dc_motor.h"
#include "utility.h"
#include "stm32h5xx_hal.h"

// #define STM32F446

static int32_t currEncodeCnt    = 0;
static int16_t currAngle        = 0;

//DC Motor Functions
DCMotor_Error DCMotor_Init(DCMotor_Instance_t* dcMotor){

    HAL_StatusTypeDef error;

    /* Assert Param */
    if(dcMotor == NULL)
        return DC_MOTOR_INSTANCE_ERR;
    else if(dcMotor->config == NULL)
        return DC_MOTOR_INSTANCE_ERR;

    /* Calculate Min and Max counter value based on user config */
    dcMotor->Min_Cnt = __HAL_TIM_GET_AUTORELOAD(dcMotor->DC_Timer) * (float)(dcMotor->config->Min_Speed / 100.0);
    dcMotor->Max_Cnt = __HAL_TIM_GET_AUTORELOAD(dcMotor->DC_Timer) * (float)(dcMotor->config->Max_Speed / 100.0);

    /* Start PWM Signal */
    error  = HAL_TIM_PWM_Start(dcMotor->DC_Timer, dcMotor->IN1_Channel);
    error |= HAL_TIM_PWM_Start(dcMotor->DC_Timer, dcMotor->IN2_Channel);
    error |= HAL_TIM_GenerateEvent(dcMotor->DC_Timer, TIM_EventSource_Update);       //Sync Counter

    /* Error Handling */
    if(error != HAL_OK){
        // HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        while(1);
    }

    return DC_MOTOR_OK;
}

DCMotor_Error Drive_DCMotor(const DCMotor_Instance_t* dcMotor, const uint8_t speed, DCMotor_Direction dir){

    /* Assert Param */
    if(dcMotor == NULL)
        return DC_MOTOR_INSTANCE_ERR;
    else if(dcMotor->config == NULL)
        return DC_MOTOR_INSTANCE_ERR;
    else if(speed < dcMotor->config->Min_Speed)
        return DC_MOTOR_UNDER_RANGE;
    else if(speed > dcMotor->config->Max_Speed)
        return DC_MOTOR_ABOVE_RANGE;

    /* Map speed value to counter value */
    uint16_t mappedValue = map(speed, dcMotor->config->Min_Speed, dcMotor->config->Max_Speed, dcMotor->Min_Cnt, dcMotor->Max_Cnt);

    if(dir == CLOCKWISE){
        __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN1_Channel, mappedValue);
        __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN2_Channel, 0);
    }
    else if(dir == COUNTER_CLOCKWISE){
        __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN1_Channel, 0);
        __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN2_Channel, mappedValue);
    }

    HAL_TIM_GenerateEvent(dcMotor->DC_Timer, TIM_EventSource_Update);       //Sync Counter

    return DC_MOTOR_OK;
}

DCMotor_Error Stop_DCMotor(const DCMotor_Instance_t* dcMotor){

    /* Assert Param */
    if(dcMotor == NULL)
        return DC_MOTOR_INSTANCE_ERR;
    else if(dcMotor->config == NULL)
        return DC_MOTOR_INSTANCE_ERR;

    //Set Both Compare to 0 for 0% power
    __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN1_Channel, 0);
    __HAL_TIM_SET_COMPARE(dcMotor->DC_Timer, dcMotor->IN2_Channel, 0);

    return DC_MOTOR_OK;
}

//DC Motor W Encoder Functions
DCMotor_Error DCMotor_Encoder_Init(DCMotor_Encoder_Instance_t* encMotor){

    HAL_StatusTypeDef error;

    /* Assert Param */
    if((encMotor == NULL) || (encMotor->encConfig == NULL) || (encMotor->motorInstance == NULL))
        return DC_MOTOR_INSTANCE_ERR;

    /* Set Local Scope Variables */
    currAngle       = encMotor->encConfig->Current_Angle;
    currEncodeCnt   = encMotor->encConfig->Default_Counter;

    /* Set Encoder Default Counter Value */
    __HAL_TIM_SET_COUNTER(encMotor->Encoder_Timer, encMotor->encConfig->Default_Counter);
    error = HAL_TIM_Encoder_Start(encMotor->Encoder_Timer, TIM_CHANNEL_ALL);

    /* Error Handling */
    if(error != HAL_OK){
        // HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        while(1);
    }

    return DC_MOTOR_OK;

}

DCMotor_Error Drive_DCMotor_Angle(const DCMotor_Encoder_Instance_t* encMotor, int16_t angle){

    DCMotor_Error error;
    uint32_t desiredCnt;
    uint32_t angle_to_counter = 0;

    /* Assert Param */
    if((encMotor == NULL) || (encMotor->encConfig == NULL) || (encMotor->motorInstance == NULL))
        return DC_MOTOR_OK;
    else if(angle > encMotor->encConfig->Max_Angle)
        return DC_MOTOR_ABOVE_RANGE;
    else if(angle < encMotor->encConfig->Min_Angle)
        return DC_MOTOR_UNDER_RANGE;

    /* Default Parameter Arguments */
    if(angle == 0)
        return DC_MOTOR_OK;
    else if(angle == 360){
        desiredCnt = encMotor->encConfig->Default_Counter * 2;
        error = Drive_DCMotor(encMotor->motorInstance, 40, CLOCKWISE);
        while(currEncodeCnt <= desiredCnt){
            currEncodeCnt = __HAL_TIM_GET_COUNTER(encMotor->Encoder_Timer);
        }
        error |= Stop_DCMotor(encMotor->motorInstance);
    }
    else if(angle == -360){
        desiredCnt = 0;
        error = Drive_DCMotor(encMotor->motorInstance, 40, COUNTER_CLOCKWISE);
        while(currEncodeCnt >= desiredCnt){
            currEncodeCnt = __HAL_TIM_GET_COUNTER(encMotor->Encoder_Timer);
        }
        error |= Stop_DCMotor(encMotor->motorInstance);
    }
    else{
        angle_to_counter = abs(angle) / encMotor->encConfig->Degree_Per_Pulse;
        if(angle < 0){
            desiredCnt = currEncodeCnt - angle_to_counter;
            error = Drive_DCMotor(encMotor->motorInstance, 99, COUNTER_CLOCKWISE);
            HAL_Delay(1);
            error = Drive_DCMotor(encMotor->motorInstance, 40, COUNTER_CLOCKWISE);
            while(currEncodeCnt >= desiredCnt){
                currEncodeCnt = __HAL_TIM_GET_COUNTER(encMotor->Encoder_Timer);
            }
            error |= Stop_DCMotor(encMotor->motorInstance);
        }
        else{
            desiredCnt = currEncodeCnt + angle_to_counter;
            error = Drive_DCMotor(encMotor->motorInstance, 99, CLOCKWISE);
            HAL_Delay(1);
            error = Drive_DCMotor(encMotor->motorInstance, 40, CLOCKWISE);
            while(currEncodeCnt <= desiredCnt){
                currEncodeCnt = __HAL_TIM_GET_COUNTER(encMotor->Encoder_Timer);
            }
            error |= Stop_DCMotor(encMotor->motorInstance);
        }
    }

    /* Update Control Variable */
    currAngle += angle;
    currEncodeCnt = __HAL_TIM_GET_COUNTER(encMotor->Encoder_Timer);

    return DC_MOTOR_OK;
}
