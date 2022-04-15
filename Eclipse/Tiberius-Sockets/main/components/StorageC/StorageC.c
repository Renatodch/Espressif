/*
 * storage.c
 *
 *  Created on: 28 feb. 2020
 *      Author: Renato
 */

//#include "main.h"
#include "main.h"
TStorage	   *sto;
//extern EventGroupHandle_t main_event_group;

    /*
    * ./partitions_example.csv. For reference, its contents are as follows:
    *
    *  nvs,        data, nvs,      0x9000,  0x6000,
    *  phy_init,   data, phy,      0xf000,  0x1000,
    *  factory,    app,  factory,  0x10000, 1M,
    *  storage,    data, ,             , 0x40000,
    */

ESP_EVENT_DEFINE_BASE(STORAGE_EVENT);

void pWriteInc(void){
	if((sto->pWrite + 1) == REFERENCE_SECTOR)	sto->pWrite = 0;
	else										sto->pWrite++;

}


bool Storage_Save_Info(char *name, void* val, val_e t, uint32_t sector){
	char buffer[256] = {0};
	char value[64] = {0};
	char line[64]={0};

	strcat(line,name);
	strcat(line,"=");

	switch(t)
	{
		case INT:
			sprintf(value,"%d&",*(uint32_t*)val);
			break;
		case STRING:
			sprintf(value,"%s&",(char*)val);
			break;
		case DOUBLE:
			sprintf(value,"%.2f&",*(float*)val);
			break;
	}

	//strcat(value,"\r\n");
	strcat(line,value);

	//Debug("sector:%d line:%s",sector,line);

	ESP_ERROR_CHECK(esp_partition_read(sto->partition, sector, buffer, sizeof(buffer)));

	if(Params_Update_Lines(buffer,name,line))
	{
		ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, sector, MMAP_SIZE_4KB)); //borramos
		ESP_ERROR_CHECK(esp_partition_write(sto->partition, sector, buffer,sizeof(buffer)));
		return 1;
	}
	return 0;
}



void Storage_Save_Reference(uint32_t sector){
	uint32_t *vals = (uint32_t *)calloc(3,sizeof(uint32_t));

	ESP_ERROR_CHECK(esp_partition_read(sto->partition, REFERENCE_SECTOR, vals, 3*sizeof(uint32_t)));
	ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, REFERENCE_SECTOR, MMAP_SIZE_4KB)); //borramos

	switch(sector){
		case REFERENCE_WRITE_SECTOR:
			*vals = sto->pWrite;
			break;
		case REFERENCE_READ_SECTOR:
			*(vals+1) = sto->pRead;
			break;
		case REFERENCE_PAGE_SECTOR:
			*(vals+2) = sto->pPage;
			break;
	}

	ESP_ERROR_CHECK(esp_partition_write(sto->partition, REFERENCE_SECTOR, vals,3*sizeof(uint32_t)));

	free(vals);
}


void Storage_Load_References(void){
	uint32_t *vals = (uint32_t *)calloc(3,sizeof(uint32_t));

	ESP_ERROR_CHECK(esp_partition_read(sto->partition, REFERENCE_SECTOR, vals, 3*sizeof(uint32_t)));

	if(*vals==0xffffffff) 		sto->pWrite = 0;
	else			   	  		sto->pWrite = *vals;
	if(*(vals+1)==0xffffffff) 	sto->pRead = 0;
	else			   	  		sto->pRead = *(vals+1);
	if(*(vals+2)==0xffffffff) 	sto->pPage = 0;
	else			   	 		sto->pPage = *(vals+2);

	free(vals);
}


void Storage_Reset_References(void){
	sto->pWrite = 0;
	sto->pRead = 0;
	sto->pPage = 0;
	ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition,0,sto->partition->size));

	Storage_Save_Reference(REFERENCE_WRITE_SECTOR);
	Storage_Save_Reference(REFERENCE_READ_SECTOR);
	Storage_Save_Reference(REFERENCE_PAGE_SECTOR);
	Storage_Load_References();
}

/*
void Save_Task(void* arg){
	bool restart = false;
	char nulo = '\0';
	uint8_t temp = 0;
	uint32_t xDia = pulse->xDia;
	float acc = pulse->acc;
	uint32_t year = pulse->date.year,month = pulse->date.month,day= pulse->date.day;
	uint32_t hour= pulse->tim.hour,min= pulse->tim.min,sec= pulse->tim.sec;
	while(1)
	{
		if(xDia!=pulse->xDia){ //pulsos acumulados
			xDia=pulse->xDia;
			Storage_Save_Info("pulsos",&xDia,INT,ACC_SECTOR);
		}
		if(acc!=pulse->acc){ //tiempo acumulado
			acc=pulse->acc;
			Storage_Save_Info("acc",&acc,DOUBLE,ACC_SECTOR);
		}
		if(year!=pulse->date.year){ //Fecha del pulso
			year=pulse->date.year;
			Storage_Save_Info("year",&year,INT,ACC_SECTOR);
		}
		if(month!=pulse->date.month){
			month=pulse->date.month;
			Storage_Save_Info("month",&month,INT,ACC_SECTOR);
		}
		if(day!=pulse->date.day){
			day=pulse->date.day;
			Storage_Save_Info("day",&day,INT,ACC_SECTOR);
			//Storage_Save_Info("hoy",&day,INT,ACC_SECTOR);
		}
		if(hour!=pulse->tim.hour){ //hora del pulso
			hour=pulse->tim.hour;
			Storage_Save_Info("hour",&hour,INT,ACC_SECTOR);
		}
		if(min!=pulse->tim.min){
			min=pulse->tim.min;
			Storage_Save_Info("min",&min,INT,ACC_SECTOR);
		}
		if(sec!=pulse->tim.sec){
			sec=pulse->tim.sec;
			Storage_Save_Info("sec",&sec,INT,ACC_SECTOR);
		}

		if(pulse->fifo.tail!=pulse->fifo.head){

			while(temp!=0xFF)
			{
				ESP_ERROR_CHECK(esp_partition_read(sto->partition, sto->pWrite, &temp, 1));

				if(temp==0xFF){
					if(restart){
						Debug("temp = '0xFF, pWrite = %d", sto->pWrite);
						ESP_ERROR_CHECK(esp_partition_write(sto->partition, sto->pWrite, &nulo,1));
						pWriteInc();
						restart = false;
					}
					ESP_ERROR_CHECK(esp_partition_write(sto->partition, sto->pWrite, &pulse->fifo.buf[pulse->fifo.tail],pulse->fifo.head - pulse->fifo.tail + 1 ));

					if(pulse->fifo.tail + pulse->fifo.head + 1 >= FIFO_SIZE){
						pulse->fifo.tail = pulse->fifo.head - FIFO_SIZE;
					}
					else{
						pulse->fifo.tail += pulse->fifo.head;
					}

					//Debug("Escribiendo: %c",pulse->fifo.buf[pulse->fifo.tail-1]);
					temp=0;
					break;
				}
				else{ // Hay un dato en pWrite
					//Debug("Hay un dato en pWrite...pWrite++");
					pWriteInc();
					restart = true;
				}
			}

			pWriteInc();
			Storage_Save_Reference(REFERENCE_WRITE_SECTOR);
		}

		vTaskDelay(1/portTICK_PERIOD_MS);
	}
}
*/

void Storage_Init(void){

	ESP_LOGI(STORAGE_TAG,"Inicializando Storage");

	sto->pWrite = 0;//0x1F3F96;
	sto->pRead = 0;//0x3F96;
	sto->pPage = 0;//0x1F0000;

	sto->len = 0;
    sto->buf = calloc(1,BUFFER_SIZE);

    /*Determina la partición de memoria externa*/
    sto->partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY,"storage");
    assert(sto->partition != NULL);
    ESP_LOGI(STORAGE_TAG,"Partition Address:\t%i\n",sto->partition->address);
    ESP_LOGI(STORAGE_TAG,"Partition Size:\t%i\n",sto->partition->size);

	//ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition,0,sto->partition->size));

    Storage_Load_References();

    //xTaskCreatePinnedToCore(Save_Task, "Save_Task", 4096, pulse, 3, NULL,0);

}







