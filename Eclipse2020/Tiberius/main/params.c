/*
 * params.c
 *
 *  Created on: 26 jul. 2020
 *      Author: Renato
 */

#include "main.h"
TParam	*params;

void Params_Task(void* arg){

	//Storage_Save_Info("config","123456",STRING,PARAM_SECTOR)
	while(1)
	{
		Params_Rx_Events();

		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}


bool Params_Update_Lines(char *src,char*name, char *line)
{
	//char lim[64]={0};
	char oldVal[64]={0};
	char newVal[64]={0};
	char out[64]={0};
	char token[64]={0};
	char result[256]={0};
	char *p=out;

	//Agrega Parametro
	if(strstr(src,name)==NULL){

		//esto para poner un & al inicio de segmento
		strcat(result,line);

		if((p = strchr(src,(char)0xff))!=NULL){ //Inicio de escritura
			if(p==src)
				src[0]='\0';
		}
		strcat(src, result);
		//Debug("src: %s",src);
		return 1;
	}

	//Actualiza parametro
	do
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
	char value[256] = {0};

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
	pulse->tim.minute = atof(value);
	Params_Get(buffer,"sec=",value,"0");
	pulse->tim.second = atof(value);

	//***************PARAMS_SECTOR************************
	ESP_ERROR_CHECK(esp_partition_read(sto->partition, PARAM_SECTOR, buffer, sizeof(buffer)));

	Params_Get(buffer,"_limit#3=",value,"20");
	pulse->limit = (uint32_t)atof(value);
	Params_Get(buffer,"_ssid#5=",value,"TIBERIUS"); //TIBERIUS
	memcpy(sta->ssid,(uint8_t*)value,sizeof(sta->ssid));
	Params_Get(buffer,"_pswd#4=",value,"tiberius2021"); //tiberius2021
	memcpy(sta->psswd,(uint8_t*)value,sizeof(sta->psswd));
	Params_Get(buffer,"_url#1=",value,"http://tiberiusstorage.000webhostapp.com/dbwrite.php");
	strcpy(sta->url,value);
	Params_Get(buffer,"_config#2=",value,"123456");
	strcpy(params->configPass,value);
	Params_Get(buffer,"_id#6=",value,"c1");
	strcpy(params->idV,value);
}


void Params_Autenticado_Evts(PARAMS_EVT evt, char* data){
	char buffer[64]={0};
	uint32_t i = 4,acc=0;
	uint32_t *vals = (uint32_t *)calloc(3,sizeof(uint32_t));

	switch(evt){
		case IDV_READ_RCV:
			printf("+ID: %s\n",params->idV);
			break;
		case IDV_WRITE_RCV:
			if(strlen(data)<10){
				if(Storage_Save_Info("_id#6",data,STRING,PARAM_SECTOR))
					printf("+ID: cambiado a: %s\n",data);
				else
					printf("+ID: duplicidad de datos, no se actualizï¿½\n");
			}
			else{
				printf("+ID: valor demasiado largo, no puede exceder de 9 dï¿½gitos\n\n");
			}
			break;
		case SSID_READ_RCV:
			printf("+SSID: %s\n",sta->ssid);
			break;
		case SSID_WRITE_RCV:
			if(strlen(data)<33){
			    if(Storage_Save_Info("_ssid#5",data,STRING,PARAM_SECTOR))
			    	printf("+SSID: cambiado a: %s\n",data);
			    else
			    	printf("+SSID: duplicidad de datos, no se actualizï¿½\n");
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
			    if(Storage_Save_Info("_pswd#4",data,STRING,PARAM_SECTOR))
			    	printf("+PSWD: cambiado a: %s\n",data);
			    else
			    	printf("+PSWD: duplicidad de datos, no se actualizï¿½\n");
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
			    if(Storage_Save_Info("_limit#3",&i,INT,PARAM_SECTOR))
			    	printf("+LIMIT: cambiado a: %d\n",i);
			    else
					printf("+LIMIT: duplicidad de datos, no se actualizï¿½\n");
			}
			else{
				printf("+LIMIT: valor invï¿½lido\n");
			}
			break;
		case CONFIG_READ_RCV:
			printf("+CONFIG: %s\n",params->configPass);
			break;
		case CONFIG_WRITE_RCV:
			if(strlen(data)>32){
			    if(Storage_Save_Info("_config#2",data,STRING,PARAM_SECTOR))
			    	printf("+CONFIG: cambiado a: %s\n",data);
			    else
					printf("+CONFIG: duplicidad de datos, no se actualizï¿½\n");
			}
			else{
				printf("+CONFIG: valor demasiado largo, no puede exceder de 32 caracteres\n");
			}
			break;
		case URL_READ_RCV:
			printf("+URL: %s\n",sta->url);
			break;
		case URL_WRITE_RCV:

			if(strstr(data,"http://")!=NULL)
			{
				if(Storage_Save_Info("_url#1",data,STRING,PARAM_SECTOR))
					printf("+URL: cambiado a: %s\n",data);
				else
					printf("+URL: duplicidad de datos, no se actualizï¿½\n");
			}
			else{
				printf("+URL: valor invalido\n");
			}

			break;
		case EXIT_RCV:
			params->state = INICIO;
			printf("Saliendo del modo de configuraciï¿½n...\n");
			while(i--){
				vTaskDelay(1000/portTICK_PERIOD_MS);
				printf("Reiniciando en %d\n",i);
			}
			esp_restart();
			break;
		case SHOW_RCV:

			printf("EN SRAM\n");
			printf("ssid=%s\n",sta->ssid);printf("psswd=%s\n",sta->psswd);printf("limit=%d\n",pulse->limit);printf("config=%s\n",params->configPass);
			printf("url=%s\n",sta->url);printf("pulsos=%d\n",pulse->xDia);
			printf("acc=%.1f\n",pulse->acc);printf("year=%04d\n",pulse->date.year);printf("month=%02d\n",pulse->date.month);
			printf("day=%02d\n",pulse->date.day);printf("hour=%02d\n",pulse->tim.hour);printf("min=%02d\n",pulse->tim.minute);
			printf("sec=%02d\n",pulse->tim.second);printf("id=%s\n",params->idV);

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
			printf("Comando Invï¿½lido\n");
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
			printf("-Caracteres NO permitidos: %s & [ESPACIO]\n","%");
			printf("-No se permite valor de 0 para limite de pulsos por dia\n");
			printf("-No se permite valor de 0 para identificador del vehï¿½culo\n");
			printf("Ingrese:\n");
			printf("id? : para saber la placa del vehï¿½culo\n");
			printf("id={id} : para cambiar la placa del vehï¿½culo\n");
			printf("ssid? : para saber el nombre de acceso a wifi\n");
			printf("ssid={nombre} : para sobreescribir el nombre de acceso a wifi\n");
			printf("pswd? : para saber la clave de acceso a wifi\n");
			printf("pswd={pass} : para sobreescribir la clave de acceso a wifi\n");
			printf("limit? : para saber los pulsos por dia limite\n");
			printf("limit={limit} : para sobreescribir los pulsos por dia limite actual\n");
			printf("config? : para saber la clave [config]\n");
			printf("config={config} : para sobreescribir la clave [config] actual\n");
			printf("exit: para salir del modo de configuraciï¿½n y reniciar el sistema\n");

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

	if(strchr(cmd,'%')!=NULL ||
			strchr(cmd,'&')!=NULL)
	{
		Params_FSM(INVALID_RCV,cmd);
		printf("Caracteres invalidos\n");
		return;
	}

	//*********** DEVICE *****************
	if(strcmp(cmd,params->configPass)==0){
		Params_FSM(CONFIG_RCV,cmd);
		return;
	}

	if((s=strstr(cmd,"ssid?")) != NULL) {
		if(s==cmd){
			Params_FSM(SSID_READ_RCV,NULL);
			return;
		}
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

	//************** WEB HOST ****************
	if(strcmp(cmd,"url?")==0) {
		Params_FSM(URL_READ_RCV,cmd);
		return;
	}
	if((s=strstr(cmd,"url=")) != NULL) {
		if(s==cmd && (s+4)!=NULL){
			Params_FSM(URL_WRITE_RCV,s+4);
			return;
		}
	}
	//**************************************
	if(strcmp(cmd,"exit")==0){ //RESETEO
		Params_FSM(EXIT_RCV,cmd);
		return;
	}

	//Para los desarrolladores
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
        Debug("Tamaño de la cola de patrones muy pequeï¿½o");
        uart_flush_input(0);
    }
}


void Params_Rx_Init(void) {
	gpio_config_t io_conf;

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
        Debug("config uart parameter failed");

    if (uart_set_pin(0, -1, 3, -1, -1) != ESP_OK)
    	Debug("config uart gpio failed");

    if (uart_driver_install(0, 256, 0,16, &params->queueCmd, 0) != ESP_OK)
    	Debug("install uart driver failed");

    /* Set pattern interrupt, used to detect the end of a line */
    uart_enable_pattern_det_baud_intr(0, '\n', 1, 10000, 10, 10);
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
				Debug("HW FIFO Overflow");
				uart_flush(0);
				xQueueReset(params->queueCmd);
				break;
			case UART_BUFFER_FULL:
				Debug("Ring Buffer Full");
				uart_flush(0);
				xQueueReset(params->queueCmd);
				break;
			case UART_BREAK:
				Debug("Rx Break");
				break;
			case UART_PARITY_ERR:
				Debug("Parity Error");
				break;
			case UART_FRAME_ERR:
				Debug("Frame Error");
				break;
			case UART_PATTERN_DET:
				cmd_uart_pattern();
				break;
			default:
				Debug("unknown uart event type: %d", event.type);
				break;
		}
	}
}




