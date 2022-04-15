/*
 * params.h
 *
 *  Created on: 26 jul. 2020
 *      Author: Renato
 */

#ifndef MAIN_PARAMS_H_
#define MAIN_PARAMS_H_

#include "main.h"

typedef enum{
	INT,
	STRING,
	DOUBLE
}val_e;

typedef enum{
	CONFIG_RCV,
	EXIT_RCV,
	INVALID_RCV,
	SSID_READ_RCV,
	SSID_WRITE_RCV,
	PSWD_READ_RCV,
	PSWD_WRITE_RCV,
	LIMIT_READ_RCV,
	LIMIT_WRITE_RCV,
	CONFIG_READ_RCV,
	CONFIG_WRITE_RCV,
	SHOW_RCV,
	CLEAR_RCV,
	IP_READ_RCV,
	IP_WRITE_RCV,
	PORT_READ_RCV,
	PORT_WRITE_RCV,
	IDV_READ_RCV,
	IDV_WRITE_RCV,
}PARAMS_EVT;
typedef enum{
	INICIO,
	AUTENTICADO,
}PARAMS_STA;

typedef struct{
	PARAMS_STA state;
	QueueHandle_t queueCmd;
	char *buf;
	char configPass[64]; /*contraseña para entrar en modo de configuracion*/
	uint32_t idV; 		 /*identificador del vehículo*/
}TParam;

extern TParam	*params;
//extern uint8_t dia_de_hoy;

void Rx_Init(void);
void Params_Rx_Events(void);
void Params_FSM(PARAMS_EVT evt, char* data);
void Params_Rx_Init(void);
void Params_Init(void);
void Params_Process(char * cmd);
bool Params_Update_Lines(char *src,char*name, char *line);
void Params_Get(char* buf,char *name,char*val,char *_default );
#endif /* MAIN_PARAMS_H_ */
