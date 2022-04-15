#include "main.h"

TWifi_Station * sta;
EventGroupHandle_t s_wifi_event_group;




static void wifi_event(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	TWifi_Station * station = (TWifi_Station *) arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        Debug("Desconectando de SSID:%s",(char *)sta->ssid,(char *)sta->psswd);
    	sta->state = LEYENDO_AP;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        memmove(station->ipv4,ip4addr_ntoa(&event->ip_info.ip),sizeof(station->ipv4));
        Debug(WIFI_STATION_TAG, "ip: %s",station->ipv4);

        sta->state = CONECTADO_AP;
    }
}



void Enviando_Datos(WIFI_EVT e){
	switch(e){
	case COMPLETADO:
	case FALLO_ENVIO:
		shutdown(sta->socket, 0);
		close(sta->socket);
		pulse->acc = 0.0; // regresamos a cero el acumulador
		pulse->xDia = 0;
		Storage_Save_Info("pulsos",&pulse->xDia,INT,ACC_SECTOR);
		Storage_Save_Info("acc",&pulse->acc,DOUBLE,ACC_SECTOR);
		esp_wifi_disconnect();
		sta->state = LEYENDO_AP;
		break;
	default: break;
	}
}

void Conectado_AP(WIFI_EVT e){
	switch(e){
		case FALLO_CONEXION_AL_SERVIDOR:
			esp_wifi_disconnect();
			sta->state = LEYENDO_AP;
			break;
		case CONECTO_AL_SERVIDOR:
			sta->state = ENVIANDO_DATOS;
			break;
		default:break;
	}
}

void Leyendo_AP(WIFI_EVT e){
   wifi_config_t wifi_config
		={
			.sta = {
				.ssid = "",
				.password = "",
				.bssid_set = false,
			},
		};
	int i;
	switch(e){
		case AP_RSSI_PULSOS:

			sta->state=CONECTANDO_A_AP;

			for(i=0;i<64;i++){
				wifi_config.sta.ssid[i] = sta->ssid[i];
			}
			for(i=0;i<64;i++){
				wifi_config.sta.password[i] = sta->psswd[i];
			}

			ESP_LOGI(WIFI_STATION_TAG,"ssid:%s",(char *)wifi_config.sta.ssid);
			ESP_LOGI(WIFI_STATION_TAG,"password:%s",(char *)wifi_config.sta.password);

			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
			ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

			Debug("Conectando a AP: %s",(char *)sta->ssid);
			esp_wifi_connect();
			break;
		default:
			break;
	}
}


void Wifi_Station_Task(void *arg){

	char temp[64] = {0};
	uint8_t retry = 0;

	tcp_client_t * tcp_client = sta->tcp_client;
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(tcp_client->ipv4);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(tcp_client->port);
    char addr_str[128] = {0};
    int addr_family;
    int ip_protocol;

	uint16_t ap_number,i;
	while(1)
	{
		/*Tarea Automatica*/
		if(sta->state == LEYENDO_AP)
		{
			Debug("Leyendo AP entry...");
			/*Buscando APs*/
			wifi_scan_config_t wifi_scan_config={
						.ssid = NULL,
						.bssid = NULL,
						.channel = 0,
						.scan_time.active = {
						  .min = 500,
						  .max = 1500
						},
						.scan_type = WIFI_SCAN_TYPE_ACTIVE,
						.show_hidden = false
			};

			//sta->ap_record_num = 0xFF;

			i=32;

			esp_wifi_scan_start(&wifi_scan_config, 1);
			esp_wifi_scan_get_ap_num(&ap_number);
		//	loop=ap_number;
			Debug("ap_number:%d",ap_number);
			wifi_ap_record_t *ap_records = calloc(ap_number,sizeof(wifi_ap_record_t));

			esp_wifi_scan_get_ap_records(&i,  ap_records);

			for(i=0; i<ap_number;i++)
			{
				Debug("ap number %d ssid:%s rssi:%d",i,(char*)ap_records[i].ssid,ap_records[i].rssi);
				if(strcmp((char*)ap_records[i].ssid,(char*)sta->ssid/*sizeof(ap_records[i].ssid*/)==0){
					Debug("Se encontró AP:%s rssi:%d",sta->ssid,ap_records[i].rssi);
					memmove(&sta->ap_record,&ap_records[i],sizeof(wifi_ap_record_t));
					//sta->ap_record_num = i;
					break;
				}
			}

			if(i==ap_number){
				memset(&sta->ap_record,0x00,sizeof(sta->ap_record));
				//Debug("No se encuentra AP: %s",(char*)sta->ssid);
			}
			else{
				if(sta->ap_record.rssi > MAX_DB){
					Debug("rssi > %d", MAX_DB);
					if(sto->pWrite != sto->pRead + sto->pPage){
						Debug("Hay Pulsos");
						Leyendo_AP(AP_RSSI_PULSOS);
					}
					else
						Debug("No hay Pulsos");
				}
				else{
					Debug("Pero tiene Baja Señal");
				}
			}

			//free(sta->ap_records);
		}
		/*############################ CONECTANDO A AP #########################################*/
		if(sta->state==CONECTANDO_A_AP){
			//"Esperando asignación de IPv4...";
		}
		/*############################ CONECTANDO AL SERVIDOR ######################################*/
		if(sta->state==CONECTADO_AP)
		{
			Debug("Conectado AP entry...");
			Debug("conectado al AP SSID:%s password:%s",(char *)sta->ssid, (char *)sta->psswd);

			dest_addr.sin_addr.s_addr = inet_addr(tcp_client->ipv4);
		    dest_addr.sin_port = htons(tcp_client->port);

			addr_family = AF_INET;
			ip_protocol = IPPROTO_IP;
			inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

			if(!(sta->socket = socket(addr_family, SOCK_STREAM, ip_protocol)))
			{
				Debug("No se pudo crear socket: errno %d", errno);
				Conectado_AP(FALLO_CONEXION_AL_SERVIDOR);
				continue;
			}
			Debug("Socket creado, conectando a %s:%d", tcp_client->ipv4, tcp_client->port);

			int err = connect(sta->socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if (err) {
				Debug("No se pudo conectar al servidor: errno %d", errno);
				Conectado_AP(FALLO_CONEXION_AL_SERVIDOR);
				continue;
			}
			Debug("Conectó al Servidor");
			Conectado_AP(CONECTO_AL_SERVIDOR);
		}
		/*############################### ENVIANDO DATOS ##############3######################################*/
		if(sta->state == ENVIANDO_DATOS){
			State(ENVIANDO);
			Debug("Enviando Datos entry...");
			i=0;

			ESP_ERROR_CHECK(esp_partition_mmap(sto->partition, sto->pPage, MMAP_SIZE_64KB, SPI_FLASH_MMAP_DATA, &sto->pMap, &sto->map_handle));
			char * p = (char *)sto->pMap;

			while((sto->pRead + sto->pPage) != sto->pWrite)
			{
				temp[i] = *(p + sto->pRead);

				// Cuando enviar??
				// 1.Cuando temp[j] = null (final de trama)
				// 2.Cuando se desborda bloque de 64KB
				// 3.Cuando se desborda el espacio para datos (0x1F4000 = 31*64 + 0x4000)
				if((temp[i++] == '\0') || (sto->pRead + 1 == MMAP_SIZE_64KB) || ((sto->pRead +1) == 0x4000 && (sto->pPage == 0x1F*MMAP_SIZE_64KB)))
				{
					Debug("datos a enviar por wifi: %s",temp);

					retry = 0;
					while(retry < 5)
					{
						int flag = 1;
						setsockopt(sta->socket, IPPROTO_TCP, TCP_NODELAY, (void *) &flag, sizeof(int));
						int err = send(sta->socket, temp, i, 0);
						flag = 0;
						setsockopt(sta->socket, IPPROTO_TCP, TCP_NODELAY, (void *) &flag, sizeof(int));

						if (err < 0)
						{
							retry++;
							Debug("Error ocurrió durante el envio: errno %d", errno);

						}else break;
					}
					Debug("len: %u, temp[%d]=%d , *(p+%d)=%c",i,i-1,temp[i-1], sto->pRead ,*(p + sto->pRead ));

					if(retry == 5){ 						// hacemos algo por que nova bien la conexion;
						sto->pRead = sto->pRead - (i - 1); //read vuelve desde donde empezo j
						break;
					}

					memset(temp,0x00,sizeof(temp));
					i=0;
				}

				// Situaciones de desborde
				// ultima direccion posible
				if((sto->pRead +1) == 0x4000 && (sto->pPage == 0x1F*MMAP_SIZE_64KB))
				{
					ESP_LOGI(STORAGE_TAG,"Borrando ultimos datos...");
					ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition,sto->pPage,0x4000));

					sto->pRead = 0;
					sto->pPage = 0;

					ESP_ERROR_CHECK(esp_partition_mmap(sto->partition, sto->pPage, MMAP_SIZE_64KB, SPI_FLASH_MMAP_DATA, &sto->pMap, &sto->map_handle));
					p = (char *)sto->pMap;
				}
				// ultima direccion de bloque
				else if((sto->pRead +1) == MMAP_SIZE_64KB){
					ESP_LOGI(STORAGE_TAG,"Borrando datos...");
					ESP_ERROR_CHECK(esp_partition_erase_range(sto->partition,sto->pPage,MMAP_SIZE_64KB));

					sto->pRead = 0;
					sto->pPage += MMAP_SIZE_64KB;

					ESP_ERROR_CHECK(esp_partition_mmap(sto->partition, sto->pPage, MMAP_SIZE_64KB, SPI_FLASH_MMAP_DATA, &sto->pMap, &sto->map_handle));
					p = (char *)sto->pMap;
				}
				else
				{
					sto->pRead++;
				}
				vTaskDelay(1);

			}

			Storage_Save_Reference(REFERENCE_READ_SECTOR);
			Storage_Save_Reference(REFERENCE_PAGE_SECTOR);

			if(sto->pWrite == sto->pRead + sto->pPage){
				Enviando_Datos(COMPLETADO);
			}
			else{
				Enviando_Datos(FALLO_ENVIO);
			}
			State(LISTO);

		}

		vTaskDelay(100/portTICK_PERIOD_MS);

	}
}


void WifiStation_Init()
{
	Debug("Inicializando Wifi Station");

	memset(sta->ipv4,0x00,sizeof(sta->ipv4));

	sta->tcp_client = calloc(1,sizeof(tcp_client_t));

	/*Paramaetros del servidor*/
	strcpy(sta->tcp_client->ipv4,"");
	sta->tcp_client->port = 0;

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event, sta));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event, sta));


    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config
	={
		.sta = {
			.ssid = "",
			.password = "",
			.bssid_set = false,
		},
	};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    esp_wifi_start();

    sta->state=LEYENDO_AP;
}


