#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h5xx_hal.h"

#define CONSOLE_NAME_MAX_CHAR 15
#define CONSOLE_HELP_MAX_CHAR 30

typedef enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
} LOG_PRI;

typedef struct
{
    char name[CONSOLE_NAME_MAX_CHAR];
    char help[CONSOLE_HELP_MAX_CHAR];
    void (*command)();
} ConsoleComm_t;

void ConsoleRegisterHandle(UART_HandleTypeDef * ConsoleUart);
bool ConsoleLog(LOG_PRI pri, char message[], ...);
bool ConsolePrint(char message[], ...);
bool ConsoleRegisterComm(ConsoleComm_t * command);


#endif