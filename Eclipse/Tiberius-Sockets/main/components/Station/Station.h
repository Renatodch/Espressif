/*
 * Station.h
 *
 *  Created on: 26 feb. 2020
 *      Author: Renato
 */

#ifndef _STATION_H_
#define _STATION_H_

#include "main.h"


#define WIFI_STATION_TAG "WIFI_STA"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MIN_DB	(-80)
#define MAX_DB	(-75)

typedef struct{
	char ipv4[16];
	uint16_t port;
}tcp_client_t;

typedef enum{
	AP_RSSI_PULSOS,
	CONECTANDO_AP,
	FALLO_CONEXION_AL_SERVIDOR,
	FALLO_CONEXION_AL_AP,
	CONECTO_AL_SERVIDOR,
	DESCONECTANDO_SERVIDOR,
	DESCONECTANDO_AP,
	FALLO_ENVIO,
	COMPLETADO

}WIFI_EVT;

typedef enum{
	LEYENDO_AP,
	CONECTANDO_A_AP,
	CONECTADO_AP,
	ENVIANDO_DATOS
}WIFI_STE;

typedef struct{
	WIFI_STE state;
	int socket;
	tcp_client_t *tcp_client;
	char ipv4[16];
	uint8_t ssid[64];
	uint8_t psswd[64];
	wifi_ap_record_t ap_record;
	//int ap_record_num;
}TWifi_Station;
extern TWifi_Station * sta;
void WifiStation_Init(void);

void Wifi_Station_Task(void *arg);
#endif /* INC_STATION_H_ */
