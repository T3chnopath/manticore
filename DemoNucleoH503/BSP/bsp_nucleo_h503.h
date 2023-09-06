#ifndef __MAIN_H
#define __MAIN_H

#include "stm32h5xx_hal.h"
#include "stm32h5xx_ll_bus.h"
#include "stm32h5xx_ll_cortex.h"
#include "stm32h5xx_ll_rcc.h"
#include "stm32h5xx_ll_system.h"
#include "stm32h5xx_ll_utils.h"
#include "stm32h5xx_ll_pwr.h"
#include "stm32h5xx_ll_gpio.h"

// Pin Definitions
#define LED1_GREEN_Pin GPIO_PIN_5
#define LED1_GREEN_GPIO_Port GPIOA

// Public Functions
void BSP_Error_Handler(void);
void BSP_SystemClock_Config(void);
void BSP_GPIO_Init(void);

#endif /* __MAIN_H */
