#include "stm32h5xx_it.h"
#include "stm32h5xx_hal.h"

#include <stdint.h>
#include "tx_api.h"

void SysTick_Handler(void)
{
  _tx_timer_interrupt();
}

uint32_t HAL_GetTick(void)
{
  return _tx_time_get();
}