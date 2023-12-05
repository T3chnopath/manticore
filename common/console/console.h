#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h5xx_hal.h"
#include "native_commands.h"

#define CONSOLE_NAME_MAX_CHAR 25
#define CONSOLE_HELP_MAX_CHAR 50

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
    uint8_t argumentCount;
    void (*command)(char *argv[]);
} ConsoleComm_t;

void ConsoleInit(UART_HandleTypeDef *ConsoleUart);

char ConsoleInChar(void);
char ConsoleInCharFilter(char charFilter[], uint8_t filterSize);    // Wait for input char in filter

// Populate inString when ENTER pressed or max length reached
void ConsoleInString(char inString[], uint8_t stringMaxLen);                             

// Populate inString until a character in the filter is matched. Will return the value of the filter matched.
// Will return NULL if terminated via max string length or enter;
char ConsoleInStringFilter(char inString[], uint8_t stringMaxLen, char charFilter[], uint8_t filterSize);

void ConsoleClear(void);
bool ConsolePrint(char message[], ...);
bool ConsoleLog(LOG_PRI pri, char message[], ...);

bool ConsoleRegisterComm(ConsoleComm_t *command);

#endif