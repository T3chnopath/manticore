#include "servo.h"
#include <stddef.h>


static char servoFreqError      = 0;
static char actFreqError        = 0;

//Servo Functions
Servo_Error Servo_Init(Servo_Instance_t* servo){

    /* Makes sure Servo Struct and Servo Config Struct are initialized */
    if(servo == NULL)   
        return SERVO_INSTANCE_ERROR;
    else if(servo->config == NULL)
        return SERVO_INSTANCE_ERROR;

    uint16_t timFreq = 0;

	/* Check if this timer is setup correctly for SERVO Control */
    #ifdef STM32F446
	    timFreq = Get_Freq(HAL_RCC_GetSysClockFreq()/2, servo->htim->Init.Prescaler, __HAL_TIM_GET_AUTORELOAD(servo->htim));
    #else
        timFreq = Get_Freq(HAL_RCC_GetSysClockFreq(), servo->htim->Init.Prescaler, __HAL_TIM_GET_AUTORELOAD(servo->htim));
    #endif

	if(timFreq != DESIRED_SERVO_FREQ){
		servoFreqError = 1;
		return SERVO_FREQ_ERROR;
	}

	/* Configure Min and Max count based on duty cycle given */
	servo->minCnt = servo->config->minDuty * __HAL_TIM_GET_AUTORELOAD(servo->htim);
	servo->maxCnt = servo->config->maxDuty * __HAL_TIM_GET_AUTORELOAD(servo->htim);

    /* Start PWM Signal */
    HAL_TIM_PWM_Start(servo->htim, servo->channel);
    HAL_TIM_GenerateEvent(servo->htim, TIM_EventSource_Update);     //Sync Counter

	/* Zero Out SERVO */
	Drive_Servo(servo, 0);

    return SERVO_OK;
}

Servo_Error Drive_Servo(const Servo_Instance_t* servo, const int8_t angle){

    /* Servo Protection */
	if(servoFreqError)
		return SERVO_FREQ_ERROR;

	/* Asserting Params */
	if(servo == NULL)
		return SERVO_INSTANCE_ERROR;
	else if(angle < servo->config->minAngle)
		return SERVO_RANGE_ERROR_MIN;
	else if(angle > servo->config->maxAngle)
		return SERVO_RANGE_ERROR_MAX;

	/* Set New Compare Value */
	__HAL_TIM_SET_COMPARE(servo->htim, servo->channel, map(angle, servo->config->minAngle, servo->config->maxAngle, servo->minCnt, servo->maxCnt));
    HAL_TIM_GenerateEvent(servo->htim, TIM_EventSource_Update);

    return SERVO_OK;
}


//Actuator Functions
Actuator_Error Actuator_Init(Actuator_Instance_t* act){

    uint16_t min_cnt;
    uint16_t max_cnt;

    /* Makes sure Actuator Instance Struct and Actuator Config struct are initialzied */
    if(act == NULL)
        return ACTUATOR_INSTANCE_ERROR;
    else if(act->config == NULL)
        return ACTUATOR_INSTANCE_ERROR;

    /* Calculate Timer Frequency */
    uint16_t timFreq = 0;
    #ifdef STM32F446
        timFreq = Get_Freq(HAL_RCC_GetSysClockFreq()/2, act->Act_Timer->Init.Prescaler, __HAL_TIM_GET_AUTORELOAD(act->Act_Timer));
    #else
        timFreq = Get_Freq(HAL_RCC_GetSysClockFreq(), act->Act_Timer->Init.Prescaler, __HAL_TIM_GET_AUTORELOAD(act->Act_Timer));
    #endif

    /* Check if timer gives desired frequency */
    if(timFreq != DESIRED_ACT_FREQ){
        actFreqError = 1;
        return ACTUATOR_FREQ_ERROR;
    }

    /* Configure Min and Max counter based on user config */

    //Grab Min and Max Count from Min and Max length
    min_cnt = act->config->Min_Pulse / (1 / (DESIRED_ACT_FREQ * pow(10, -6)) / (float)__HAL_TIM_GET_AUTORELOAD(act->Act_Timer));
    max_cnt = act->config->Max_Pulse / (1 / (DESIRED_ACT_FREQ * pow(10, -6)) / (float)__HAL_TIM_GET_AUTORELOAD(act->Act_Timer));
    
    //Save Min and Max Counter for Desired Min and Max Length
    act->Min_Cnt = map(act->config->Desired_Min_Length, act->config->Min_Length, act->config->Max_Length, min_cnt, max_cnt);
    act->Max_Cnt = map(act->config->Desired_Max_Length, act->config->Min_Length, act->config->Max_Length, min_cnt, max_cnt);

    /* Start Actuator PWM Signal */
    __HAL_TIM_SET_COMPARE(act->Act_Timer, act->Channel, act->Min_Cnt);
    HAL_TIM_PWM_Start(act->Act_Timer, act->Channel);
    HAL_TIM_GenerateEvent(act->Act_Timer, TIM_EventSource_Update);      //Sync Coutner

    return ACTUATOR_OK;
}

Actuator_Error Drive_Actuator(const Actuator_Instance_t* act, const uint8_t length){

    /* Asserting Parameters */
    if(act == NULL)
        return ACTUATOR_INSTANCE_ERROR;
    else if(act->config == NULL)
        return ACTUATOR_INSTANCE_ERROR;
    else if(length < act->config->Desired_Min_Length)
        return ACTUATOR_UNDER_RANGE;
    else if(length > act->config->Desired_Max_Length)
        return ACTUATOR_ABOVE_RANGE;

    /* Map length to counter value range */
    uint16_t mappedLength = map(length, act->config->Desired_Min_Length, act->config->Desired_Max_Length, act->Min_Cnt, act->Max_Cnt);
    
    /* Set New PWM Compare Value */
    __HAL_TIM_SET_COMPARE(act->Act_Timer, act->Channel, mappedLength);

    HAL_TIM_GenerateEvent(act->Act_Timer, TIM_EventSource_Update);      //Sync Counter

    return ACTUATOR_OK;
}

