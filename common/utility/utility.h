#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdint.h>

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

#endif