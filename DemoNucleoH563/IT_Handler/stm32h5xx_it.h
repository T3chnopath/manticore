#ifndef __STM32H5xx_IT_H
#define __STM32H5xx_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void FDCAN1_IT0_IRQHandler(void);
void FDCAN2_IT0_IRQHandler(void);
void _tx_timer_interrupt(void);

#endif /* __STM32H5xx_IT_H */