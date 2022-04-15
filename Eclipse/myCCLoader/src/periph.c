/*
 * periph.c
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#include <ccDebugger.h>
#include "stdbool.h"
#include "string.h"
#include "stm32f4xx.h"
#include "periph.h"
#include "dev.h"

void USART1_IRQHandler(){

	if(USART1->SR & USART_SR_RXNE){
		char c = (uint8_t)USART1->DR;

		if(stream.head + 1 == stream.tail || ((stream.head + 1 == MAX_BUF_SIZE) && stream.tail == 0))
			return; //no hay espacio
		else{
			stream.buf[stream.head] = c;

			if(++stream.head == MAX_BUF_SIZE)
				stream.head = 0;

			stream.i++;
			/*
			if(stream.head > stream.tail)
				stream.i = stream.head - stream.tail; //arrived
			else{
				stream.i = MAX_BUF_SIZE - stream.tail + stream.head; //arrived
			}
			*/
		}
	}
}

uint8_t rcv(){
	uint8_t c = 0;
	if(stream.tail != stream.head){
		c= stream.buf[stream.tail];
		if(++stream.tail == MAX_BUF_SIZE)
			stream.tail = 0;
		stream.i--;
	}
	return c;
}

void portInit(){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIODEN;

	GPIOF->MODER |= 0x01<<2*LED;
	GPIOF->OSPEEDR |= 0x03<<2*LED;
	GPIOF->BSRR = (uint32_t)(1<<(LED));

	GPIOD->MODER = 0x00;
	/*DD as input*/
	GPIOD->OSPEEDR |= 0x03<<2*DD;
	change_to_input(DD);
	low(DD);

	/*DC as output*/
	GPIOD->MODER |= 0x01<<2*DC;
	GPIOD->OSPEEDR |= 0x03<<2*DC;
	low(DC);

	/*RST as ouput*/
	GPIOD->MODER |= 0x01<<2*RST;
	GPIOD->OSPEEDR |= 0x03<<2*RST;
	low(RST);
}

void flush(){
	memset(stream.buf,0x00,sizeof(stream.buf));
	stream.i = 0;
}

void send(uint8_t res, uint8_t c0, uint8_t c1){
	USART1->DR = (uint16_t)res;
	while(!(USART1->SR & USART_SR_TC));
	USART1->DR = (uint16_t)c1;
	while(!(USART1->SR & USART_SR_TC));
	USART1->DR = (uint16_t)c0;
	while(!(USART1->SR & USART_SR_TC));
}

void uartConfig(){
	RCC->APB2ENR|= RCC_APB2ENR_USART1EN;
	RCC->AHB1ENR|= RCC_AHB1ENR_GPIOAEN;
	USART1->BRR = USART1->CR1 = USART1->CR2 = USART1->CR3 = USART1->GTPR = USART1->SR = 0;

	GPIOA->AFR[1] = 0b0111 << 4;
	GPIOA->AFR[1] |= 0b0111 << 8;

	GPIOA->PUPDR = 0;
	GPIOA->MODER |= 0x2<<9*2;
	GPIOA->MODER |= 0x2<<10*2;
	GPIOA->OSPEEDR |= 0x3<<9*2;
	GPIOA->OSPEEDR |= 0x3<<10*2;


	USART1->BRR = 0x2D9; //45.57   16*0.57 = 9//BD=fck/16*DIB
	USART1->CR1 |= (1<<13); //UE
	USART1->CR1 |= (1<<3); //TE
	USART1->CR1 |= (1<<2); //RE
	USART1->CR1 |= (1<<8); //PEIE
	USART1->CR1 |= (1<<5); //RXNEIE

	NVIC_EnableIRQ(USART1_IRQn);

}

void setSysClock(void){

  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  /* Enable HSE */
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != 0x05000));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01)
  {
    /* Select regulator voltage output Scale 1 mode */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    /* HCLK = SYSCLK / 1     PCLK2 = HCLK / 2    PCLK1 = HCLK / 4*/
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1|RCC_CFGR_PPRE2_DIV2|RCC_CFGR_PPRE1_DIV4);

    /* Configure the main PLL */
    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
                   (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);


    /* Enable the main PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till the main PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

    /* Select the main PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait till the main PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);


  }
  SystemCoreClockUpdate();

}

void delay(uint32_t d){
	asm volatile("mov r0,%0\n\t"
				 "1:sub r0,r0,#1\n\t"
				 "cmp r0,#1\n\t"
				 "bne 1b"
				 ::"r"(d):"r0"
				);
}
