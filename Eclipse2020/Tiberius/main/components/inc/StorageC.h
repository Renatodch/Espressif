/*
 * storage.h
 *
 *  Created on: 28 feb. 2020
 *      Author: Renato
 */

#ifndef _STORAGEC_H_
#define _STORAGEC_H_

#include "main.h"


#define STORAGE_TAG  "STORAGE"
#define INFO_LEN 43

#define MMAP_SIZE_64KB 0x10000
#define MMAP_SIZE_4KB 0x1000

#define PULSE_SECTOR 	 0x000000
#define REFERENCE_SECTOR 0x1F4000

enum{
	REFERENCE_WRITE_SECTOR,
	REFERENCE_READ_SECTOR,
	REFERENCE_PAGE_SECTOR
};


#define PARAM_SECTOR	0x1F5000
#define ACC_SECTOR   	0x1F6000

ESP_EVENT_DECLARE_BASE(STORAGE_EVENT);

typedef struct{
	uint32_t pWrite;
	uint32_t pRead;
	uint32_t pPage;

	uint32_t len;
	char *buf;
    const void *pMap;
    spi_flash_mmap_handle_t map_handle;
    const esp_partition_t * partition;

    esp_event_loop_handle_t event_loop_hdl;

    //pulse_t * pulse;
}TStorage;

typedef enum {
    SAVE_EVT,
    LOAD_EVT,
} storage_event_id_t;

#define COMPLETE_BIT 		BIT0 // Se completo la transferencia de todos los datos
#define INCOMPLETE_BIT      BIT1 // No se completo la transferencia de todos los datos
#define NO_DATA_BIT			BIT2

extern TStorage	   *sto;
void Storage_Init(void);
bool Storage_Save_Info(char *name, void* val, val_e t, uint32_t sector);
void Storage_Save_Reference( uint32_t sector);
void Storage_Load_References(void);
void Storage_Reset_References(void);
void Save_Task(void* arg);
void pWriteInc(void);
#endif /* COMPONENTS_STORAGEC_STORAGEC_H_ */
