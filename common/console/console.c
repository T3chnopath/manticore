#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "console.h"
#include "native_commands.h"
#include "stm32h5xx_hal.h"
#include "tx_api.h"

static UART_HandleTypeDef * _ConsoleUart;

#define CONSOLE_PRI_MAX_CHAR 10
#define CONSOLE_MAX_CHAR 100
static char ConsoleOutBuff[CONSOLE_MAX_CHAR];
static TX_MUTEX ConsoleOutBuffMutex;

#define MAX_COMMANDS 10
#define MAX_COMMAND_ARGS 4
#define ARG_STRING_BUFF_SIZE 30
#define COMM_HELP_SPACING 15
static uint8_t registeredCommands = 0;
static ConsoleComm_t *ConsoleCommArr[MAX_COMMANDS] = {0};


static char *argvBuff[MAX_COMMAND_ARGS] = {0};
static char argBuff[MAX_COMMAND_ARGS][ARG_STRING_BUFF_SIZE] = {0};
static uint8_t _argIndex = 0;

static const char unlockString[] = "console";
static const char ENTER = '\r';
static const char DEL = 127;
static const char BACKSPACE = '\b';
static const uint8_t IN_CHAR_SLEEP = 10;

static bool enableLogging = false;

static volatile char UART_RxChar;
static volatile bool newChar = false;

// Console Thread
#define THREAD_CONSOLE_STACK_SIZE 4096
static TX_THREAD stThreadConsole;
static uint8_t auThreadConsoleStack[THREAD_CONSOLE_STACK_SIZE];
void thread_console(ULONG ctx);


// Static function Declarations

void _consoleUnlock(void);                          // [BLOCKING] wait until unlock string is entered

ConsoleComm_t *_getCommand(void);                       // [BLOCKING] process input, returns command pointer.
                                                    // returns NULL if invalid command.

                                                    // Returns number of processed arguments.

void _initArgvBuff(void);
void _exeComm(ConsoleComm_t *comm);                 // Execute command
int8_t _findCommIndex(char commName[]);                  // Find command in the commArr by name

void _consoleUnlock(void)
{
    uint8_t len = strlen(unlockString);
    char inBuff[sizeof(unlockString)/sizeof(char)];
    bool equalFlag = false;

    while(true)
    {
        for(uint8_t i = 0; i < len; i++)
        {
            inBuff[i] = ConsoleInCharFilter(unlockString, len); 
            if (unlockString[i] != inBuff[i])
            {
                equalFlag = false;
                break;
            }
            else
            {
                equalFlag = true;
            }
        }

        // If equal flag, perform a strcmp to verify consecutivity
        if(equalFlag && strcmp(unlockString, inBuff) == 0)
        {
            break;
        }
    }
    
    ConsolePrint("\r\n\r\n"); 
}

ConsoleComm_t *_getCommand(void)
{
    char inStringBuffer[CONSOLE_MAX_CHAR] = {0};
    _argIndex = 0;
    int8_t commIndex = 0;
    char *token;

    // Clear previous arguments
    memset(argBuff, 0, sizeof(argBuff));

    // Wait for string input
    ConsoleInString(inStringBuffer, CONSOLE_MAX_CHAR);

    // Get tokens and assign them to arg buffer 
    token = strtok(inStringBuffer, " ");
    while (token != NULL && _argIndex < MAX_COMMAND_ARGS) {
        strcpy(argBuff[_argIndex++], token);
        token = strtok(NULL, " "); // Get the next substring
    }

    // Search for command name from first argument
    commIndex = _findCommIndex(argBuff[0]);
    if( commIndex != -1)
    {
        return ConsoleCommArr[commIndex];
    }

    return NULL;
}


int8_t _findCommIndex(char commName[])
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

void _initArgvBuff(void)
{
    for(uint8_t i = 0; i < MAX_COMMAND_ARGS; i++)
    {
        argvBuff[i] = argBuff + i;
    }
}

void _exeComm(ConsoleComm_t *comm)
{
    comm->command(argvBuff);
}

// Global Functions
void ConsoleInit(UART_HandleTypeDef * ConsoleUart)
{
    _ConsoleUart = ConsoleUart;

    // Initialize buffers
    _initArgvBuff();

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

    ConsoleRegisterNativeCommands();
}

// Char functions
char ConsoleInChar(void)
{
    // Loiter until new character 
    while(!newChar)
    {
        tx_thread_sleep(IN_CHAR_SLEEP);
    }

    // If delete or backspace, print a backspace
    if(UART_RxChar == DEL || UART_RxChar == BACKSPACE)
    {
        ConsolePrint("%c", BACKSPACE);
        ConsolePrint(" ");
        ConsolePrint("%c", BACKSPACE);
        UART_RxChar = BACKSPACE;
    }
    else
    {
        ConsolePrint("%c", UART_RxChar);
    }

    newChar = false;
    return UART_RxChar;
}

char ConsoleInCharFilter(char charFilter[], uint8_t filterSize)
{
    char inChar;

    // Wait until input is within the filter 
    while(true)
    {
        inChar = ConsoleInChar();

        for(uint8_t i = 0; i < filterSize; i++)
        {
            if(charFilter[i] == inChar)
            {
                return inChar;
            }
        }
    }
}

// Wait for max length or enter key
void ConsoleInString(char inString[], uint8_t stringMaxLen)
{
    uint8_t stringIndex = 0;
    char inChar;

    while(true)
    {
        inChar = ConsoleInChar(); 

        // Break if ENTER key or if max length reached
        if(inChar == ENTER || stringIndex == stringMaxLen)
        {
            break;
        }   

        // Remove character if backspace
        if(inChar == BACKSPACE && stringIndex > 0)
        {
            inString[--stringIndex] = (char) NULL;
        }

        // Otherwise assign string 
        else if(inChar != BACKSPACE)
        {
            inString[stringIndex++] = inChar;
        }
    }
}

char ConsoleInStringFilter(char inString[], uint8_t stringMaxLen, char charFilter[], uint8_t filterSize)
{
    uint8_t stringIndex = 0;
    char inChar;
    while(true)
    {
        inChar = ConsoleInChar(); 

        // Break if ENTER key or if max length reached
        if(stringIndex == stringMaxLen)
        {
            return (char) NULL;
        }   

        // return enter if detected (natively filtered)
        if(inChar == ENTER)
        {
            return ENTER;
        }

        // Process user filters 
        for(uint8_t i = 0; i < filterSize; i++)
        {
            if(charFilter[i] == inChar)
            {
                return inChar;
            }
        }

        if(inChar == BACKSPACE && stringIndex > 0)
        {
            inString[stringIndex] == (char) NULL;
            stringIndex--;
        }

        // Otherwise assign string 
        else
        {
            inString[stringIndex++] = inChar;
        }
    }
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

void ConsoleClear(void)
{
    for(uint8_t i = 0; i < CONSOLE_MAX_CHAR / 4; i++)
    {
        ConsolePrint("    ");
    }
    ConsolePrint("\r\n");
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
    if( registeredCommands == MAX_COMMANDS - 1)
    {
        return false;
    }

    ConsoleCommArr[registeredCommands++] = command;
    return true;

}

void thread_console(ULONG ctx)
{
    ConsoleComm_t *newCommand = {0};
    char spaceBuff[COMM_HELP_SPACING] = {0};

    ConsoleClear();
    _consoleUnlock();
        
    ConsolePrint("Welcome to manticore Serial Console \r\n\r\n");

    while(true)
    { 
        // Print commands
        ConsolePrint("Registered Commands \r\n");
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
        ConsolePrint("\r\n");

        // Execute commands
        newCommand = _getCommand();
        ConsolePrint("\r\n\r\n");
        if(newCommand == NULL)
        {
            ConsolePrint("Invalid Command!");
        }
        else if ( (_argIndex - 1) != newCommand->argumentCount)
        {
            ConsolePrint("Invalid argument Count. %s has %d arguments.", newCommand->name, newCommand->argumentCount);
        }
        else
        {
            _exeComm(newCommand);
        }
        
        ConsolePrint("\r\n\r\n");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
    newChar = true;
    HAL_UART_Receive_IT(_ConsoleUart, &UART_RxChar, sizeof(char)); 
}