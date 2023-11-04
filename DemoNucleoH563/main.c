#include "bsp_nucleo_h563.h"
#include "tx_api.h"
#include "mcan.h"
#include "bno055.h"

// Main Thread
#define THREAD_MAIN_STACK_SIZE 512
static TX_THREAD stThreadMain;
static uint8_t auThreadMainStack[THREAD_MAIN_STACK_SIZE];

static bool heartbeatFlag = false;
static sMCAN_Message mcanRxMessage = { 0 };
static uint8_t heartbeatData[] = { 0xDE, 0xCA, 0XF, 0xC0, 0xFF, 0xEE, 0xCA, 0xFE};

void thread_main(ULONG ctx);

int main(void)
{
    /* Initialize BSP */
    BSP_Init();

    MCAN_Init( FDCAN2, DEV_MAIN_COMPUTE, &mcanRxMessage );

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
                      4,
                      4, 
                      0, // Time slicing unused if all threads have unique priorities     
                      TX_AUTO_START);
}

void thread_main(ULONG ctx)
{
    MCAN_SetEnableIT(MCAN_ENABLE);
    bool heartbeatFlagPrevious = false;

    while( true )
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        tx_thread_sleep(1000);
        
        // If MCAN_Rx, update the heart beat enable or disable
        if ( heartbeatFlagPrevious != heartbeatFlag)
        {
            switch(heartbeatFlag)
            {
                case true:
                    MCAN_EnableHeartBeats(1000, heartbeatData); 
                    break;

                case false:
                    MCAN_DisableHeartBeats(); 
                    break;
            }
            heartbeatFlagPrevious = heartbeatFlag;
        }
    }
}

void MCAN_Rx_Handler( void )
{
    if ( mcanRxMessage.mcanID.MCAN_RX_Device == DEV_MAIN_COMPUTE || mcanRxMessage.mcanID.MCAN_RX_Device == DEV_ALL )
    {
        heartbeatFlag = (bool) mcanRxMessage.mcanData[0];
    } 
}