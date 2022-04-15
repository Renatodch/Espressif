/*
 * periph.h
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#ifndef INC_PERIPH_H_
#define INC_PERIPH_H_

#include "stm32f4xx.h"
#include "stdbool.h"

#define PLL_M      8
#define PLL_Q      7
#define PLL_N      336
#define PLL_P      2

/*GPIOD*/
#define DD	9
#define DC	11
#define RST	10

/*GPIOF*/
#define LED 9

#define PORT GPIOD
#define high(x)	 PORT->BSRR = (uint32_t)(1UL<<x)
#define low(x)	 PORT->BSRR = (uint32_t)(1UL<<(x+0x10))
#define input(x) PORT->IDR & (uint32_t)(1UL<<x)
#define change_to_input(x)	PORT->MODER &= (uint32_t)(~(0x03<<2*x));
#define change_to_output(x) PORT->MODER |= (uint32_t)(0x01<<2*x);


uint8_t rcv();
void send(uint8_t res, uint8_t c1, uint8_t c0);
void delay(uint32_t d);
void setSysClock();
void uartConfig();
void portInit();
void flush();


#endif /* INC_PERIPH_H_ */
