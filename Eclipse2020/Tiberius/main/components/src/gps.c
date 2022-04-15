/*
 * gps.c
 *
 *  Created on: 28 feb. 2020
 *      Author: Renato
 */

#include "main.h"
#include "gps.h"

ESP_EVENT_DEFINE_BASE(ESP_NMEA_EVENT);


TGPS_Parser	   *gps;
TGPS 			myGPS;

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	TGPS_Parser * GPS = NULL;

    switch (event_id)
    {
		case GPS_UPDATE:
			GPS = (TGPS_Parser *)event_data;

			/*
			GPS->parent.tim.hour += TIME_ZONE;
			if(GPS->parent.tim.hour>=24)
				GPS->parent.tim.hour -=24;
*/
			GPS->parent.date.year += YEAR_BASE;

			//day != 0: evita resetear antes de cargar parametro...
			/*
			if((pulse->date.day != GPS->parent.date.day) && (pulse->date.day != 0 )&& (GPS->parent.date.day != 0))
			{
				pulse->acc 	= 0.0;
				pulse->xDia = 0;
				Storage_Save_Info("pulsos",&pulse->xDia,INT,ACC_SECTOR);
				Storage_Save_Info("acc",&pulse->acc,DOUBLE,ACC_SECTOR);
			}
*/
			memcpy(&myGPS,&GPS->parent,sizeof(TGPS));

			Debug("%d/%d/%d %d:%d:%d.%d => \r\n"
					 "\t\t\t\t\t\tlatitude   = %.05f�N\r\n"
					 "\t\t\t\t\t\tlongitude = %.05f�E\r\n"
					 "\tSat in use = %d"
					 "\tSat in view = %d",
					 myGPS.date.year, myGPS.date.month, myGPS.date.day,
					 myGPS.tim.hour, myGPS.tim.minute, myGPS.tim.second, myGPS.tim.thousand,
					 myGPS.latitude, myGPS.longitude,myGPS.sats_in_use,myGPS.sats_in_view);

			break;
		case GPS_UNKNOWN:
			/* print unknown statements */
			Debug("Unknown statement:%s", (char *)event_data);
			break;
		default:
			break;
    }
}

void Gps_Init(void){

	ESP_LOGI(TAG_GPS,"Inicializando GPS");
	gps->parent.altitude = 0.0f;
	gps->parent.longitude = 0.0f;
	gps->parent.latitude = 0.0f;
	gps->parent.speed = 0.0f;
	gps->parent.date.year = 0;
	gps->parent.date.month = 0;
	gps->parent.date.day = 0;
	gps->parent.tim.hour = 0;
	gps->parent.tim.minute = 0;
	gps->parent.tim.second = 0;

	gps->buffer = calloc(1, GPS_PARSER_RUNTIME_BUFFER_SIZE);
    if (!gps->buffer){
    	ESP_LOGE(GPS_TAG, "calloc memory for runtime buffer failed");
    	//esp_restart();
    }

    gps->all_statements |= (1 << STATEMENT_GSA);
    gps->all_statements |= (1 << STATEMENT_GSV);
    gps->all_statements |= (1 << STATEMENT_GGA);
    gps->all_statements |= (1 << STATEMENT_RMC);
    gps->all_statements |= (1 << STATEMENT_GLL);
    gps->all_statements |= (1 << STATEMENT_VTG);
    /* Set attributes */
    gps->uart_port = GPS_UART;
    gps->all_statements &= 0xFE;
    /* Install UART driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    if (uart_param_config(gps->uart_port, &uart_config) != ESP_OK)
        ESP_LOGE(GPS_TAG, "config uart parameter failed");

    if (uart_set_pin(gps->uart_port, -1, 16,-1, -1) != ESP_OK)
        ESP_LOGE(GPS_TAG, "config uart gpio failed");

    if (uart_driver_install(gps->uart_port, 1024, 0, 16, &gps->event_queue, 0) != ESP_OK)
        ESP_LOGE(GPS_TAG, "install uart driver failed");

    /* Set pattern interrupt, used to detect the end of a line */
    uart_enable_pattern_det_baud_intr(gps->uart_port, '\n', 1, 10000, 10, 10);
    /* Set pattern queue size */
    uart_pattern_queue_reset(gps->uart_port, 16);
    uart_flush(gps->uart_port);

    /* Create Event loop */
    esp_event_loop_args_t loop_args = {
        .queue_size = GPS_EVENT_LOOP_QUEUE_SIZE,
        .task_name = NULL,
		//.task_stack_size = 2048,
    };
    if (esp_event_loop_create(&loop_args, &gps->event_loop_hdl) != ESP_OK)
        ESP_LOGE(GPS_TAG, "create event loop failed");

    /* Create GPS Parser task */
    xTaskCreatePinnedToCore(Gps_Task, "Gps_Task",4096,gps,3,&gps->tsk_hdl,0);

    //if (err != pdTRUE)     ESP_LOGE(GPS_TAG, "create NMEA Parser task failed");
    uart_flush( gps->uart_port);

    Debug("NMEA Parser init OK");

    /* register event handler for NMEA parser library */
    esp_event_handler_register_with(gps->event_loop_hdl, ESP_NMEA_EVENT, ESP_EVENT_ANY_ID,gps_event_handler, NULL);
}





/**
 * @brief Parse NMEA statements from GPS receiver
 *
 * @param esp_gps esp_gps_t type object
 * @param len number of bytes to decode
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error
 */
static esp_err_t gps_decode(TGPS_Parser *esp_gps, size_t len)
{
    const uint8_t *d = esp_gps->buffer;
    while (*d) {
        /* Start of a statement */
        if (*d == '$') {
            /* Reset runtime information */
            esp_gps->asterisk = 0;
            esp_gps->item_num = 0;
            esp_gps->item_pos = 0;
            esp_gps->cur_statement = 0;
            esp_gps->crc = 0;
            esp_gps->sat_count = 0;
            esp_gps->sat_num = 0;
            /* Add character to item */
            esp_gps->item_str[esp_gps->item_pos++] = *d;
            esp_gps->item_str[esp_gps->item_pos] = '\0';
        }
        /* Detect item separator character */
        else if (*d == ',') {
            /* Parse current item */
            parse_item(esp_gps);
            /* Add character to CRC computation */
            esp_gps->crc ^= (uint8_t)(*d);
            /* Start with next item */
            esp_gps->item_pos = 0;
            esp_gps->item_str[0] = '\0';
            esp_gps->item_num++;
        }
        /* End of CRC computation */
        else if (*d == '*') {
            /* Parse current item */
            parse_item(esp_gps);
            /* Asterisk detected */
            esp_gps->asterisk = 1;
            /* Start with next item */
            esp_gps->item_pos = 0;
            esp_gps->item_str[0] = '\0';
            esp_gps->item_num++;
        }
        /* End of statement */
        else if (*d == '\r') {
            /* Convert received CRC from string (hex) to number */
            uint8_t crc = (uint8_t)strtol(esp_gps->item_str, NULL, 16);

            //printf("crc en string: %i",crc);printf("\n");
            //printf("crc calculado: %i",esp_gps->crc);printf("\n");
            /* CRC passed */
            if (esp_gps->crc == crc) {

            	//printf((const char *)esp_gps->item_str[esp_gps->item_pos]);printf("\n");

                switch (esp_gps->cur_statement) {
                case STATEMENT_GGA:
                    esp_gps->parsed_statement |= 1 << STATEMENT_GGA;
                    break;
                case STATEMENT_GSA:
                    esp_gps->parsed_statement |= 1 << STATEMENT_GSA;
                    break;
                case STATEMENT_RMC:
                    esp_gps->parsed_statement |= 1 << STATEMENT_RMC;
                    break;
                case STATEMENT_GSV:
                    if (esp_gps->sat_num == esp_gps->sat_count) {
                        esp_gps->parsed_statement |= 1 << STATEMENT_GSV;
                    }
                    break;
                case STATEMENT_GLL:
                    esp_gps->parsed_statement |= 1 << STATEMENT_GLL;
                    break;
                case STATEMENT_VTG:
                    esp_gps->parsed_statement |= 1 << STATEMENT_VTG;
                    break;
                default:
                    break;
                }
                /* Check if all statements have been parsed */
                if (((esp_gps->parsed_statement) & esp_gps->all_statements) == esp_gps->all_statements) {
                    esp_gps->parsed_statement = 0;
                    /* Send signal to notify that GPS information has been updated */
                    esp_event_post_to(esp_gps->event_loop_hdl, ESP_NMEA_EVENT, GPS_UPDATE,
                                      esp_gps, sizeof(TGPS_Parser), 100 / portTICK_PERIOD_MS);

                    //memset(esp_gps->buffer,0x00,NMEA_PARSER_RUNTIME_BUFFER_SIZE);
                }
            } else {
                ESP_LOGD(GPS_TAG, "CRC Error for statement:%s", esp_gps->buffer);
            }
            if (esp_gps->cur_statement == STATEMENT_UNKNOWN) {
                /* Send signal to notify that one unknown statement has been met */
                esp_event_post_to(esp_gps->event_loop_hdl, ESP_NMEA_EVENT, GPS_UNKNOWN,
                                  esp_gps->buffer, len, 100 / portTICK_PERIOD_MS);

            }
        }
        /* Other non-space character */
        else {
            if (!(esp_gps->asterisk)) {
                /* Add to CRC */
                esp_gps->crc ^= (uint8_t)(*d);
            }
            /* Add character to item */
            esp_gps->item_str[esp_gps->item_pos++] = *d;
            esp_gps->item_str[esp_gps->item_pos] = '\0';
        }
        /* Process next character */
        d++;
    }
    return ESP_OK;
}

/**
 * @brief Handle when a pattern has been detected by uart
 *
 * @param esp_gps esp_gps_t type object
 */
static void esp_handle_uart_pattern(TGPS_Parser *esp_gps)
{
    int pos = uart_pattern_pop_pos(esp_gps->uart_port);
    if (pos != -1) {
        /* read one line(include '\n') */
        int read_len = uart_read_bytes(esp_gps->uart_port, esp_gps->buffer, pos + 1, 100 / portTICK_PERIOD_MS);
        /* make sure the line is a standard string */
        esp_gps->buffer[read_len] = '\0';

        //Debug(( char *)esp_gps->buffer);
        //printf("\n");


        /* Send new line to handle */
        if (gps_decode(esp_gps, read_len + 1) != ESP_OK) {
            ESP_LOGW(GPS_TAG, "GPS decode line failed");
        }
    } else {
        ESP_LOGW(GPS_TAG, "Pattern Queue Size too small");
        uart_flush_input(esp_gps->uart_port);
    }
}

/**
 * @brief NMEA Parser Task Entry
 *
 * @param arg argument
 */
void Gps_Task(void *arg)
{
	TGPS_Parser *esp_gps = (TGPS_Parser *)arg;
    uart_event_t event;
    while (1) {
        if (xQueueReceive(esp_gps->event_queue, &event, pdMS_TO_TICKS(200))) {
        	//ESP_LOGI(TAG_GPS, "UART2 Data Received");
            switch (event.type) {
            case UART_DATA:
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(GPS_TAG, "HW FIFO Overflow");
                uart_flush(esp_gps->uart_port);
                xQueueReset(esp_gps->event_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(GPS_TAG, "Ring Buffer Full");
                uart_flush(esp_gps->uart_port);
                xQueueReset(esp_gps->event_queue);
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
            	//Debug("UART pattern");
                esp_handle_uart_pattern(esp_gps);
                break;
            default:
                ESP_LOGW(GPS_TAG, "unknown uart event type: %d", event.type);
                break;
            }
        }
        /* Drive the event loop */
        esp_event_loop_run(esp_gps->event_loop_hdl, pdMS_TO_TICKS(50));
    }
}






/**
 * @brief parse latitude or longitude
 *              format of latitude in NMEA is ddmm.sss and longitude is dddmm.sss
 * @param esp_gps esp_gps_t type object
 * @return float Latitude or Longitude value (unit: degree)
 */
static float parse_lat_long(TGPS_Parser *esp_gps)
{
    float ll = strtof(esp_gps->item_str, NULL);
    int deg = ((int)ll) / 100;
    float min = ll - (deg * 100);
    ll = deg + min / 60.0f;
    return ll;
}

/**
 * @brief Converter two continuous numeric character into a uint8_t number
 *
 * @param digit_char numeric character
 * @return uint8_t result of converting
 */
static inline uint8_t convert_two_digit2number(const char *digit_char)
{
    return 10 * (digit_char[0] - '0') + (digit_char[1] - '0');
}

/**
 * @brief Parse UTC time in GPS statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_utc_time(TGPS_Parser *esp_gps)
{
    esp_gps->parent.tim.hour = convert_two_digit2number(esp_gps->item_str + 0);
    if(esp_gps->parent.tim.hour >= 5)
    	esp_gps->parent.tim.hour -= 5; //if 13 hour = 0 -> 8
    	//esp_gps->parent.tim.hour += TIME_ZONE;
    else
        esp_gps->parent.tim.hour += 19;


    esp_gps->parent.tim.minute = convert_two_digit2number(esp_gps->item_str + 2) ;
    esp_gps->parent.tim.second = convert_two_digit2number(esp_gps->item_str + 4);
    if (esp_gps->item_str[6] == '.') {
        uint16_t tmp = 0;
        uint8_t i = 7;
        while (esp_gps->item_str[i]) {
            tmp = 10 * tmp + esp_gps->item_str[i] - '0';
            i++;
        }
        esp_gps->parent.tim.thousand = tmp;
    }
}

/**
 * @brief Parse GGA statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gga(TGPS_Parser *esp_gps)
{
    /* Process GGA statement */
    switch (esp_gps->item_num) {
    case 1: /* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 2: /* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 3: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 4: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 5: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 6: /* Fix status */
        esp_gps->parent.fix = (gps_fix_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 7: /* Satellites in use */
        esp_gps->parent.sats_in_use = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 8: /* HDOP */
        esp_gps->parent.dop_h = strtof(esp_gps->item_str, NULL);
        break;
    case 9: /* Altitude */
        esp_gps->parent.altitude = strtof(esp_gps->item_str, NULL);
        break;
    case 11: /* Altitude above ellipsoid */
        esp_gps->parent.altitude += strtof(esp_gps->item_str, NULL);
        break;
    default:
        break;
    }
}


/**
 * @brief Parse GSA statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gsa(TGPS_Parser *esp_gps)
{
    /* Process GSA statement */
    switch (esp_gps->item_num) {
    case 2: /* Process fix mode */
        esp_gps->parent.fix_mode = (gps_fix_mode_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 15: /* Process PDOP */
        esp_gps->parent.dop_p = strtof(esp_gps->item_str, NULL);
        break;
    case 16: /* Process HDOP */
        esp_gps->parent.dop_h = strtof(esp_gps->item_str, NULL);
        break;
    case 17: /* Process VDOP */
        esp_gps->parent.dop_v = strtof(esp_gps->item_str, NULL);
        break;
    default:
        /* Parse satellite IDs */
        if (esp_gps->item_num >= 3 && esp_gps->item_num <= 14) {
            esp_gps->parent.sats_id_in_use[esp_gps->item_num - 3] = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        }
        break;
    }
}

/**
 * @brief Parse GSV statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gsv(TGPS_Parser *esp_gps)
{
    /* Process GSV statement */
    switch (esp_gps->item_num) {
    case 1: /* total GSV numbers */
        esp_gps->sat_count = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 2: /* Current GSV statement number */
        esp_gps->sat_num = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 3: /* Process satellites in view */
        esp_gps->parent.sats_in_view = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    default:
        if (esp_gps->item_num >= 4 && esp_gps->item_num <= 19) {
            uint8_t item_num = esp_gps->item_num - 4; /* Normalize item number from 4-19 to 0-15 */
            uint8_t index;
            uint32_t value;
            index = 4 * (esp_gps->sat_num - 1) + item_num / 4; /* Get array index */
            if (index < GPS_MAX_SATELLITES_IN_VIEW) {
                value = strtol(esp_gps->item_str, NULL, 10);
                switch (item_num % 4) {
                case 0:
                    esp_gps->parent.sats_desc_in_view[index].num = (uint8_t)value;
                    break;
                case 1:
                    esp_gps->parent.sats_desc_in_view[index].elevation = (uint8_t)value;
                    break;
                case 2:
                    esp_gps->parent.sats_desc_in_view[index].azimuth = (uint16_t)value;
                    break;
                case 3:
                    esp_gps->parent.sats_desc_in_view[index].snr = (uint8_t)value;
                    break;
                default:
                    break;
                }
            }
        }
        break;
    }
}


/**
 * @brief Parse RMC statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_rmc(TGPS_Parser *esp_gps)
{
    /* Process GPRMC statement */
    switch (esp_gps->item_num) {
    case 1:/* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 2: /* Process valid status */
        esp_gps->parent.valid = (esp_gps->item_str[0] == 'A');
        break;
    case 3:/* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 4: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 5: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 6: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 7: /* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) * 1.852;
        break;
    case 8: /* Process true course over ground */
        esp_gps->parent.cog = strtof(esp_gps->item_str, NULL);
        break;
    case 9: /* Process date */
        esp_gps->parent.date.day = convert_two_digit2number(esp_gps->item_str + 0);
        esp_gps->parent.date.month = convert_two_digit2number(esp_gps->item_str + 2);
        esp_gps->parent.date.year = convert_two_digit2number(esp_gps->item_str + 4);
        break;
    case 10: /* Process magnetic variation */
        esp_gps->parent.variation = strtof(esp_gps->item_str, NULL);
        break;
    default:
        break;
    }
}


/**
 * @brief Parse GLL statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gll(TGPS_Parser *esp_gps)
{
    /* Process GPGLL statement */
    switch (esp_gps->item_num) {
    case 1:/* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 2: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 3: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 4: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 5:/* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 6: /* Process valid status */
        esp_gps->parent.valid = (esp_gps->item_str[0] == 'A');
        break;
    default:
        break;
    }
}


/**
 * @brief Parse VTG statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_vtg(TGPS_Parser *esp_gps)
{
    /* Process GPVGT statement */
    switch (esp_gps->item_num) {
    case 1: /* Process true course over ground */
        esp_gps->parent.cog = strtof(esp_gps->item_str, NULL);
        break;
    case 3:/* Process magnetic variation */
        esp_gps->parent.variation = strtof(esp_gps->item_str, NULL);
        break;
    case 5:/* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) * 1.852;//knots to m/s
        break;
    case 7:/* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) / 3.6;//km/h to m/s
        break;
    default:
        break;
    }
}


/**
 * @brief Parse received item
 *
 * @param esp_gps esp_gps_t type object
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error
 */
esp_err_t parse_item(TGPS_Parser *esp_gps)
{
    esp_err_t err = ESP_OK;
    /* start of a statement */
    if (esp_gps->item_num == 0 && esp_gps->item_str[0] == '$') {
        if (0) {
        }
        else if (strstr(esp_gps->item_str, "GGA")) {
            esp_gps->cur_statement = STATEMENT_GGA;
        }
        else if (strstr(esp_gps->item_str, "GSA")) {
            esp_gps->cur_statement = STATEMENT_GSA;
        }
        else if (strstr(esp_gps->item_str, "RMC")) {
            esp_gps->cur_statement = STATEMENT_RMC;
        }
        else if (strstr(esp_gps->item_str, "GSV")) {
            esp_gps->cur_statement = STATEMENT_GSV;
        }
        else if (strstr(esp_gps->item_str, "GLL")) {
            esp_gps->cur_statement = STATEMENT_GLL;
        }
        else if (strstr(esp_gps->item_str, "VTG")) {
            esp_gps->cur_statement = STATEMENT_VTG;
        }
        else {
            esp_gps->cur_statement = STATEMENT_UNKNOWN;
        }
        goto out;
    }
    /* Parse each item, depend on the type of the statement */
    if (esp_gps->cur_statement == STATEMENT_UNKNOWN) {
        goto out;
    }
    else if (esp_gps->cur_statement == STATEMENT_GGA) {
        parse_gga(esp_gps);
    }

    else if (esp_gps->cur_statement == STATEMENT_GSA) {
        parse_gsa(esp_gps);
    }

    else if (esp_gps->cur_statement == STATEMENT_GSV) {
        parse_gsv(esp_gps);
    }
    else if (esp_gps->cur_statement == STATEMENT_RMC) {
        parse_rmc(esp_gps);
    }
    else if (esp_gps->cur_statement == STATEMENT_GLL) {
        parse_gll(esp_gps);
    }
    else if (esp_gps->cur_statement == STATEMENT_VTG) {
        parse_vtg(esp_gps);
    }
    else {
        err =  ESP_FAIL;
    }
out:
    return err;
}



