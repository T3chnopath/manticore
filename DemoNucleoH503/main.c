#include "bsp_nucleo_h503.h"
#include "tx_api.h"
#include "mcan.h"
#include "console.h"

// Main Thread
#define THREAD_MAIN_STACK_SIZE 2048
static TX_THREAD stThreadMain;
static uint8_t auThreadMainStack[THREAD_MAIN_STACK_SIZE];
static const uint16_t THREAD_MAIN_DELAY_MS = 10;
void thread_main(ULONG ctx);

// Blink Thread
#define THREAD_BLINK_STACK_SIZE 256
static TX_THREAD stThreadBlink;
static uint8_t auThreadBlinkStack[THREAD_BLINK_STACK_SIZE];
static const uint16_t THREAD_BLINK_DELAY_MS = 1000;
void thread_blink(ULONG ctx);

static sMCAN_Message mcanRxMessage = { 0 };
static const uint16_t HEARTBEAT_DELAY_MS = 1000;
static uint8_t heartbeatData[] = {0xDE, 0xCA, 0xFF, 0xC0, 0xFF, 0xEE, 0xCA, 0xFE};
static bool heartbeatFlag = false;

// Serial Console Testing
extern UART_HandleTypeDef ConsoleUart;

int main(void)
{
       tx_kernel_enter();
   }

void tx_application_define(void *first_unused_memory)
{
    // Create main thread
    tx_thread_create( &stThreadMain, 
                     "thread_main", 
                      thread_main, 
                      0, 
                      auThreadMainStack, 
                      THREAD_MAIN_STACK_SIZE, 
                      0,
                      0, 
                      0, 
                      TX_AUTO_START);

    // Create blink thread
    tx_thread_create( &stThreadBlink, 
                     "thread_blink", 
                      thread_blink, 
                      0, 
                      auThreadBlinkStack, 
                      THREAD_BLINK_STACK_SIZE, 
                      10,
                      10, 
                      0, 
                      TX_AUTO_START);
}

void thread_main(ULONG ctx)
{
    bool heartbeatFlagPrevious = false;

    // Init BSP
    BSP_Init();

    // Init App Layer
    MCAN_Init( FDCAN1, DEV_MAIN_COMPUTE, &mcanRxMessage );
    MCAN_SetEnableIT(MCAN_ENABLE);

    ConsoleRegisterHandle(&ConsoleUart);
    
    while( true )
    {
        // If MCAN_Rx, update the heart beat enable or disable
        if ( heartbeatFlagPrevious != heartbeatFlag)
        {
            switch(heartbeatFlag)
            {
                case true:
                    MCAN_EnableHeartBeats(HEARTBEAT_DELAY_MS, heartbeatData); 
                    break;

                case false:
                    MCAN_DisableHeartBeats(); 
                    break;
            }
            heartbeatFlagPrevious = heartbeatFlag;
        }
        
        tx_thread_sleep(THREAD_MAIN_DELAY_MS);
    }
}


{
    while(true)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        tx_thread_sleep(THREAD_BLINK_DELAY_MS);
    }
}

void MCAN_Rx_Handler( void )
{
    if ( mcanRxMessage.mcanID.MCAN_RX_Device == DEV_DEPLOYMENT || mcanRxMessage.mcanID.MCAN_RX_Device == DEV_ALL )
    {
        heartbeatFlag = (bool) mcanRxMessage.mcanData[0];
    } 
}