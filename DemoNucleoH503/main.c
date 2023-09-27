#include "bsp_nucleo_h503.h"
#include "tx_api.h"
#include "mcan.h"
#include <stdbool.h>

#define THREAD_STACK_SIZE 1024

uint8_t thread_stack[THREAD_STACK_SIZE];
TX_THREAD thread_ptr;

static sMCAN_ID mcanID = { 0 };
static sMCAN_Message mcanMessage = {
    .mcanID = &mcanID,
    .mcanData = { 0 },
};

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
    MCAN_RegisterRX_Buf(&mcanMessage);
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

    sMCAN_ID mcanTxID = {
        .MCAN_PRIORITY = MCAN_DEBUG,
        .MCAN_RX_Device = ALL_DEVICES,
        .MCAN_TX_Device = ALL_DEVICES,
        .MCAN_TIME_STAMP = 0,
        .MCAN_TYPE = LOG,
    };

    uint8_t mcanTxData[64] = { 0 };

    sMCAN_Message mcanTxMessage = {
        .mcanID = &mcanTxID,
        .mcanData = mcanTxData,
    };

   while( true )
    {
        tx_thread_sleep(500);
        HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);

        MCAN_TX(&mcanTxMessage);
    }

    

}

bool MCAN_Rx_Handler( void )
{
    HAL_Delay(500);
    return false;
}