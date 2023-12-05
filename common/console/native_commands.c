#include "native_commands.h"
#include "console.h"
#include "mcan.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PRI_FIELD_MAX 15 //17
#define CAT_FIELD_MAX 19
#define DEV_FIELD_MAX 19

// Static Variables
static bool _newRxMessage = false;
static sMCAN_Message _mcanRxMessage = {0};

// Static Function Declarations
static void _helloWorld(char *argv[]);
static void _candump(char *argv[]);
static void _mcandump(char *argv[]);

// Static Data Structions
ConsoleComm_t _commHelloWorld = {
    "HelloWorld",
    "Prints hello world",
    0,
    _helloWorld,
};

ConsoleComm_t _commCandump = {
    "candump",
    "View raw received CAN message",
    0,
    _candump,
};

ConsoleComm_t _commMcandump = {
    "mcandump",
    "View decoded MCAN messages",
    0,
    _mcandump,
};

// Static Function Definitions
static void _helloWorld(char *argv[])
{
    ConsolePrint("Hello world!");
}

static void _candump(char *argv[])
{
    uint32_t CanID = 0;
    while(true)
    {
        if(_newRxMessage)
        {
            // Print ID
            MCAN_Conv_ID_To_Uint32(&(_mcanRxMessage.mcanID), &CanID );
            ConsolePrint("%08X ", CanID);  

            // Print Payload
            ConsolePrint("  [8] ");
            for(uint8_t i = 0; i < 8; i++)
            {
                ConsolePrint ("%02X ", _mcanRxMessage.mcanData[i]);
            } 

            ConsolePrint("\r\n");

            _newRxMessage = false;
        }
    }
}

static void _mcandump(char *argv[])
{
    static const uint8_t priFieldMax = PRI_FIELD_MAX;
    static char priField[PRI_FIELD_MAX] = {0};
    priField[PRI_FIELD_MAX-1] = NULL;
   
    static const uint8_t catFieldMax = CAT_FIELD_MAX;
    static char catField[CAT_FIELD_MAX] = {0};
    catField[CAT_FIELD_MAX-1] = NULL;

    static const uint8_t devFieldMax = DEV_FIELD_MAX;
    static char rxDevField[DEV_FIELD_MAX] = "RX_";
    rxDevField[DEV_FIELD_MAX-1] = NULL;

    static char txDevField[DEV_FIELD_MAX] = "TX_";
    txDevField[DEV_FIELD_MAX-1] = NULL;
    
    while(true)
    {
        if(_newRxMessage)
        {
            // Construct Priority
            sprintf(priField, "%-*s", priFieldMax, MCAN_Pri_String(_mcanRxMessage.mcanID.MCAN_PRIORITY));

            // Construct Category
            sprintf(catField, "%-*s", catFieldMax, MCAN_Cat_String(_mcanRxMessage.mcanID.MCAN_CAT));

            // Construct ID
            
            sprintf(txDevField + 3, "%-*s", devFieldMax-3, MCAN_Dev_String(_mcanRxMessage.mcanID.MCAN_TX_Device));
            sprintf(rxDevField + 3, "%-*s", devFieldMax-3, MCAN_Dev_String(_mcanRxMessage.mcanID.MCAN_RX_Device));

            // Print message
            ConsolePrint("%04d  %s %s %s %s", _mcanRxMessage.mcanID.MCAN_TimeStamp, priField, catField, txDevField, rxDevField);  

            // Print Payload
            ConsolePrint("[8] ");
            for(uint8_t i = 0; i < 8; i++)
            {
                ConsolePrint ("%02X ", _mcanRxMessage.mcanData[i]);
            } 

            ConsolePrint("\r\n");

            _newRxMessage = false;
 
        }
    }
}


// Command Registration
void ConsoleRegisterNativeCommands(void)
{
    ConsoleRegisterComm(&_commHelloWorld);
    ConsoleRegisterComm(&_commCandump);
    ConsoleRegisterComm(&_commMcandump);
}

// Called when CAN message is received
void MCAN_RX_GetLatest( sMCAN_Message mcanRxMessage )
{
    _newRxMessage = true;
    _mcanRxMessage = mcanRxMessage;
}