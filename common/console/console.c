#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "console.h"
#include "stm32h5xx_hal.h"
#include "tx_api.h"

static UART_HandleTypeDef * _ConsoleUart;

#define CONSOLE_PRI_MAX_CHAR 10
#define CONSOLE_MAX_CHAR 100
static char ConsoleBuff[CONSOLE_MAX_CHAR];
static TX_MUTEX ConsoleBuffMutex;

#define MAX_COMMANDS 10
static uint8_t registeredCommands = 0;
static ConsoleComm_t *ConsoleCommArr[MAX_COMMANDS] = {0};

static bool enableConsolePrint = false;

// Static function Declarations
uint8_t _tokenizeInput(char input[], char *argv[]); // Accept the input from terminal and set argv to the tokens. 
                                                 // Returns number of processed arguments.

bool _addComm(ConsoleComm_t *comm);              // Add command to the commArr
int8_t _findComm(char commName[]);               // Find command in the commArr by name

// Static Function Definitions
uint8_t _tokenizeInput(char input[], char *argv[])
{
    const char delimit = ' ';
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
    for(uint8_t i = 0; i <= registeredCommands; i++)
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
    uint8_t constructedMessageLen;
    
    // Only print if enabled through the serial console
    if(messageLen > CONSOLE_MAX_CHAR || !enableConsolePrint)
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
    
    // Force NULL termination
    ConsoleBuff[CONSOLE_MAX_CHAR] = NULL;

    // Get length of message after variadic arguments inserted
    constructedMessageLen = strlen(ConsoleBuff);
    if ( constructedMessageLen > CONSOLE_MAX_CHAR)
    {
        return false;
    }

    // Print
    HAL_UART_Transmit(_ConsoleUart, (uint8_t *) ConsoleBuff, constructedMessageLen, HAL_MAX_DELAY);
    
    // Release console buff mutex    
    tx_mutex_put(&ConsoleBuffMutex);                  // exit critical section

    return true;
}

bool ConsoleRegisterComm(ConsoleComm_t * command)
{
    return (bool) _addComm(command);
}

void ConsoleMenu(void)
{
    enableConsolePrint = true; 
    ConsolePrint("Welcome to manticore Serial Console \r\n");
    ConsolePrint("Please select a command: \r\n");
    char spaceBuff[CONSOLE_NAME_MAX_CHAR];

    for(uint8_t i = 0; i < registeredCommands; i++)
    {
        ConsolePrint("%s", ConsoleCommArr[i]->name);

        // Right align command description
        for(char j = 0; j < CONSOLE_NAME_MAX_CHAR - strlen(ConsoleCommArr[i]->name); j++ )
        {
            sprintf(spaceBuff + j, " ");
        }

        ConsolePrint("%s    %s \r\n", spaceBuff, ConsoleCommArr[i]->help);
    }

    ConsolePrint("\r\n");
    enableConsolePrint = false;
}