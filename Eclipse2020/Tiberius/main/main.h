/*
 * main.h
 *
 *  Created on: 28 feb. 2020
 *      Author: Renato
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"

//BT includes
//#include "./bt/common/include/bt_common.h"
#include "esp_bt.h"

#include "esp_tls.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_event.h"
//#include "esp_event_loop.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "esp_netif.h" //#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_err.h"

#include "bt.h"

#include "str.h"
#include "params.h"
#include "StorageC.h"
#include "Pulse.h"
#include "gps.h"
#include "Station.h"
#include "Led.h"

#define DEBUG
//#define NO_GPS_TEST

typedef enum{
	CONFIGURANDO,
	LISTO,
	PULSANDO,
	ENVIANDO
}EState;

typedef struct{
	EState val;
	char *str;
}TState;


extern char Estados[4][10];
extern TState state;

#define CLOSE_LIMIT		10

#define RESET_INPUT		(21)
#define ON_BOARD_LED	(2)
#define RED_LED			(5)  //igual o mayor al limite
#define GREEN_LED		(19)
#define YELLOW_LED		(18) // por llegar al limite
#define BUFFER_SIZE 	256
#define GPS_UART		UART_NUM_2

void Debug(char* format, ...);
void State(EState e);
#endif /* MAIN_MAIN_H_ */
