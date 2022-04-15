/*
 * system.h
 *
 *  Created on: 4 mar. 2020
 *      Author: SAGE
 */

#ifndef MAIN_HEADERS_H_
#define MAIN_HEADERS_H_



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_err.h"

#endif /* MAIN_HEADERS_H_ */
