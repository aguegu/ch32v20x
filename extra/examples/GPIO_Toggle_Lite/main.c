#include "ch32v20x_conf.h"

static uint8_t  p_us = 0;
static uint16_t p_ms = 0;

void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Delay_Init(void)
{
    p_us = SystemCoreClock / 8000000;
    p_ms = (uint16_t)p_us * 1000;
}

void Delay_Ms(uint32_t n)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)n * p_ms;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while((SysTick->SR & (1 << 0)) != (1 << 0));
    SysTick->CTLR &= ~(1 << 0);
}

int main(void)
{
    u8 i = 0;

    SystemCoreClockUpdate();
    Delay_Init();
    GPIO_Toggle_INIT();

    while(1)
    {
        Delay_Ms(100);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, i);
        i ^= 1;
    }
}
