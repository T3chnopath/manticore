#include "bsp_nucleo_h503.h"
#include "tx_api.h"
#include "stdbool.h"

#define THREAD_STACK_SIZE 1024

uint8_t thread_stack[THREAD_STACK_SIZE];
TX_THREAD thread_ptr;

void my_thread_entry(ULONG ctx);

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    BSP_SystemClock_Config();

    /* Initialize all configured peripherals */
    BSP_GPIO_Init();

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
        HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);
    }

}