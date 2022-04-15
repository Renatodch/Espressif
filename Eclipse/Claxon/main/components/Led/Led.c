/*
 * Led.c
 *
 *  Created on: 22 feb. 2021
 *      Author: Renato
 */
#include "main.h"

void Led_Task(void* arg){
	int value,level=1;
	while(1)
	{
		/**************Indicador de limite de pulsos****************/
		if(pulse->xDia < (pulse->limit-CLOSE_LIMIT)){
			/*Led verde cuando la cantidad de pulsos es menor del límite cercano p<L1*/
			gpio_set_level(GREEN_LED, 1);gpio_set_level(RED_LED, 0);gpio_set_level(YELLOW_LED, 0);
		}
		else if(pulse->xDia<pulse->limit && pulse->xDia>=(pulse->limit-CLOSE_LIMIT)){
			/*Led amarillo cuando la cantidad de pulsos es mayor del límite cercano y menor del límite L2>p>=L1*/
			gpio_set_level(YELLOW_LED, 1);gpio_set_level(GREEN_LED, 0);gpio_set_level(RED_LED, 0);
		}
		else{
			/*Led rojo cuando la cantidad de pulsos es mayor al limite p>=L2*/
			gpio_set_level(RED_LED, 1);gpio_set_level(YELLOW_LED, 0);gpio_set_level(GREEN_LED, 0);
		}

		switch(state.val)
		{
			case ENVIANDO:value = 50;break;
			case CONFIGURANDO:value = 100;break;
			default: value = 1000; break;
		}

		gpio_set_level(ON_BOARD_LED, level); level = !level;

		vTaskDelay(value/portTICK_PERIOD_MS);
	}
}
void Led_Init(void){
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = (1ULL<<ON_BOARD_LED) | (1ULL<<RED_LED) | (1ULL<<GREEN_LED) | (1ULL<<YELLOW_LED);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

}
