#include "main.h"

TState	state;

void app_main(void)
{
	if(!(params = calloc(1,sizeof(TParam))))			Debug("calloc memory for params failed");
	if(!(gps = calloc(1,sizeof(TGPS_Parser))))			Debug("calloc memory for gps failed");
	if(!(pulse = calloc(1,sizeof(TPulse))))				Debug("calloc memory for pulse failed");
	if(!(sta = calloc(1,sizeof(TWifi_Station))))		Debug("calloc memory for station failed");
	if(!(sto = calloc(1,sizeof(TStorage))))				Debug("calloc memory for storage failed");

	//Inicializa GPS
	//Gps_Init();
	//Inicializa LED
	Led_Init();
	//Inicializa Memoria externa
	Storage_Init();
	//Inicializa WIFI
	//WifiStation_Init();
	//Inicializa Claxon
	//Pulse_Init();
	//Inicializa Bluetooth
	Bluetooth_Init();
	//Inicializa RX para comandos / Carga Parametros
	Params_Rx_Init();

	xTaskCreatePinnedToCore(Led_Task,"Led_Task",4096,NULL,6,NULL,0);

	State(CONFIGURANDO);
	vTaskDelay(5000/portTICK_PERIOD_MS);
	State(LISTO);

    //xTaskCreatePinnedToCore(Wifi_Station_Task,"Wifi_Station_Task",8092,NULL,1,NULL,0);
    //xTaskCreatePinnedToCore(Pulse_task,"Pulse_task", 8092, NULL, 2, NULL,0);
	//xTaskCreatePinnedToCore(Params_Task,"Params_Task", 8092, NULL, 5, NULL,0);


    printf("Dispositivo Listo\n");

	while(1)
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}


void State(EState e){
	state.val = e;
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




