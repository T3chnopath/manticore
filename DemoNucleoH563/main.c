#include "bsp_nucleo_h563.h"

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    BSP_SystemClock_Config();

    /* Initialize all configured peripherals */
    BSP_GPIO_Init();
  
    while (1)
    {
        HAL_Delay(1000);
        HAL_GPIO_TogglePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin);
    }
}
