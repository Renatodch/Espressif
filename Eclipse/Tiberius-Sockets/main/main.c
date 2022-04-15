#include "main.h"

/*Estados principales del sistema*/
const char States[4][10] = {
		"INICIANDO",
		"LISTO    ",
		"PULSANDO ",
		"ENVIANDO "
};


TState	state;


void Led_Task(void* arg){
	int value,level=1;
	while(1)
	{
		if(pulse->xDia < (pulse->limit-CLOSE_LIMIT)){
			gpio_set_level(GREEN_LED, 1);gpio_set_level(RED_LED, 0);gpio_set_level(YELLOW_LED, 0);
		}
		else if(pulse->xDia<pulse->limit && pulse->xDia>=(pulse->limit-CLOSE_LIMIT)){
			gpio_set_level(YELLOW_LED, 1);gpio_set_level(GREEN_LED, 0);gpio_set_level(RED_LED, 0);
		}
		else{
			gpio_set_level(RED_LED, 1);gpio_set_level(YELLOW_LED, 0);gpio_set_level(GREEN_LED, 0);
		}

		switch(state.val)
		{
			case CONFIGURANDO:value = 100;break;
			default: value = 1000; break;
		}

		gpio_set_level(ON_BOARD_LED, level); level = !level;

		vTaskDelay(value/portTICK_PERIOD_MS);
	}
}

static void Led_Init(void){
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = (1ULL<<ON_BOARD_LED) | (1ULL<<RED_LED) | (1ULL<<GREEN_LED) | (1ULL<<YELLOW_LED);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

}


static void Read_Params_Task(void* arg){
	int reset=0;

	while(1)
	{
		Params_Rx_Events();
		/**********Reset [config]**************/
		if(!(gpio_input_get()&(1ULL<<RESET_INPUT)))
		{
			if(reset>=4){

				if(Storage_Save_Info("config","123456",STRING,PARAM_SECTOR)){
					printf("+CONFIG: se cambió clave a valor de fábrica: 123456\n");
				}
				else
				{
					printf("+CONFIG: duplicidad de datos, ya tiene el valor de fábrica: 123456\n");
				}
				reset = 0;
			}
			else{
				reset++;
			}
		}
		else reset = 0;
		/***********End of reset pulse*********/
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

void app_main(void)
{

	Debug("Iniciando sistema...\n");

    Debug("mínima cantidad de stack a utilizar: %d\n",uxTaskGetStackHighWaterMark(NULL));
	if(!(params = calloc(1,sizeof(TParam))))			Debug("calloc memory for params failed");
	if(!(myGPS_Parser = calloc(1,sizeof(TGPS_Parser))))	Debug("calloc memory for gps failed");
	if(!(pulse = calloc(1,sizeof(TPulse))))				Debug("calloc memory for pulse failed");
	if(!(sta = calloc(1,sizeof(TWifi_Station))))		Debug("calloc memory for station failed");
	if(!(sto = calloc(1,sizeof(TStorage))))				Debug("calloc memory for storage failed");

	//Inicializa RX para comandos / Carga Parametros
	Params_Rx_Init();
	//Inicializa LED
	Led_Init();
	xTaskCreatePinnedToCore(Led_Task,"Led_Task",4096,NULL,6,NULL,0);
	{
		State(CONFIGURANDO);
		//Inicializa Memoria externa
		Storage_Init();
		//Inicializa WIFI
		WifiStation_Init();
		//Inicializa Claxon
		Pulse_Init();
		//Inicializa GPS
		Gps_Init();
		//Inicializa LCD
		Lcd_Init(20,4,LCD_5x8DOTS);
		vTaskDelay(5000/portTICK_PERIOD_MS);
		State(LISTO);
	}

	xTaskCreatePinnedToCore(Lcd_Task,"LCD_task",4096,NULL,1,NULL,1);
    xTaskCreatePinnedToCore(Pulse_task, "Pulse_task", 8092, NULL, 2, NULL,0);
    xTaskCreatePinnedToCore(Gps_Task, "Gps_Task",4096,myGPS_Parser,3,&myGPS_Parser->tsk_hdl,0);
    xTaskCreatePinnedToCore(Wifi_Station_Task,"Wifi_Station_Task",8092,NULL,1,NULL,0);
	xTaskCreatePinnedToCore(Read_Params_Task,"Read_Params_Task", 8092, NULL, 5, NULL,0);

    printf("Dispositivo Listo\n");
    printf("Ingrese clave [config] para configurar parametros\n");

	while(1)
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}


void State(EState e){
	state.val = e;
	state.str = (char *)&States[(int)e];
}

void Debug(char* format, ...){
	va_list arglist;
	char str[128] = {0};

	va_start(arglist,format);
	vsnprintf(str,sizeof(str),format,arglist);
	va_end(arglist);

	strcat(str,"\r\n");
#ifdef DEBUG
	printf(str);
#endif
}




