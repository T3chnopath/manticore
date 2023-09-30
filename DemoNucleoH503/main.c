#include "bsp_nucleo_h503.h"
#include "tx_api.h"
#include "mcan.h"
#include <stdbool.h>
#include <string.h>

#define THREAD_STACK_SIZE 1024

uint8_t thread_stack[THREAD_STACK_SIZE];
TX_THREAD thread_ptr;

static uint8_t mcanTxData[64] = {  177, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                            21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                            31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                            41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                            51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                            61, 62, 63, 64 };


static sMCAN_Message mcanRxMessage;


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
                      4,
                      4, 
                      1, 
                      TX_AUTO_START);

}

void my_thread_entry(ULONG initial_input)
{

   while( true )
    {
        HAL_Delay(1000);
       
        MCAN_TX( MCAN_EMERGENCY, LOG, ALL_DEVICES, mcanTxData);

    }

}

bool MCAN_Rx_Handler( void )
{
    if ( mcanRxMessage.mcanID.MCAN_RX_Device == ALL_DEVICES)
    {
        if ( mcanRxMessage.mcanData[0] == 177 )
        {
            HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);
            return true;
        }
    } 

    return false;
}