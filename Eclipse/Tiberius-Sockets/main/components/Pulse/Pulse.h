/*
 * Pulse.h
 *
 *  Created on: 29 feb. 2020
 *      Author: Renato
 */

#ifndef MAIN_COMPONENTS_PULSE_PULSE_H_
#define MAIN_COMPONENTS_PULSE_PULSE_H_

#include "main.h"


#define GPIO_INPUT    4
#define CLAXON_INPUT  (1ULL<<GPIO_INPUT)
#define ESP_INTR_FLAG_DEFAULT 0
#define INPUT_PULSE		gpio_get_level(GPIO_INPUT)

#define PULSE_TAG "PULSE"

#define FIFO_SIZE 512

typedef struct{
	uint32_t tail;
	uint32_t head;
	char buf[FIFO_SIZE];
}TFifo;

typedef struct{
	uint8_t day;   /*!< Day (start from 1) */
	uint8_t month; /*!< Month (start from 1) */
	uint16_t year;
}TDate;

typedef struct{
	uint8_t hour;      /*!< Hour */
	uint8_t min;    /*!< Minute */
	uint8_t sec;    /*!< Second */
}TTim;

typedef struct{
	TDate date;
	TTim tim;
	float len;     //Periodo
	float acc; 	   //Tiempo acumulado
	uint32_t xDia; //Pulsos por dia
	uint32_t limit; //Limite Pulsos por dia
	TFifo fifo;
}TPulse;

typedef enum {
    PULSE_MEASURED,
    PULSE_ISR,
} pulse_event_id_t;

extern TPulse *pulse;
void Pulse_Init(void);
void Pulse_task(void* arg);


#endif /* MAIN_COMPONENTS_PULSE_PULSE_H_ */
