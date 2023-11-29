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

#define MAX_COMMANDS 10
static uint8_t registeredCommands = 0;
static ConsoleComm_t *ConsoleCommArr[MAX_COMMANDS] = {0};

// Static function Declarations
bool _tokenizeInput(char input[], char *argv[]); // Accept the input from terminal and set argv to the tokens. 
                                                 // Returns number of processed arguments.

bool _addComm(ConsoleComm_t *comm);              // Add command to the commArr
int8_t _findComm(char commName[]);               // Find command in the commArr by name

// Static Function Definitions
bool _tokenizeInput(char input[], char *argv[])
{
    const char delimit = " ";
    char *token;
    uint8_t argvIndex = 0;

    // Get first token
    token = strtok(input, &delimit);

    // Step through remaining tokens
    while( token != NULL )
    {
        argv[argvIndex++] = token;
        token = strtok(NULL, &delimit);
    }

    return ++argvIndex; 
}


bool _addComm(ConsoleComm_t *comm)
{
    if( registeredCommands == MAX_COMMANDS - 1)
    {
        return false;
    }

    ConsoleCommArr[registeredCommands++] = comm;
    return true;
}

int8_t _findComm(char commName[])
{
    for(uint8_t i = 0; i <= registeredCommands)
    {
        if( strcmp(ConsoleCommArr[i]->name, commName ) )
        {
            return i;
        }
    }

    return -1;
}

// Global Functions
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

void ConsoleRegisterCommand(ConsoleComm_t_t * command)
{

}