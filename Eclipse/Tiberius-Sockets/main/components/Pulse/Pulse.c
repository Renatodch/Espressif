/*
 * Pulse.c
 *
 *  Created on: 29 feb. 2020
 *      Author: Renato
 */
#include "main.h"


#include "esp_int_wdt.h"

TPulse *pulse;


xQueueHandle pulse_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t start = xTaskGetTickCount();

    xQueueSendFromISR(pulse_evt_queue, &start, NULL);
}


void Pulse_task(void* arg)
{
    uint32_t start,stop,interval;
	char buf[70] = {0};

    while(1)
    {
		//Debug("core: %d",xPortGetCoreID());
		if(xQueueReceive(pulse_evt_queue, &start, pdMS_TO_TICKS(100)))
		{
		    //uint32_t start2 = xTaskGetTickCount();
		    //Debug("%d - %d = %d",start2,start,start2-start); //TESTEAR

			Debug("Se puls� el claxon");

			vTaskDelay(1/portTICK_PERIOD_MS); //like a filter

			State(PULSANDO);

			if(!INPUT_PULSE)
			{
				while(!INPUT_PULSE){vTaskDelay(1);}

				stop = xTaskGetTickCount();

				#ifdef NO_GPS_TEST
				if(1)
				#else
				if(/*sta->state != ENVIANDO_DATOS &&*/
						myGPS.sats_in_use >= MIN_SATS_TO_USE)
				#endif
				{
					//Calcula el tiempo pulsado del claxon
					if(stop >= start) interval = stop - start;
					else			  interval = 0xFFFFFFFF - start + stop;

					pulse->len = (float)interval * 10.0f;
					pulse->len = pulse->len /1000.0f; //(segundos)

					//#ifdef NO_GPS_TEST
					pulse->acc += pulse->len;
					pulse->xDia++;
					/*
					#else

					if(pulse->date.day != myGPS.date.day){
						pulse->acc 	= 0;
						pulse->xDia = 0;
					}
					else{
						pulse->acc += pulse->len;
						pulse->xDia++;
					}
					#endif
					*/

					pulse->date.year = myGPS.date.year;
					pulse->date.month = myGPS.date.month;
					pulse->date.day = myGPS.date.day;
					pulse->tim.hour = myGPS.tim.hour;
					pulse->tim.min= myGPS.tim.minute;
					pulse->tim.sec = myGPS.tim.second;

					Debug("Duraci�n: %.02f s\n", pulse->len);
					Debug("Acumulador de Duraciones: %.02f s\n", pulse->acc);
					Debug("Contador de Pulsos: %06d\n", pulse->xDia);

					//Crea cadena de datos. Altitud no es muy necesaria
					sprintf(buf,"$I%05dD%04d,%02d,%02dH%02d,%02d,%02dL%.02f,%.02f,%06dP%.05f,%.05f*",params->idV,
							myGPS.date.year,myGPS.date.month,myGPS.date.day,myGPS.tim.hour,myGPS.tim.minute,
							myGPS.tim.second,pulse->len,pulse->acc,pulse->xDia,myGPS.latitude,myGPS.longitude);

					int len = strlen(buf) + 1;//mas el NULL

					Debug("head: %d buf: %s\n",len,buf);

					Storage_Save_Info("pulsos",&pulse->xDia,INT,ACC_SECTOR);
					Storage_Save_Info("acc",&pulse->acc,DOUBLE,ACC_SECTOR);
					Storage_Save_Info("year",&pulse->date.year,INT,ACC_SECTOR);
					Storage_Save_Info("month",&pulse->date.month,INT,ACC_SECTOR);
					Storage_Save_Info("day",&pulse->date.day,INT,ACC_SECTOR);
					Storage_Save_Info("hour",&pulse->tim.hour,INT,ACC_SECTOR);
					Storage_Save_Info("min",&pulse->tim.min,INT,ACC_SECTOR);
					Storage_Save_Info("sec",&pulse->tim.sec,INT,ACC_SECTOR);

					if((sto->pWrite + len) >= REFERENCE_SECTOR){
						int rest = REFERENCE_SECTOR - sto->pWrite;
						//guarda hasta donde entre
						esp_partition_write(sto->partition, sto->pWrite, &buf[0], rest); //5
						sto->pWrite = 0;
						//guarda lo que falta desde donde se quedo
						esp_partition_write(sto->partition, sto->pWrite, &buf[rest], len - rest);
						sto->pWrite = len -rest;
					}
					else{
						esp_partition_write(sto->partition, sto->pWrite, buf, len);
						sto->pWrite += len;
					}

					Storage_Save_Reference(REFERENCE_WRITE_SECTOR);
				}
				else{
					//if(sta->state==ENVIANDO_DATOS)
						//Debug("Se estan enviando datos, no presione el claxon");
					//if(myGPS.sats_in_use<5)
						Debug("Se�al GPS no establecida......");
				}
			}
			State(LISTO);
		}

    }
}

static void Fifo_Init(void){
	pulse->fifo.head = 0;
	pulse->fifo.tail = 0;
	memset(pulse->fifo.buf,0x00,sizeof(pulse->fifo.buf));
}

void Pulse_Init(void)
{
	Debug("Inicializando Receptor de Pulsos");

	Fifo_Init();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    io_conf.pin_bit_mask = CLAXON_INPUT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    pulse_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT, gpio_isr_handler, (void*) GPIO_INPUT);

}

