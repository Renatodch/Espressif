/*
 * bt.c
 *
 *  Created on: 26 mar. 2021
 *      Author: Renato
 */

#include "main.h"
#include "bt.h"

void bleAdvtTask(void * args){

}

void Bluetooth_Init(){
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        return;
        printf("Bluetooth controller release classic bt memory failed: %s \n", esp_err_to_name(ret));
    }

    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
    	printf("Bluetooth controller initialize failed: %s \n", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
    	printf("Bluetooth controller enable failed: %s \n", esp_err_to_name(ret));
        return;
    }

    /*
     * If call mem release here, also work. Input ESP_BT_MODE_CLASSIC_BT, the function will
     * release the memory of classic bt mode.
     * esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
     *
     */

    /*
     * If call mem release here, also work. Input ESP_BT_MODE_BTDM, the function will calculate
     * that the BLE mode is already used, so it will release of only classic bt mode.
     * esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
     */

    xTaskCreatePinnedToCore(&bleAdvtTask, "bleAdvtTask", 2048, NULL, 5, NULL, 0);
}


