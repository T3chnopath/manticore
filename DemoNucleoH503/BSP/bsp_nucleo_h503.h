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

#define FDCAN1_EN

// Pin Definitions
#define LED_GREEN_GPIO_Port     GPIOA
#define LED_GREEN_Pin           GPIO_PIN_5
#define LED_GREEN_PORT_CLK_EN() __HAL_RCC_GPIOA_CLK_ENABLE()

#ifdef  FDCAN1_EN  
#define FDCAN_TX_Port           GPIOB
#define FDCAN_TX_Pin            GPIO_PIN_10
#define FDCAN_TX_PORT_CLK_EN()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define FDCAN_RX_Port           GPIOB
#define FDCAN_RX_Pin            GPIO_PIN_12
#define FDCAN_RX_PORT_CLK_EN()  __HAL_RCC_GPIOB_CLK_ENABLE()
#endif

// Public Functions
void BSP_Init(void);

#endif /* __MAIN_H */
