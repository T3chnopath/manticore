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
static char ConsoleOutBuff[CONSOLE_MAX_CHAR];
static TX_MUTEX ConsoleOutBuffMutex;

#define MAX_COMMANDS 10
#define MAX_COMMAND_ARGS 4
static uint8_t registeredCommands = 0;
static ConsoleComm_t *ConsoleCommArr[MAX_COMMANDS] = {0};
static char *argvBuff[MAX_COMMAND_ARGS] = {0};

static const char unlockString[] = "console";
static const uint16_t CONSOLE_IN_DELAY = 200;
static const char ENTER = '\r';

static bool enableLogging = false;

static volatile char UART_RxChar;
static volatile bool newChar = false;

static char spaceBuff[CONSOLE_NAME_MAX_CHAR];
static char inputBuff[CONSOLE_MAX_CHAR];

static char *token;

// Console Thread
#define THREAD_CONSOLE_STACK_SIZE 2048
static TX_THREAD stThreadConsole;
static uint8_t auThreadConsoleStack[THREAD_CONSOLE_STACK_SIZE];
static const uint16_t THREAD_CONSOLE_DELAY_MS = 10;
void thread_console(ULONG ctx);


// Static function Declarations

char _consoleInChar();                                    // [BLOCKING] wait for a single character
bool _consoleInFilter(char input[], char filterChar);     // [BLOCKING] populate input buff with the serial input. 
                                                          // terminate if filter char is detected.

void _consoleUnlock(void);                          // [BLOCKING] wait until unlock string is entered

uint8_t _tokenizeInput(char input[]); // Accept the input from terminal and set argv to the tokens. 
                                                    // Returns number of processed arguments.

bool _addComm(ConsoleComm_t *comm);                 // Add command to the commArr
int8_t _findComm(char commName[]);                  // Find command in the commArr by name
void _exeComm(ConsoleComm_t *comm);                 // Execute command
bool _processInput(char input[]);                    // tokenize string input and process it if there is a valid command

char _consoleInChar(void)
{
    while(!newChar);
    newChar = false;
    return UART_RxChar;
}

bool _consoleInFilter(char input[], char filterChar)
{
    uint16_t inputIndex = 0; 
    uint8_t inputChar = ' ';
    
    HAL_StatusTypeDef uartStatus;

    // Populate input until the filter char is detected
    while (true) 
    {
        inputChar = _consoleInChar();
        ConsolePrint("%c", inputChar);

        if(inputChar != filterChar)
        {
            input[inputIndex++] = inputChar;
        }

        // Break if filter detected
        if (inputChar == filterChar)
        {
            return true;
        }

        // Break if max characters
        else if (inputIndex == CONSOLE_MAX_CHAR)
        {
            return false; 
        }
    };
}

void _consoleUnlock(void)
{
    uint8_t len = strlen(unlockString);
    char inBuff[sizeof(unlockString)/sizeof(char)];
    bool unlock = false;
    bool validChar = true;

    while(!unlock)
    {
        for(uint8_t i = 0; i < len; i++)
        {
            validChar = _consoleInFilter(inBuff, unlockString[i]);

            // if char is invalid, restart the for loop
            if(!validChar)
            {
                break;
            } 
        }

        // if all characters are valid, break out of while loop
        if(validChar)
        {
            enableLogging = true;
            unlock = true;
        }
    }
    ConsolePrint("\r\n");
}

uint8_t _tokenizeInput(char input[])
{
    const char delimit = ' ';
    uint8_t argvIndex = 0;

    // Get first token
    token = strtok(input, &delimit);

    // Step through remaining tokens
    while( token != NULL )
    {
        argvBuff[argvIndex++] = token;
        token = strtok(NULL, &delimit);
    }

    return argvIndex - 1; 
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
    for(uint8_t i = 0; i < registeredCommands; i++)
    {
        if( strcmp(ConsoleCommArr[i]->name, commName ) == 0 )
        {
            return i;
        }
    }

    return -1;
}

void _exeComm(ConsoleComm_t *comm)
{
    comm->command(argvBuff);
}

bool _processInput(char input[])
{
    uint8_t commIndex = 0;
    uint8_t numArgs = 0;
    ConsoleComm_t *comm;
    // populate argv with the command name and arguments
    numArgs = _tokenizeInput(input);

    // find if the command is in the registered list
    commIndex = _findComm(argvBuff[0]);

    // If index not found, return;
    if (commIndex == -1)
    {
        return false;
    }

    comm = ConsoleCommArr[commIndex];
    if(comm->argumentCount != numArgs)
    {
        return false;
    }
    
    _exeComm(comm);
    return true;
}

// Global Functions
void ConsoleInit(UART_HandleTypeDef * ConsoleUart)
{
    _ConsoleUart = ConsoleUart;

    // Start UART Rx interrupts
    HAL_UART_Receive_IT(_ConsoleUart, &UART_RxChar, sizeof(char)); 

    tx_thread_create( &stThreadConsole, 
        "thread_console", 
        thread_console, 
        0, 
        auThreadConsoleStack, 
        THREAD_CONSOLE_STACK_SIZE, 
        2,
        2, 
        0, 
        TX_AUTO_START);

}

bool ConsoleLog(LOG_PRI pri, char message[], ...)
{
    va_list ap;
    char logBuff[CONSOLE_MAX_CHAR];
    uint16_t totalLen = strlen(message) + CONSOLE_PRI_MAX_CHAR;
   
    if ( totalLen > CONSOLE_MAX_CHAR || !enableLogging)
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
    if(messageLen > CONSOLE_MAX_CHAR)
    {
        return false;
    }

    // Acquire mutex 
    tx_mutex_get(&ConsoleOutBuffMutex, TX_WAIT_FOREVER); // enter critical section, suspend if mutex is locked
   
    // Clear buffer
    memset(ConsoleOutBuff, 0, sizeof(ConsoleOutBuff));

    // Insert variadic arguments into console buffer
    va_start(ap, message);
    vsprintf(ConsoleOutBuff, message, ap);
    va_end(ap);
    
    // Force NULL termination
    ConsoleOutBuff[CONSOLE_MAX_CHAR] = (char) NULL;

    // Get length of message after variadic arguments inserted
    constructedMessageLen = strlen(ConsoleOutBuff);
    if ( constructedMessageLen > CONSOLE_MAX_CHAR)
    {
        return false;
    }

    // Print
    HAL_UART_Transmit(_ConsoleUart, (uint8_t *) ConsoleOutBuff, constructedMessageLen, HAL_MAX_DELAY);
    
    // Release console buff mutex    
    tx_mutex_put(&ConsoleOutBuffMutex);                  // exit critical section

    return true;
}

bool ConsoleRegisterComm(ConsoleComm_t * command)
{
    return (bool) _addComm(command);
}

void thread_console(ULONG ctx)
{
    // Wait until unlock command is issued    
    bool validCommand = false;
    
    _consoleUnlock();
        
    ConsolePrint("Welcome to manticore Serial Console \r\n");
    ConsolePrint("Please select a command: \r\n");
    
    for(uint8_t i = 0; i < registeredCommands; i++)
    {
        // Print name 
        ConsolePrint("%s", ConsoleCommArr[i]->name);

        // Right align help
        for(char j = 0; j < CONSOLE_NAME_MAX_CHAR - strlen(ConsoleCommArr[i]->name); j++ )
        {
            sprintf(spaceBuff + j, " ");
        }

        // Print help
        ConsolePrint("%s    %s \r\n", spaceBuff, ConsoleCommArr[i]->help);
    }

    while(true)
    {
        memset(inputBuff, 0, sizeof(inputBuff));
        _consoleInFilter(inputBuff, ENTER);
        if(_processInput(inputBuff))

        ConsolePrint("\r\n");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
    newChar = true;
    HAL_UART_Receive_IT(_ConsoleUart, &UART_RxChar, sizeof(char)); 
}