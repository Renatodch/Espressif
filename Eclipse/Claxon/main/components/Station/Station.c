#include "main.h"

TWifi_Station * sta;
EventGroupHandle_t s_wifi_event_group;


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
        	Debug("HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED://1
        	Debug("HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT: //2
        	Debug("HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER://3
        	Debug("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
        	Debug("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            Debug("HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
        	Debug("HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
            	Debug("Last esp error code: 0x%x", err);
            	Debug("Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

//const char tiberius_com_root_cert_pem_start[] asm("_binary_tiberius_com_root_cert_pem_start");


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
        Debug("ip: %s",station->ipv4);

        sta->state = CONECTADO_AP;
    }
}

void Conectado_AP(WIFI_EVT e){
	switch(e){
	case COMPLETADO:
		pulse->acc = 0.0; // regresamos a cero el acumulador
		pulse->xDia = 0;
		Storage_Save_Info("pulsos",&pulse->xDia,INT,ACC_SECTOR);
		Storage_Save_Info("acc",&pulse->acc,DOUBLE,ACC_SECTOR);
	case FALLO_ENVIO:
		shutdown(sta->socket, 0);
		close(sta->socket);
		esp_wifi_disconnect();
		sta->state = LEYENDO_AP;
		break;
	default: break;
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

			Debug("ssid:%s",(char *)wifi_config.sta.ssid);
			Debug("password:%s",(char *)wifi_config.sta.password);

			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
			ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

			Debug("Conectando a AP: %s",(char *)sta->ssid);
			esp_wifi_connect();
			break;
		default:
			break;
	}
}

#undef CONFIG_ESP_HTTP_CLIENT_ENABLE_HTTPS
void Wifi_Station_Task(void *arg){

    char post_data[256] ={0};
	uint16_t ap_number,i;
	//esp_http_client_handle_t client = NULL;
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

			i=32; //max ap number for searching

			esp_wifi_scan_start(&wifi_scan_config, 1);
			esp_wifi_scan_get_ap_num(&ap_number);

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

			free(ap_records);
		}
		/*############################ CONECTANDO A AP #########################################*/
		if(sta->state==CONECTANDO_A_AP){};//"Esperando asignación de IPv4..."
		/*############################ CONECTANDO AL REMOTE HOST ######################################*/
		if(sta->state==CONECTADO_AP)
		{
			Debug("Conectado al AP SSID:%s password:%s",(char *)sta->ssid, (char *)sta->psswd);
			State(ENVIANDO);
			i=0;

		    esp_http_client_config_t config = {
		        .url = sta->url,
		        .event_handler = _http_event_handler,
				//.cert_pem = tiberius_com_root_cert_pem_start,

				.is_async = false,
		    };

		    esp_http_client_handle_t client = esp_http_client_init(&config);

		    Debug("sta->url = %s", sta->url);
		    esp_http_client_set_url(client,sta->url);

		    esp_http_client_set_method(client, HTTP_METHOD_POST);

		    //strcpy(post_data,"id=100&date=2020-11-09 15:03:01&len=2.22&acc=4.66&total=22&lat=15.56&lon=-9.63");


			ESP_ERROR_CHECK(esp_partition_mmap(sto->partition, sto->pPage, MMAP_SIZE_64KB, SPI_FLASH_MMAP_DATA, &sto->pMap, &sto->map_handle));
			char * p = (char *)sto->pMap;

			while((sto->pRead + sto->pPage) != sto->pWrite)
			{
				post_data[i] = *(p + sto->pRead);

				// Enviamos cuando
				// 1.Cuando post_data[j] = null (final de trama)
				// 2.Cuando se desborda bloque de 64KB
				// 3.Cuando se desborda el espacio para datos (0x1F4000 = 31*64 + 0x4000)
				if(
					(post_data[i++] == '\0') || (sto->pRead + 1 == MMAP_SIZE_64KB) ||
					(((sto->pRead +1) == 0x4000) && (sto->pPage == 0x1F*MMAP_SIZE_64KB))
				)
				{
					Debug("datos a enviar por wifi: %s",post_data);

					esp_http_client_set_url(client,sta->url);
				    esp_http_client_set_method(client, HTTP_METHOD_POST);
					esp_http_client_set_post_field(client, (const char*)post_data, strlen(post_data));
					esp_err_t err = esp_http_client_perform(client);
					if (err == ESP_OK) {
						Debug("HTTP POST Status = %d, content_length = %d",
								esp_http_client_get_status_code(client),
								esp_http_client_get_content_length(client));
					} else {
						Debug("HTTP POST request failed: %s", esp_err_to_name(err));
						sto->pRead = sto->pRead - (i-1); //read vuelve desde donde empezo i, no va bien la conexion
						break;
					}

					Debug("len: %u, temp[%d]=%d , *(p+%d)=%c",i,i-1,post_data[i-1], sto->pRead ,*(p + sto->pRead ));

					memset(post_data,0x00,sizeof(post_data)); //limpiamos buffer
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
				else if((sto->pRead +1) == MMAP_SIZE_64KB)
				{
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

			esp_http_client_cleanup(client);

			Storage_Save_Reference(REFERENCE_READ_SECTOR);
			Storage_Save_Reference(REFERENCE_PAGE_SECTOR);

			if(sto->pWrite == sto->pRead + sto->pPage){
				Conectado_AP(COMPLETADO);
			}
			else{
				Conectado_AP(FALLO_ENVIO);
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



