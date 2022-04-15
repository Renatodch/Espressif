/*
 * params.c
 *
 *  Created on: 26 jul. 2020
 *      Author: Renato
 */

#include "main.h"
TParam	*params;

bool Params_Update_Lines(char *src,char*name, char *line)
{
	//char lim[64]={0};
	char oldVal[64]={0};
	char newVal[64]={0};
	char out[64]={0};
	char token[64]={0};
	char result[256]={0};
	char *p=out;

	if(strstr(src,name)==NULL){ //Agrega Parametro

		strcat(result,line);

		if((p = strchr(src,(char)0xff))!=NULL){ //Inicio de escritura
			if(p==src)
				src[0]='\0';
		}
		strcat(src, result);
		//Debug("src: %s",src);
		return 1;
	}

	do //Actualiza parametro
	{
		strcpy(out,"");
		String_Between(src,token,"&",out);

		//Debug("out:%s",out);
		if(String_Is_Empy(out)) break;

		if(strstr(out,name)!=NULL)
		{
			//Verificamos si es el mismo valor para no cambiarlo
			String_Between(out,"=","",oldVal);
			String_Between(line,"=","&",newVal);
			if(strcmp(oldVal,newVal)==0){
				//Debug("Duplicidad de datos, no actualizamos...");
				return 0;
			}

			//Debug("Dato nuevo, actualizamos");
			strcat(result,line);
		}
		else{
			//Debug("No es la linea");
			strcat(result,out);
			strcat(result,"&");
		}
		strcpy(token,out);
		strcat(token,"&");
		//Debug("token:%s",token);
		//Debug("src:%s",src);
	}
	while(!String_Is_Empy(out));

//   Debug("tokens.. result: %s", result);
   strcpy(src, result);
   return 1;
}



/*
 * name : "name="
 * */
void Params_Get(char* buf,char *name,char*val,char *_default ){

	*val='\0';
	String_Between(buf,name,"&",val);
	if(String_Is_Empy(val)){
		strcpy(val,_default);
	}

}

void Params_Init(void)
{
	char buffer[256]={0};
	char value[64] = {0};

	//Debug("val:%s, len:%d",value,strlen(value));
	params->buf = (char*)malloc(64);
	//**********************ACUMULATOR_SECTOR**********************
	ESP_ERROR_CHECK(esp_partition_read(sto->partition, ACC_SECTOR, buffer, sizeof(buffer)));

	Params_Get(buffer,"pulsos=",value,"0");
	pulse->xDia = atoi(value);
	Params_Get(buffer,"acc=",value,"0.0");
	pulse->acc = atof(value);
	Params_Get(buffer,"year=",value,"2020");
	pulse->date.year = atof(value);
	Params_Get(buffer,"month=",value,"1");
	pulse->date.month = atof(value);
	Params_Get(buffer,"day=",value,"1");
	pulse->date.day = atof(value);
	Params_Get(buffer,"hour=",value,"0");
	pulse->tim.hour = atof(value);
	Params_Get(buffer,"min=",value,"0");
	pulse->tim.min = atof(value);
	Params_Get(buffer,"sec=",value,"0");
	pulse->tim.sec = atof(value);

	//***************PARAMS_SECTOR************************
	ESP_ERROR_CHECK(esp_partition_read(sto->partition, PARAM_SECTOR, buffer, sizeof(buffer)));

	Params_Get(buffer,"limit=",value,"20");
	pulse->limit = (uint32_t)atof(value);
	Params_Get(buffer,"ssid=",value,"ROCCO");
	memcpy(sta->ssid,(uint8_t*)value,sizeof(sta->ssid));
	Params_Get(buffer,"pswd=",value,"12281101");
	Params_Get(buffer,"ip=",value,"192.168.0.25");
	strcpy(sta->tcp_client->ipv4,value);
	Params_Get(buffer,"port=",value,"8888");
	sta->tcp_client->port = atof(value);
	memcpy(sta->psswd,(uint8_t*)value,sizeof(sta->psswd));
	Params_Get(buffer,"config=",value,"123456");
	strcpy(params->configPass,value);
	Params_Get(buffer,"id=",value,"1");
	params->idV = (uint32_t)atof(value);

}


void Params_Autenticado_Evts(PARAMS_EVT evt, char* data){
	char buffer[64]={0};
	uint32_t i = 4,acc=0;
	uint32_t *vals = (uint32_t *)calloc(3,sizeof(uint32_t));

	switch(evt){
		case IDV_READ_RCV:
			printf("+ID: %s\n",sta->ssid);
			break;
		case IDV_WRITE_RCV:
			i=atoi(data);
			if(i!=0){
				if(Storage_Save_Info("id",&i,INT,PARAM_SECTOR))
					printf("+ID: cambiado a: %d\n",i);
				else
					printf("+ID: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+ID: valor inválido\n");
			}
			break;
		case SSID_READ_RCV:
			printf("+SSID: %s\n",sta->ssid);
			break;
		case SSID_WRITE_RCV:
			if(strlen(data)<33){
			    if(Storage_Save_Info("ssid",data,STRING,PARAM_SECTOR))
			    	printf("+SSID: cambiado a: %s\n",data);
			    else
			    	printf("+SSID: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+SSID: valor demasiado largo, no puede exceder de 32 caracteres\n");
			}
			break;
		case PSWD_READ_RCV:
			printf("+PSWD: %s\n",sta->psswd);

			break;
		case PSWD_WRITE_RCV:
			if(strlen(data)<33){
			    if(Storage_Save_Info("pswd",data,STRING,PARAM_SECTOR))
			    	printf("+PSWD: cambiado a: %s\n",data);
			    else
			    	printf("+PSWD: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+PSWD: valor demasiado largo, no puede exceder de 32 caracteres\n");
			}
			break;
		case LIMIT_READ_RCV:
			printf("+LIMIT: %d\n",pulse->limit);
			break;
		case LIMIT_WRITE_RCV:
			i=atoi(data);
			if(i!=0){
			    if(Storage_Save_Info("limit",&i,INT,PARAM_SECTOR))
			    	printf("+LIMIT: cambiado a: %d\n",i);
			    else
					printf("+LIMIT: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+LIMIT: valor inválido\n");
			}
			break;
		case CONFIG_READ_RCV:
			printf("+CONFIG: %s\n",params->configPass);
			break;
		case CONFIG_WRITE_RCV:
			if(strlen(data)>32){
			    if(Storage_Save_Info("config",data,STRING,PARAM_SECTOR))
			    	printf("+CONFIG: cambiado a: %s\n",data);
			    else
					printf("+CONFIG: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+CONFIG: valor demasiado largo, no puede exceder de 32 caracteres\n");
			}
			break;
		case IP_READ_RCV:
			printf("+SERVER IP: %s\n",sta->tcp_client->ipv4);
			break;
		case IP_WRITE_RCV:
			//data = xxx.xxx.xxxx.xxx
			if(inet_addr(data)!=INADDR_NONE)
			{
				if(Storage_Save_Info("ip",data,STRING,PARAM_SECTOR))
					printf("+SERVER IP: cambiado a: %s\n",data);
				else
					printf("+SERVER IP: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+IP: valor inválido\n");
			}

			break;
		case PORT_READ_RCV:
			printf("+SERVER PORT: %d\n",sta->tcp_client->port);
			break;
		case PORT_WRITE_RCV:
			i=atoi(data);
			if(i!=0){
				if(Storage_Save_Info("port",&i,INT,PARAM_SECTOR))
					printf("+SERVER PORT: cambiado a: %d\n",i);
				else
					printf("+SERVER PORT: duplicidad de datos, no se actualizó\n");
			}
			else{
				printf("+PORT: valor inválido\n");
			}
			break;
		case EXIT_RCV:
			params->state = INICIO;
			printf("Saliendo del modo de configuración...\n");
			while(i--){
				vTaskDelay(1000/portTICK_PERIOD_MS);
				printf("Reiniciando en %d\n",i);
			}
			esp_restart();
			break;
		case SHOW_RCV:

			printf("EN SRAM\n");
			printf("ssid=%s\n",sta->ssid);printf("psswd=%s\n",sta->psswd);printf("limit=%d\n",pulse->limit);printf("config=%s\n",params->configPass);
			printf("ip=%s\n",sta->tcp_client->ipv4);printf("port=%d\n",sta->tcp_client->port);printf("pulsos=%d\n",pulse->xDia);
			printf("acc=%.1f\n",pulse->acc);printf("year=%04d\n",pulse->date.year);printf("month=%02d\n",pulse->date.month);
			printf("day=%02d\n",pulse->date.day);printf("hour=%02d\n",pulse->tim.hour);printf("min=%02d\n",pulse->tim.min);
			printf("sec=%02d\n",pulse->tim.sec);


			printf("\nEN MEMORIA FLASH\n");
			ESP_ERROR_CHECK(esp_partition_read(sto->partition, PARAM_SECTOR, buffer, sizeof(buffer)));
			printf("PARAMETROS CONFIGURADOS:\n");
			printf("%s\n",buffer);

			ESP_ERROR_CHECK(esp_partition_read(sto->partition, ACC_SECTOR, buffer, sizeof(buffer)));
			printf("ACUMULADORES:\n");
			printf("%s\n",buffer);

			//char *pPulso = (char *)calloc(pulse->xDia,64);
			char pulso0[64]={0};
			for(int j=0; j<pulse->xDia;j++){
				ESP_ERROR_CHECK(esp_partition_read(sto->partition, PULSE_SECTOR+acc+sto->pRead, pulso0, 64));
				acc+=strlen(pulso0)+1;
				printf("PULSO %d:\n",j+1);
				printf("%s\n",pulso0);
			}

			ESP_ERROR_CHECK(esp_partition_read(sto->partition, REFERENCE_SECTOR,vals ,3*sizeof(uint32_t)));

			printf("pWrite: %d pRead: %d pPage:%d\n", *vals, *(vals+1), *(vals+2));

			break;

		case CLEAR_RCV:
			ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, PULSE_SECTOR, MMAP_SIZE_4KB)); //borramos
			ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, REFERENCE_SECTOR, MMAP_SIZE_4KB)); //borramos
			ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, PARAM_SECTOR, MMAP_SIZE_4KB)); //borramos
			ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition, ACC_SECTOR, MMAP_SIZE_4KB)); //borramos
			break;

		case INVALID_RCV:
			printf("Comando Inválido\n");
			break;
		default:break;
	}
}

void Params_Inicio_Evts(PARAMS_EVT evt, char* data){

	switch(evt){
		case CONFIG_RCV:
			params->state = AUTENTICADO;
			printf("###################################\n");
			printf("Bienvenido al modo de configuracion\n");
			printf("###################################\n");
			printf("\n");
			printf("-Caracteres NO permitidos: # $ %s & [ESPACIO]\n","%");
			printf("-No se permite valor de 0 para limite de pulsos por dia\n");
			printf("-No se permite valor de 0 para identificador del vehículo\n");
			printf("Ingrese:\n");
			printf("id? : para saber el identificador de este vehículo\n");
			printf("id={nombre} : para sobreescribir el identificador\n");
			printf("ssid? : para saber el nombre de acceso a wifi\n");
			printf("ssid={nombre} : para sobreescribir el nombre de acceso a wifi\n");
			printf("pswd? : para saber la clave de acceso a wifi\n");
			printf("pswd={pass} : para sobreescribir la clave de acceso a wifi\n");
			printf("limit? : para saber los pulsos por dia limite\n");
			printf("limit={limit} : para sobreescribir los pulsos por dia limite actual\n");
			printf("config? : para saber la clave [config]\n");
			printf("config={config} : para sobreescribir la clave [config] actual\n");
			printf("exit: para salir del modo de configuración y reniciar el sistema\n");

		break;

		default:
			printf("Clave [config] Incorrecta\n");
			printf("Ingrese de nuevo la clave [config] por favor\n");
		break;
	}
}

void Params_FSM(PARAMS_EVT evt, char* data){
	switch(params->state){
		case INICIO:
			Params_Inicio_Evts(evt,data);
			break;
		case AUTENTICADO:
			Params_Autenticado_Evts(evt,data);
			break;
	}
}

void Params_Process(char * cmd){
	char *s;

	if(strchr(cmd,'#')!=NULL ||
			strchr(cmd,'$')!=NULL ||
				strchr(cmd,'%')!=NULL ||
					strchr(cmd,'&')!=NULL)
	{
		Params_FSM(INVALID_RCV,cmd);
		printf("Caracteres Inválidos\n");
		return;
	}

	//*********** DEVICE *****************
	if(strcmp(cmd,params->configPass)==0){
		Params_FSM(CONFIG_RCV,cmd);
		return;
	}

	if(strcmp(cmd,"ssid?")==0) {
		Params_FSM(SSID_READ_RCV,cmd);
		return;
	}

	if((s=strstr(cmd,"ssid=")) != NULL) {
		if(s==cmd && (s+5)!=NULL){
			Params_FSM(SSID_WRITE_RCV,s+5);
			return;
		}
	}
	if(strcmp(cmd,"pswd?")==0) {
		Params_FSM(PSWD_READ_RCV,cmd);
		return;
	}

	if((s=strstr(cmd,"pswd=")) != NULL) {
		if(s==cmd && (s+5)!=NULL){
			Params_FSM(PSWD_WRITE_RCV,s+5);
			return;
		}
	}
	if(strcmp(cmd,"limit?")==0){
		Params_FSM(LIMIT_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"limit=")) != NULL) {
		if(s==cmd && (s+6)!=NULL){
			Params_FSM(LIMIT_WRITE_RCV,s+6);
			return;
		}
	}
	if(strcmp(cmd,"config?")==0) {
		Params_FSM(CONFIG_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"config=")) != NULL) {
		if(s==cmd && (s+7)!=NULL){
			Params_FSM(CONFIG_WRITE_RCV,s+7);
			return;
		}
	}
	if(strcmp(cmd,"id?")==0) {
		Params_FSM(IDV_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"id=")) != NULL) {
		if(s==cmd && (s+3)!=NULL){
			Params_FSM(IDV_WRITE_RCV,s+3);
			return;
		}
	}

	//************** SERVER ****************
	if(strcmp(cmd,"ip?")==0) {
		Params_FSM(IP_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"ip=")) != NULL) {
		if(s==cmd && (s+7)!=NULL){
			Params_FSM(IP_WRITE_RCV,s+3);
			return;
		}
	}
	if(strcmp(cmd,"port?")==0) {
		Params_FSM(PORT_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"port=")) != NULL) {
		if(s==cmd && (s+5)!=NULL){
			Params_FSM(PORT_WRITE_RCV,s+5);
			return;
		}
	}
	//**************************************
	if(strcmp(cmd,"exit")==0){ //RESETEO
		Params_FSM(EXIT_RCV,cmd);
		return;
	}

	//Para los desarrolladores :p
	if(strcmp(cmd,"show")==0) { //Mostrar variables:
		Params_FSM(SHOW_RCV,cmd);
		return;
	}

	if(strcmp(cmd,"clear")==0) { //Borrar 4KB en cada sector
		Params_FSM(CLEAR_RCV,cmd);
		return;
	}

	Params_FSM(INVALID_RCV,cmd);
}

static void cmd_uart_pattern(void)
{
    int pos = uart_pattern_pop_pos(0);
    char *p;
    if (pos != -1 && pos<=64) {

    	memset(params->buf,0x00,64);
        /* read one line(include '\n') */
        int read_len = uart_read_bytes(0, (uint8_t *)params->buf, pos + 1, 1000 / portTICK_PERIOD_MS);

        if(read_len > pos){

			Debug("len= %d pos:%d read_len:%d cmd=%s",strlen(params->buf),pos,read_len,params->buf);

			if((p=strchr(params->buf,'\n'))!=NULL){
				//Debug("Tiene line feed");
				params->buf[(uint32_t)(p-params->buf)] = '\0';
				//strlcpy(params->buf,params->buf,p-params->buf);
			}
			if((p=strchr(params->buf,'\r'))!=NULL){
				//Debug("Tiene CR");
				params->buf[(uint32_t)(p-params->buf)] = '\0';
			}

			Params_Process(params->buf);
			//free(params->buf);
        }
    	uart_flush_input(0);

    }else if (pos>64){
    	uart_flush_input(0);
    	printf("ERROR\n");
    }
    else {
    	printf("ERROR\n");
        ESP_LOGW(GPS_TAG, "Tamaño de la cola de patrones muy pequeño");
        uart_flush_input(0);
    }
}


void Params_Rx_Init(void) {
	gpio_config_t io_conf;

	/*Configura pin de reseteo*/
	io_conf.pin_bit_mask = (1ULL<<RESET_INPUT);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    if (uart_param_config(0, &uart_config) != ESP_OK)
        ESP_LOGE(GPS_TAG, "config uart parameter failed");

    if (uart_set_pin(0, -1, 3, -1, -1) != ESP_OK)
        ESP_LOGE(GPS_TAG, "config uart gpio failed");

    if (uart_driver_install(0, 256, 0,16, &params->queueCmd, 0) != ESP_OK)
        ESP_LOGE(GPS_TAG, "install uart driver failed");

    /* Set pattern interrupt, used to detect the end of a line */
    uart_enable_pattern_det_intr(0, '\n', 1, 10000, 10, 10);
    /* Set pattern queue size */
    uart_pattern_queue_reset(0, 16);
    uart_flush(0);

    Params_Init();
}

void Params_Rx_Events(void)
{
    uart_event_t event;
	if (xQueueReceive(params->queueCmd, &event, pdMS_TO_TICKS(200)))
	{
		switch (event.type) {
		case UART_DATA:
			break;
		case UART_FIFO_OVF:
			ESP_LOGW(GPS_TAG, "HW FIFO Overflow");
			uart_flush(0);
			xQueueReset(params->queueCmd);
			break;
		case UART_BUFFER_FULL:
			ESP_LOGW(GPS_TAG, "Ring Buffer Full");
			uart_flush(0);
			xQueueReset(params->queueCmd);
			break;
		case UART_BREAK:
			ESP_LOGW(GPS_TAG, "Rx Break");
			break;
		case UART_PARITY_ERR:
			ESP_LOGE(GPS_TAG, "Parity Error");
			break;
		case UART_FRAME_ERR:
			ESP_LOGE(GPS_TAG, "Frame Error");
			break;
		case UART_PATTERN_DET:
			cmd_uart_pattern();
			break;
		default:
			ESP_LOGW(GPS_TAG, "unknown uart event type: %d", event.type);
			break;
		}
	}
}




