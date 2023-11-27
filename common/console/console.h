#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h5xx_hal.h"

typedef enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
} LOG_PRI;

void ConsoleRegisterHandle(UART_HandleTypeDef * ConsoleUart);
bool ConsoleLog(LOG_PRI pri, char message[], ...);
#endif