/*
 * LCD_20x4.h
 *
 *  Created on: 21 jul. 2020
 *      Author: Renato
 */

#ifndef MAIN_COMPONENTS_LCD_20X4_LCD_20X4_H_
#define MAIN_COMPONENTS_LCD_20X4_LCD_20X4_H_

#define LCD_D7	(13)
#define LCD_D6	(12)
#define LCD_D5	(14)
#define LCD_D4	(27)
#define LCD_E	(26)
#define LCD_RS	(25)

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

typedef struct{

	uint8_t _addr;
	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;
	uint8_t _cols;
	uint8_t _rows;
	uint8_t _charsize;
}LCD_t;

//static void send(uint8_t, uint8_t);
//static void write4bits(uint8_t);
//static void expanderWrite(uint8_t);
//static void pulseEnable(uint8_t);
void Lcd_Clear(void);
void home(void);
void setCursor(uint8_t col, uint8_t row);
// Turn the display on/off (quickly)
void noDisplay(void);
void display(void);
void backlight(void);

void Lcd_Init( uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize);
void Lcd_WriteText(char row, unsigned char col, char *format,...);
void Lcd_Task(void* arg);


/*LOW LEVEL*/
void Lcd_sendNibble(uint8_t value);
void Lcd_command(uint8_t command);
void Lcd_write(uint8_t input);
void Lcd_send(uint8_t value, uint8_t mode);
void Lcd_write4bits(uint8_t value);
void Lcd_enable(void);
#endif /* MAIN_COMPONENTS_LCD_20X4_LCD_20X4_H_ */
