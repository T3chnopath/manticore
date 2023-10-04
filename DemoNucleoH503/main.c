#include <stdbool.h>
#include <string.h>

#include "bsp_nucleo_h503.h"
#include "tx_api.h"
#include "mcan.h"

#define THREAD_STACK_SIZE 1024

uint8_t thread_stack[THREAD_STACK_SIZE];
TX_THREAD thread_ptr;

static uint8_t mcanTxData[64];

static sMCAN_Message mcanRxMessage = { 0 };
static bool ToggleFlag = false;
static bool TransmitFlag = false;
static uint8_t count = 0;

void my_thread_entry(ULONG ctx);

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    BSP_SystemClock_Config();
    
    /* Initialize Peripheral Clocks */
    BSP_PeriphClock_Config();

    /* Initialize all configured peripherals */
    BSP_GPIO_Init();

    MCAN_PeriphConfig( FDCAN1_PERIPH, MAIN_COMPUTE);
    MCAN_RegisterRX_Buf(&mcanRxMessage);
    MCAN_StartRX_IT();


    tx_kernel_enter();
   }

void tx_application_define(void *first_unused_memory)
{
    /* Create my_thread! */
    tx_thread_create( &thread_ptr, 
                     "my_thread", 
                      my_thread_entry, 
                      0x1234, 
                      first_unused_memory, 
                      THREAD_STACK_SIZE, 
                      1,
                      1, 
                      0, // Time slicing unused if all threads have unique priorities     
                      TX_AUTO_START);
}
void my_thread_entry(ULONG initial_input)
{

   while( true )
    {
        if ( ToggleFlag )
        {
            ToggleFlag = false;
            mcanTxData[0] = count++;
            MCAN_TX( MCAN_EMERGENCY, LOG, MAIN_COMPUTE, mcanTxData);

            for (uint8_t i = 0; i < mcanRxMessage.mcanData[1]; i++)
            {
                HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET);
                tx_thread_sleep(100); 
                HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_SET);
                tx_thread_sleep(100);
            }
        } 

        if ( TransmitFlag )
        {
            MCAN_TX( MCAN_EMERGENCY, LOG, MAIN_COMPUTE, mcanTxData);
            tx_thread_sleep(1000);
        }
    }
}

void MCAN_Rx_Handler( void )
{
    if ( mcanRxMessage.mcanID.MCAN_RX_Device == MAIN_COMPUTE || mcanRxMessage.mcanID.MCAN_RX_Device == ALL_DEVICES )
    {
        HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);

        if (mcanRxMessage.mcanData[0])
        {
            HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_SET);
        }

        else if(!mcanRxMessage.mcanData[0])
        {
            HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET);

            if (mcanRxMessage.mcanData[1])
            ToggleFlag = true;
        }

        TransmitFlag = mcanRxMessage.mcanData[2];
    } 
}