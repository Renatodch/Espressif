/*
 * main.h
 *
 *  Created on: 22 feb. 2021
 *      Author: Renato
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./components/wifi/wifi.h"

#include "./components/http_server/my_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "./components/camera/camera.h"
#include "./components/camera/bitmap.h"
#include "led.h"
#include "qr_recoginize.h"

#define DEBUG

void Debug(char* format, ...);

#endif /* MAIN_MAIN_H_ */
