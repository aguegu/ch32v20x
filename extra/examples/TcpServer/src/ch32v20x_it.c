#include "ch32v20x_it.h"
#include "eth_driver.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void ETH_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
  printf("HardFault_Handler\r\n");
  printf("mcause:%08x\r\n",__get_MCAUSE());
  printf("mtval:%08x\r\n",__get_MTVAL());
  printf("mepc:%08x\r\n",__get_MEPC());
  while (1);
}

void ETH_IRQHandler(void) {
  WCHNET_ETHIsr();
}

void TIM2_IRQHandler(void) {
  WCHNET_TimeIsr(WCHNETTIMERPERIOD);
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
