/*
 * wifi.h
 *
 *  Created on: 24 feb. 2021
 *      Author: Renato
 */

#ifndef MAIN_COMPONENTS_WIFI_WIFI_H_
#define MAIN_COMPONENTS_WIFI_WIFI_H_

#include "esp_netif.h"


extern esp_ip4_addr_t s_ip_addr;


void Wifi_Init();

#endif /* MAIN_COMPONENTS_WIFI_WIFI_H_ */
