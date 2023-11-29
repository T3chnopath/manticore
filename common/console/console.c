#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "console.h"
#include "stm32h5xx_hal.h"
#include "tx_api.h"

static UART_HandleTypeDef * _ConsoleUart;
#define CONSOLE_PRI_MAX_CHAR 10
#define CONSOLE_MAX_CHAR 50
static char ConsoleBuff[CONSOLE_MAX_CHAR];
static TX_MUTEX ConsoleBuffMutex;

void ConsoleRegisterHandle(UART_HandleTypeDef * ConsoleUart)
{
    _ConsoleUart = ConsoleUart;
}

bool ConsoleLog(LOG_PRI pri, char message[], ...)
{
    va_list ap;
    char logBuff[CONSOLE_MAX_CHAR];
    uint16_t totalLen = strlen(message) + CONSOLE_PRI_MAX_CHAR;
   
    if ( totalLen > CONSOLE_MAX_CHAR )
    {
        return false;
    }

    // Populate beginning of Console Buffer with a priority
    switch(pri)
    {
        case LOG_ERROR:
            sprintf(logBuff, "[ERROR]   ");
            break;
    
        case LOG_WARNING:
            sprintf(logBuff, "[WARNING] ");
            break;

        case LOG_INFO:
            sprintf(logBuff, "[INFO]    ");
            break;

        case LOG_DEBUG:
            sprintf(logBuff, "[DEBUG]   ");
            break;
    }

    // Insert variadic arguments into console buffer
    va_start(ap, message);
    vsprintf(logBuff + CONSOLE_PRI_MAX_CHAR, message, ap);
    va_end(ap);

    // Log message
    ConsolePrint(logBuff);

    return true;
}

bool ConsolePrint(char message[], ...)
{
    va_list ap;
    uint8_t messageLen = strlen(message);
    
    if(messageLen > CONSOLE_MAX_CHAR)
    {
        return false;
    }

    // Acquire mutex 
    tx_mutex_get(&ConsoleBuffMutex, TX_WAIT_FOREVER); // enter critical section, suspend if mutex is locked
   
    // Clear buffer
    memset(ConsoleBuff, 0, sizeof(ConsoleBuff));

    // Insert variadic arguments into console buffer
    va_start(ap, message);
    vsprintf(ConsoleBuff, message, ap);
    va_end(ap);

    // Print message
    HAL_UART_Transmit(_ConsoleUart, ConsoleBuff, messageLen, HAL_MAX_DELAY);

    // Release console buff mutex    
    tx_mutex_put(&ConsoleBuffMutex);                  // exit critical section

    return true;
}