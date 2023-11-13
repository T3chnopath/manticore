#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdint.h>

#if defined(STM32H503)
#include "stm32h503xx.h"
#elif defined(STM32H563)
#include "stm32h563xx.h"
#endif

inline __attribute__((always_inline)) uint32_t Get_Freq(uint32_t systemClk, uint32_t timerPrescaler, uint32_t timerPeriod) {
    return (systemClk / (timerPrescaler+1)) / (timerPeriod+1);
}

inline __attribute__((always_inline)) int32_t map(int32_t x, int32_t x_min, int32_t x_max, int32_t out_min, int32_t out_max){
	if(x <= x_min)
		return out_min;

	if(x >= x_max)
		return out_max;

	return (x - x_min) * (out_max - out_min) / (x_max - x_min) + out_min;
}


inline __attribute__((always_inline)) void GPIO_PortClkEnable(GPIO_TypeDef * port)
{
    if(port == GPIOA)
            __HAL_RCC_GPIOA_CLK_ENABLE();

    else if(port == GPIOB)
            __HAL_RCC_GPIOB_CLK_ENABLE();
        
    else if(port == GPIOC)
            __HAL_RCC_GPIOC_CLK_ENABLE();
        
    else if(port == GPIOD)
            __HAL_RCC_GPIOD_CLK_ENABLE();

#ifdef STM32H563
    else if (port == GPIOE)
            __HAL_RCC_GPIOE_CLK_ENABLE();

    else if (port == GPIOF)
            __HAL_RCC_GPIOF_CLK_ENABLE();

    else if (port == GPIOG)
            __HAL_RCC_GPIOG_CLK_ENABLE();

    else if (port == GPIOI)
            __HAL_RCC_GPIOI_CLK_ENABLE();

#endif
    else 
            __HAL_RCC_GPIOH_CLK_ENABLE();
}

inline __attribute__((always_inline)) IRQn_Type PinToEXTI(uint16_t pin)
{
     switch(pin)
     {
         case(GPIO_PIN_0):
             return EXTI0_IRQn;
             break;

         case(GPIO_PIN_1):
             return EXTI1_IRQn;
             break;

         case(GPIO_PIN_2):
             return EXTI2_IRQn;
             break;

         case(GPIO_PIN_3):
             return EXTI3_IRQn;
             break;

         case(GPIO_PIN_4):
             return EXTI4_IRQn;
             break;

         case(GPIO_PIN_5):
             return EXTI5_IRQn;
             break;

         case(GPIO_PIN_6):
             return EXTI6_IRQn;
             break;

         case(GPIO_PIN_7):
             return EXTI7_IRQn;
             break;

         case(GPIO_PIN_8):
             return EXTI8_IRQn;
             break;

         case(GPIO_PIN_9):
             return EXTI9_IRQn;
             break;

         case(GPIO_PIN_10):
             return EXTI10_IRQn;
             break;

         case(GPIO_PIN_11):
             return EXTI11_IRQn;
             break;

         case(GPIO_PIN_12):
             return EXTI12_IRQn;
             break;

         case(GPIO_PIN_13):
             return EXTI13_IRQn;
             break;

         case(GPIO_PIN_14):
             return EXTI14_IRQn;
             break;

         case(GPIO_PIN_15):
             return EXTI15_IRQn;
             break;

         default:
             return EXTI0_IRQn;     
             break;
     }
}

#endif