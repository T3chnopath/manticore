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
#define UART3_EN

// Pin Definitions
#define LED_GREEN_GPIO_Port     GPIOA
#define LED_GREEN_Pin           GPIO_PIN_5

#ifdef  FDCAN1_EN  
#define FDCAN_TX_Port           GPIOB
#define FDCAN_TX_Pin            GPIO_PIN_10
#define FDCAN_RX_Port           GPIOB
#define FDCAN_RX_Pin            GPIO_PIN_12
#endif

// UART
#ifdef UART3_EN
#define UART_TX_Port   GPIOA
#define UART_TX_Pin    GPIO_PIN_4
#define UART_RX_Port   GPIOA
#define UART_RX_Pin    GPIO_PIN_3
#define UART_BAUDRATE  921600
#endif

// Public Functions
void BSP_Init(void);

#endif /* __MAIN_H */
