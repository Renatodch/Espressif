/*
 * LCD_20x4.c
 *
 *  Created on: 21 jul. 2020
 *      Author: Renato
 */

#include "main.h"
#include "LCD_20x4.h"

LCD_t LCD;

void Lcd_Init(uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize)
{
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = (1ULL<<LCD_D4)|(1ULL<<LCD_D5)|(1ULL<<LCD_D6)|(1ULL<<LCD_D7)|(1ULL<<LCD_E)|(1ULL<<LCD_RS);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

	gpio_set_level(LCD_D4,0);
	gpio_set_level(LCD_D5,0);
	gpio_set_level(LCD_D6,0);
	gpio_set_level(LCD_D7,0);
	gpio_set_level(LCD_E,0);
	gpio_set_level(LCD_RS,0);

	LCD._cols = lcd_cols;
	LCD._rows = lcd_rows;
	LCD._charsize = charsize;


	/*Configurar*/
	//Wire.begin();
	LCD._displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (LCD._rows > 1) {
		LCD._displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((LCD._charsize != 0) && (LCD._rows == 1)) {
		LCD._displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	ets_delay_us(50000);//delay(50);


	// Now we pull both RS and R/W low to begin commands
	//Lcd_sendNibble(LCD._backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	ets_delay_us(1000000);//delay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	Lcd_write4bits(0x03);
	ets_delay_us(4500); // wait min 4.1ms

	// second try
	Lcd_write4bits(0x03);
	ets_delay_us(4500); // wait min 4.1ms

	// third go!
	Lcd_write4bits(0x03);
	ets_delay_us(150);

	// finally, set to 4-bit interface
	Lcd_write4bits(0x02);


	// set # lines, font size, etc.
	Lcd_command(LCD_FUNCTIONSET | LCD._displayfunction);

	// turn the display on with no cursor or blinking default
	LCD._displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	Lcd_Clear();

	// Initialize to default text direction (for roman languages)
	LCD._displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	Lcd_command(LCD_ENTRYMODESET | LCD._displaymode);

	home();


	//vTaskDelay(1000/portTICK_PERIOD_MS);

	//backlight();
	//Lcd_print("Hello, world!");


}

void Lcd_Clear(void){
	Lcd_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	ets_delay_us(2000);  // this command takes a long time!
}

void home(void){
	Lcd_command(LCD_RETURNHOME);  // set cursor position to zero
	ets_delay_us(2000);  // this command takes a long time!
}

void setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > LCD._rows) {
		row = LCD._rows-1;    // we count rows starting w/0
	}
	Lcd_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay(void) {
	LCD._displaycontrol &= ~LCD_DISPLAYON;
	Lcd_command(LCD_DISPLAYCONTROL | LCD._displaycontrol);
}
void display(void) {
	LCD._displaycontrol |= LCD_DISPLAYON;
	Lcd_command(LCD_DISPLAYCONTROL | LCD._displaycontrol);
}



void Lcd_command(uint8_t cmd){
	gpio_set_level(LCD_RS,0);

	Lcd_sendNibble((cmd & 0xF0)>>4);
	gpio_set_level(LCD_E,1);
	ets_delay_us(1);
	gpio_set_level(LCD_E,0);
	ets_delay_us(50);

	Lcd_sendNibble(cmd & 0x0F);
	gpio_set_level(LCD_E,1);
	ets_delay_us(1);
	gpio_set_level(LCD_E,0);
	ets_delay_us(50);
}

void Lcd_write(uint8_t input){
	gpio_set_level(LCD_RS,1);
	ets_delay_us(10);

	Lcd_send(input,1);

	gpio_set_level(LCD_RS,0);

}

void Lcd_send(uint8_t value, uint8_t mode){

	Lcd_write4bits((value & 0xF0)>>4);
	Lcd_write4bits(value & 0x0F);
}

void Lcd_write8bits(uint8_t value) {
	Lcd_sendNibble((value & 0xF0)>>4);
	Lcd_sendNibble(value);
	Lcd_enable();
}

void Lcd_write4bits(uint8_t value) {
	Lcd_sendNibble(value & 0x0F);
	ets_delay_us(1);
	Lcd_enable();
}

void Lcd_enable(void){
	gpio_set_level(LCD_E,1);
	ets_delay_us(1);
	gpio_set_level(LCD_E,0);
	ets_delay_us(50);
}

void Lcd_sendNibble(uint8_t value)
{
	uint32_t bitnibble=0;

	bitnibble = (uint32_t)value & 0x01;
	gpio_set_level(LCD_D4,bitnibble);
	bitnibble = ((uint32_t)value & 0x02)>>1;
	gpio_set_level(LCD_D5,bitnibble);
	bitnibble = ((uint32_t)value & 0x04)>>2;
	gpio_set_level(LCD_D6,bitnibble);
	bitnibble = ((uint32_t)value & 0x08)>>3;
	gpio_set_level(LCD_D7,bitnibble);
}


void Lcd_WriteText(char row, unsigned char col, char *format,...)
{
	char address_d = 0; // address of the data in the screen.
	char str[64]={0};
	va_list arglist;
	char *inputText=0;

	va_start(arglist,format);
	vsnprintf(str,sizeof(str),format,arglist);
	va_end(arglist);

	inputText = str;


	switch(row)
	{
		case 0: address_d = 0x80 + col; // 00h - 13h
						break;
		case 1: address_d = 0xC0 + col; // 14h - 27h
						break;
		case 2: address_d = 0x94 + col; // 40h - 53h
						break;
		case 3: address_d = 0xD4 + col; // 54h - 67h
						break;
		default: address_d = 0x80 + col; // returns to first row if invalid row number is detected
						break;
	}


	while((*inputText)&&(*inputText!=0x0D)) // Place a string, letter by letter.
	{
			Lcd_command((uint8_t)address_d++);
			Lcd_write((uint8_t)*inputText++);
	}
}


void Lcd_Task(void* arg){
	char strGPS[16];
	char strSTA[16];
	char strRssi[16];
	char strSsid[32];
	//char fragment[21]={0};
	//char total[41]="Esto es una prueba  Holaaaa   SAGE_ELEC ";
	bool f = 1;
	char timeout =0 ;
	//uint32_t j=0,i = 0,pos,k=0;
	Debug("LCD_TASK\n");
	setCursor(0,0);

	//strlcpy(fragment,total,sizeof(fragment));
	//fragment[sizeof(fragment)-1]=0;
	while(1){


		if(f){
			Lcd_Clear();
			home();
			//setCursor(0,0);

			if(myGPS.sats_in_use<5) strcpy(strGPS,"SIN GPS");
			else if(myGPS.sats_in_use>=5) strcpy(strGPS,"CON GPS");

			if(sta->state==CONECTADO_AP || sta->state == ENVIANDO_DATOS)	strcpy(strSTA,"/CONECTADO");
			else if(sta->state == LEYENDO_AP)								strcpy(strSTA,"");
			else if(sta->state == CONECTANDO_A_AP ) 						strcpy(strSTA,"/CONECTANDO..");


			if(strcmp((char *)sta->ap_record.ssid,"")!=0){
				Lcd_WriteText(0,0,"%.9s/Intensidad",state.str);
				if(sta->ap_record.rssi > MAX_DB)
					strcpy(strRssi,"Buena");
				else if(sta->ap_record.rssi <= MAX_DB && sta->ap_record.rssi > MIN_DB )
					strcpy(strRssi,"Regular");
				else if(sta->ap_record.rssi <= MIN_DB)
					strcpy(strRssi,"Mala");

				sprintf(strSsid,(char*)sta->ap_record.ssid);
				while(strlen(strSsid)<9) strcat(strSsid," ");
				Lcd_WriteText(1,0,"AP:%.9s %.7s",strSsid,strRssi);
			}
			else{
				Lcd_WriteText(0,0,"%.9s",state.str);
				Lcd_WriteText(1,0,"No se encuentra AP..");
			}

			Lcd_WriteText(2,0,"%s%s",strGPS,strSTA);

			if(sto->pWrite != sto->pRead + sto->pPage)
				Lcd_WriteText(3,0,"Hay Pulsos");
			else
				Lcd_WriteText(3,0,"No Hay Pulsos");

		}
		else{
			Lcd_Clear();
			home();

			//setCursor(0,0);
			Lcd_WriteText(0,0,"Pulsos/Tiempo Total");
			Lcd_WriteText(1,0,"%06d %07.1fs",pulse->xDia,pulse->acc);
			Lcd_WriteText(2,0,"Ultimo pulso:");
			Lcd_WriteText(3,0,"%04d/%02d/%02d %02d:%02d:%02d",pulse->date.year,pulse->date.month,pulse->date.day,pulse->tim.hour,pulse->tim.min,pulse->tim.sec);

		}

		if(timeout<7) f=1;
		else if(timeout>=7 && timeout<10) f = 0;
		else timeout = 0;

		ets_delay_us(1000000);
		timeout++;

		/*
		Lcd_Clear();
		setCursor(0,0);

		Lcd_WriteText(0,0,fragment);

		j++;
		for(i = 0; i< (sizeof(fragment)-1);i++){
			pos = i+j-1;

			if(i==(sizeof(fragment)-2))  //21
			{
				if((pos) >= (sizeof(total)-1))
				{
					fragment[i] = total[k++];
					if(k==sizeof(fragment)-1){
						k=0;
						j=0;
					}
				}
				else{
					fragment[i] = total[pos];
				}
			}
			else // i =21
				fragment[i] = fragment[i+1];
		}

		fragment[i]=0;
				ets_delay_us(1000000);

*/
	}
}
