#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "console.h"
#include "stm32h5xx_hal.h"


static UART_HandleTypeDef * _ConsoleUart;
#define CONSOLE_PRI_MAX_CHAR 10
#define CONSOLE_MAX_CHAR 50
static char ConsoleBuff[CONSOLE_MAX_CHAR];

void ConsoleRegisterHandle(UART_HandleTypeDef * ConsoleUart)
{
    _ConsoleUart = ConsoleUart;
}

bool ConsoleLog(LOG_PRI pri, char message[], ...)
{
    va_list ap;
    uint16_t totalLen = strlen(message) + CONSOLE_PRI_MAX_CHAR;
   
    if ( totalLen > CONSOLE_MAX_CHAR )
    {
        return false;
    }
    
     // Clear buffer
    memset(ConsoleBuff, 0, sizeof(ConsoleBuff));

    // Populate beginning of Console Buffer with a priority
    switch(pri)
    {
        case LOG_ERROR:
            sprintf(ConsoleBuff, "[ERROR]   ");
            break;
    
        case LOG_WARNING:
            sprintf(ConsoleBuff, "[WARNING] ");
            break;

        case LOG_INFO:
            sprintf(ConsoleBuff, "[INFO]    ");
            break;

        case LOG_DEBUG:
            sprintf(ConsoleBuff, "[DEBUG]   ");
            break;
    }

    // Insert variadic arguments into console buffer
    va_start(ap, message);
    vsprintf(ConsoleBuff + CONSOLE_PRI_MAX_CHAR, message, ap);
    va_end(ap);

    // Log message
    HAL_UART_Transmit(_ConsoleUart, ConsoleBuff, CONSOLE_MAX_CHAR, HAL_MAX_DELAY);

    return true;
}