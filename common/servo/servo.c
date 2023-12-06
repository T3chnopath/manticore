#include "servo.h"
#include "utility.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_CLOCKWISE_SPEED_PERCENTAGE  (0.064f)
#define ONE_POINT_FIVE_MS_PERCENTAGE    (0.075f)
#define MAX_COUNTER_SPEED_PERCENTAGE    (0.086f)
#define SAMPLE_CNT                      (15U)
#define PID_Kp                          (0.025f)      
#define PID_Ki                          (0.00025f)
#define INTEGRAL_CAP                    (200.0)
#define SERVO_SPEED_OFFSET              (30.0)
#define ANGLE_THRESHOLD                 (2)
#define SPEED_OFFSET                    (30.0)
#define SERVO_MIN_TIM_US                (0)
#define SERVO_MAX_TIM_US                (20000)
#define ERROR_THRESHOLD                 (1.2)
#define STEADY_STATE_CNT_THRES          (1000)
#define INITIAL_ANGLE_OFFSET            (8)
#define INTEGRAL_RESET_THRESHOLD        (1000)

//Local Scope Typedef
typedef enum{
    CAPTURE_FREQUENCY,
    CAPTURE_DUTY
} FEEDBACK_CAPTURE;

typedef enum{
    CAPTURE_INITIAL_RISING,
    CAPTURE_FALLING,
    CAPTURE_NEXT_RISING,
    CALCULATE
} CAPTURE_STATE;

//Local Scope Variables
static char servoFreqError      = 0;
static char actFreqError        = 0;
static uint16_t freqCnt = 0;
static float currAngle = 0;
static float currDuty = 0;
static volatile FEEDBACK_CAPTURE feedbackCap = CAPTURE_FREQUENCY;
static volatile CAPTURE_STATE captureState   = CAPTURE_INITIAL_RISING;
static volatile uint16_t initialRiseCnt = 0;
static volatile uint16_t fallingCnt     = 0;
static volatile uint16_t nextRiseCnt    = 0;
static volatile bool captureComplete = false; 
static uint16_t stopCnt = 0;
static uint16_t clockwiseSpeedCnt = 0;
static uint16_t counterClockwiseSpeedCnt = 0;

//Local Scope Function
static void Change_IC_IT_Edge(TIM_HandleTypeDef* tim, const uint8_t channel, uint32_t polarity){
    TIM_IC_InitTypeDef sConfigIC = {0};

    sConfigIC.ICPolarity = polarity;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(tim, &sConfigIC, channel) != HAL_OK)
    {
        while(1);
    }
}

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

Servo_Error CONT_Servo_Init(CONT_Servo_Instance_t* contServo){

    /* Temp Variables */
    static uint32_t contTimerFreq = 0;
    static uint32_t counterTimerFreq = 0;
    uint32_t differenceSum = 0;

    /* Assert Parameters */
    if(contServo == NULL)
        return SERVO_INSTANCE_ERROR;
    else if(contServo->contServoConfig == NULL)
        return SERVO_INSTANCE_ERROR;

    /* Check if Continuous Servo Timer is setup to be 50Hz PWM */
    contTimerFreq = Get_Freq(HAL_RCC_GetSysClockFreq(), contServo->contServoTimer->Init.Prescaler, __HAL_TIM_GET_AUTORELOAD(contServo->contServoTimer));
    if(contTimerFreq != DESIRED_SERVO_FREQ)
        return SERVO_FREQ_ERROR;

    /* Set Servo Motor speed to Stop mode */

    //Find Counter value for 1.5ms
    stopCnt = __HAL_TIM_GET_AUTORELOAD(contServo->contServoTimer) * ONE_POINT_FIVE_MS_PERCENTAGE;
    clockwiseSpeedCnt = __HAL_TIM_GET_AUTORELOAD(contServo->contServoTimer) * MAX_CLOCKWISE_SPEED_PERCENTAGE;
    counterClockwiseSpeedCnt = __HAL_TIM_GET_AUTORELOAD(contServo->contServoTimer) * MAX_COUNTER_SPEED_PERCENTAGE;

    //Set Continuous Servo Motor Timer compare counter to 1.5ms
    __HAL_TIM_SET_COMPARE(contServo->contServoTimer, contServo->contServoChannel, stopCnt);

    //Start Continuous Servo Motor PWM Timer
    HAL_TIM_PWM_Start(contServo->contServoTimer, contServo->contServoChannel);

    //Generate Update event to sync timer
    HAL_TIM_GenerateEvent(contServo->contServoTimer, TIM_EventSource_Update);

    /* Counter Timer Init and Calibration */

    //Check if Counter Timer does not count slower than feedback signal 
    counterTimerFreq = HAL_RCC_GetSysClockFreq() / (contServo->ICTimer->Init.Prescaler + 1);
    if(counterTimerFreq < contServo->contServoConfig->feedbackFreq)
        return SERVO_FREQ_ERROR;

    //Start Counter TimerstopCounter
    feedbackCap = CAPTURE_FREQUENCY;
    Change_IC_IT_Edge(contServo->ICTimer, contServo->ICTimerChannel, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(contServo->ICTimer, contServo->ICTimerChannel);
    HAL_TIM_GenerateEvent(contServo->ICTimer, TIM_EventSource_Update);

    //Validate and Calibrate Actual Real-Time Feedback Signal
    
    //Grab Feedback Frequency
    for(uint8_t i = 0; i < SAMPLE_CNT; i++){
        //Poll until capture is complete
        while(!captureComplete);
        captureComplete = false; 

        if(nextRiseCnt < initialRiseCnt)
            differenceSum += (0xFFFF - initialRiseCnt) + nextRiseCnt;  //Account for Overflow
        else
            differenceSum += (nextRiseCnt - initialRiseCnt);
    }

    // Calculate Frequency
    freqCnt = differenceSum / SAMPLE_CNT;

    // Capture Initial Angle
    HAL_TIM_IC_Stop(contServo->ICTimer, contServo->ICTimerChannel);
    Change_IC_IT_Edge(contServo->ICTimer, contServo->ICTimerChannel, TIM_INPUTCHANNELPOLARITY_BOTHEDGE);
    __HAL_TIM_SET_COUNTER(contServo->ICTimer, 0);
    feedbackCap = CAPTURE_DUTY;
    captureState = CAPTURE_NEXT_RISING;
    HAL_TIM_IC_Start_IT(contServo->ICTimer, contServo->ICTimerChannel);

    while(captureState != CALCULATE);
    captureComplete = false;

    // Calculate Initial Angle
    if(nextRiseCnt > fallingCnt)
        currDuty = ((0xFFFF - nextRiseCnt) + fallingCnt) / (float)freqCnt;
    else
        currDuty = (fallingCnt - nextRiseCnt) / (float)freqCnt;

    // Calculate Detected Angle
    currAngle = map(currDuty, contServo->contServoConfig->feedbackMinDuty, contServo->contServoConfig->feedbackMaxDuty, 0, 360);

    HAL_TIM_IC_Stop(contServo->ICTimer, contServo->ICTimerChannel);
    captureState = CAPTURE_NEXT_RISING;

    // Zero out Servo
    if((180 - currAngle) > 0)
        Drive_CONT_Servo_Angle(contServo, 180 - currAngle, SERVO_COUNTERCLOCKWISE);
    else
        Drive_CONT_Servo_Angle(contServo, currAngle - 180, SERVO_CLOCKWISE);

    return SERVO_OK;
}

Servo_Error Drive_CONT_Servo_Angle(CONT_Servo_Instance_t* contServo, int16_t angle, CSERVO_DIR dir){

    static float targetAngle = 0.0;
    static float detectedAngle = 0.0;
    float detectedDuty = 0.0;
    float output = 0.0;
    static float error = 0.0;
    float pError = 0.0;
    float iError = 0.0;
    float prevError = 0.0;
    float offset = 0.0;
    uint16_t steadyStateCnt = 0;
    uint16_t integralResetCnt = 0;

    /* Assert Parameter */
    if(contServo == NULL)
        return SERVO_INSTANCE_ERROR;
    else if(contServo->contServoConfig == NULL)
        return SERVO_INSTANCE_ERROR;
    
    /* Calculate Current Angle */
    if(dir == SERVO_CLOCKWISE){
        targetAngle = currAngle - angle;
        if(targetAngle < 0){
            targetAngle = 360 + targetAngle;
        }
    }
    else if(dir == SERVO_COUNTERCLOCKWISE){
        targetAngle = currAngle + angle;
        if(targetAngle > 360){
            targetAngle = targetAngle - 360;
        }
    }

    /* Change IC Timer to Capture Duty */
    HAL_TIM_IC_Stop(contServo->ICTimer, contServo->ICTimerChannel);
    Change_IC_IT_Edge(contServo->ICTimer, contServo->ICTimerChannel, TIM_INPUTCHANNELPOLARITY_BOTHEDGE);
    __HAL_TIM_SET_COUNTER(contServo->ICTimer, 0);
    feedbackCap = CAPTURE_DUTY;
    captureState = CAPTURE_NEXT_RISING;
    HAL_TIM_IC_Start_IT(contServo->ICTimer, contServo->ICTimerChannel);

    /* PID Control Loop */
    while(1){
        
        // Wait until feedback is detected
        while(captureState != CALCULATE);
        captureComplete = false;

        // Calculate Detected Duty
        if(nextRiseCnt > fallingCnt)
            detectedDuty = ((0xFFFF - nextRiseCnt) + fallingCnt) / (float)freqCnt;
        else
            detectedDuty = (fallingCnt - nextRiseCnt) / (float)freqCnt;

        // Calculate Detected Angle
        detectedAngle = map(detectedDuty, contServo->contServoConfig->feedbackMinDuty, contServo->contServoConfig->feedbackMaxDuty, 0, 360);

        // P and I error calculation
        error = -(targetAngle - detectedAngle);

        //Check if we are within threshold
        if((error < ERROR_THRESHOLD) && (error > -ERROR_THRESHOLD)){
            if(steadyStateCnt > STEADY_STATE_CNT_THRES){
                __HAL_TIM_SET_COMPARE(contServo->contServoTimer, contServo->contServoChannel, stopCnt);
                currAngle = detectedAngle;
                break;
            }
            steadyStateCnt++;
        }
        else{
            steadyStateCnt = 0;
        }

        pError = PID_Kp * error;

        iError = PID_Ki * (prevError + error);

        prevError += error;

        output = pError + iError;

        // Cap Output to make sure we don't exceed PWM Limit
        if(output > INTEGRAL_CAP)
            output = 200.0;
        else if(output < -INTEGRAL_CAP)
            output = -200.0;

        // Counter Speed Offset
        if(error > ERROR_THRESHOLD)
            offset = SPEED_OFFSET;
        else if(error < -ERROR_THRESHOLD)
            offset = -SPEED_OFFSET;
        else
            offset = 0;

        // Drive Servo Motor
        __HAL_TIM_SET_COMPARE(contServo->contServoTimer, contServo->contServoChannel,
                              map(1500 + output + offset, SERVO_MIN_TIM_US, SERVO_MAX_TIM_US, 0, contServo->contServoTimer->Init.Period));

        // Prevent Integral explosion
        integralResetCnt++;
        if(integralResetCnt > INTEGRAL_RESET_THRESHOLD){
            integralResetCnt = 0;
            prevError = 0;
        }

        captureState = CAPTURE_NEXT_RISING;   

    }

    return SERVO_OK;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
    
    switch(feedbackCap){

        case CAPTURE_FREQUENCY:
            
            switch(captureState){

                case CAPTURE_INITIAL_RISING:
                    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET){
                        initialRiseCnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                        captureState = CAPTURE_NEXT_RISING;
                    }
                    break;

                case CAPTURE_NEXT_RISING:
                    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET){
                        nextRiseCnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                        captureState = CAPTURE_INITIAL_RISING;
                        captureComplete = true;
                        __HAL_TIM_SET_COUNTER(htim, 0);
                    }
                    break;

                default:
                    break;

            }

            break;

        case CAPTURE_DUTY:

            switch(captureState){

                case CAPTURE_NEXT_RISING:
                    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET){
                        nextRiseCnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                        captureState = CAPTURE_FALLING;
                    }
                    break;

                case CAPTURE_FALLING:
                    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET){
                        fallingCnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                        captureState = CALCULATE;
                        captureComplete = true;
                        __HAL_TIM_SET_COUNTER(htim, 0);
                    }
                    break;

                default:
                    break;

            }

            break; 

    }

}