#include "stm32h5xx_it.h"
#include "stm32h5xx_hal.h"

void SysTick_Handler(void)
{
  HAL_IncTick();
}