/*
 * CCDebugger.c
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#include <ccDebugger.h>
#include "stm32f4xx.h"
#include "periph.h"

uint8_t instr[16] = {
// Default CCDebug instruction set for CC254x
1, //INSTR_VERSION
0x40,//I_HALT
0x48,//I_RESUME
0x20,//I_RD_CONFIG
0x18,//I_WR_CONFIG
0x51,//I_DEBUG_INSTR_1
0x52,//I_DEBUG_INSTR_2
0x53,//I_DEBUG_INSTR_3
0x68,//I_GET_CHIP_ID
0x28,//I_GET_PC
0x30,//I_READ_STATUS
0x58,//I_STEP_INSTR
0x10,//I_CHIP_ERASE
0x00,//
0x00,//
0x00//
};

void initDebugger(){

}

void setActive( bool on ){

}

/**
* Return the error flag
*/
uint8_t error(){

  return 1;
}

////////////////////////////
// High-Level interaction
////////////////////////////

/**
* Enter debug mode
*/
uint8_t debugEnter(){
	low(RST);
	delay(500);//
	high(DC);
	delay(5); //4
	low(DC);
	delay(5);//4
	high(DC);
	delay(5);//4
	low(DC);
	delay(500);
	high(RST); //high
	delay(500);
	return 1;
}

/**
* Exit from debug mode
*/
uint8_t debugExit(){
	uint8_t bAns;

	write( instr[I_RESUME] ); // RESUME
	switchRead(255);
	bAns = read(); // debug status
	switchWrite();

	return 0;
}

/**
* Execute a CPU instructuion
*/
uint8_t exec1( uint8_t oc0 ){
	uint8_t bAns;

	write( instr[I_DEBUG_INSTR_1] ); // DEBUG_INSTR + 1b
	write( oc0 );
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t exec2( uint8_t oc0, uint8_t oc1 ){
	uint8_t bAns;

	write( instr[I_DEBUG_INSTR_2] ); // DEBUG_INSTR + 2b
	write( oc0 );
	write( oc1 );
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t exec3( uint8_t oc0, uint8_t oc1, uint8_t oc2 ){
	uint8_t bAns;

	write( instr[I_DEBUG_INSTR_3] ); // DEBUG_INSTR + 3b
	write( oc0 );
	write( oc1 );
	write( oc2 );
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t execi( uint8_t oc0, uint16_t c0 ){
	uint8_t bAns;

	write( instr[I_DEBUG_INSTR_3] ); // DEBUG_INSTR + 3b
	write( oc0 );
	write( (c0 >> 8) & 0xFF );
	write(  c0 & 0xFF );
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint16_t getChipID(){
	uint16_t bAns = 0;
	write(instr[I_GET_CHIP_ID]);
	switchRead(255);
	bAns = read()<<8;
	bAns |= read();
	switchWrite(DD);
	return bAns;
}

uint16_t getPC(){
	uint16_t bAns = 0;

	write( instr[I_GET_PC] ); // GET_PC
	switchRead(255);
	bAns = read()<<8; // High order
	bAns |= read(); // Low order
	switchWrite();

	return bAns;
}

uint8_t getStatus(){
	uint8_t bAns = 0;

	write( instr[I_READ_STATUS] ); // READ_STATUS
	switchRead(255);
	bAns = read(); // debug status
	switchWrite();
	return bAns;
}

uint8_t resume(){
	uint8_t bAns;

	write( instr[I_RESUME] ); //RESUME
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t halt(){
	uint8_t bAns;

	write( instr[I_HALT] ); //HALT
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t step(){
	uint8_t bAns;

	write( instr[I_STEP_INSTR] ); // STEP_INSTR
	switchRead(255);
	bAns = read(); // Accumulator
	switchWrite();

	return bAns;
}

uint8_t getConfig(){
	uint8_t bAns;
	write( instr[I_RD_CONFIG] ); // RD_CONFIG
	switchRead(255);
	bAns = read(); // Config
	switchWrite();
	return bAns;
}

uint8_t setConfig( uint8_t config ){
	uint8_t bAns;
	write( instr[I_WR_CONFIG] ); // WR_CONFIG
	write( config );
	switchRead(255);
	bAns = read(); // Config
	switchWrite();
	return bAns;
}

uint8_t chipErase(){
	uint8_t bAns;

	write( instr[I_CHIP_ERASE] ); // CHIP_ERASE
	switchRead(255);
	bAns = read(); // Debug status
	switchWrite();

	return bAns;
}

////////////////////////////
// Low-level interaction
////////////////////////////

uint8_t write( uint8_t data ){
    change_to_output(DD);
	for(uint8_t cn = 8; cn; cn--){
		// First put data bit on bus
		if (data & 0x80) high(DD);
		else			 low(DD);

		high(DC);
		data <<= 1;
		delay(3);
		low(DC);
		delay(3);
	}

	return 1;
}

uint8_t switchRead( uint8_t maxWaitCycles){
	//int32_t maxWaitCycles = 255;
	bool didWait = false;
	change_to_input(DD);
	// Wait at least 83 ns before checking state t(dir_change)
	delay(3);

	// Wait for DD to go LOW (Chip is READY)
	while (input(DD)) {
		// Do 8 clock cycles
		for (uint8_t cnt = 8; cnt; cnt--) {
		  high(DC);
		  delay(3);
		  low(DC);
		  delay(3);
		}

		// Let next function know that we did wait
		didWait = true;

		// Check if we ran out if wait cycles
		if (!--maxWaitCycles) {

		  // If we are waiting for too long, we have lost the chip,
		  // so also assume we are out of debugging mode
		  //errorFlag = CC_ERROR_NOT_WIRED;
		  //inDebugMode = 0;

		  //digitalWrite(pinReadLED, LOW);

		  return 0;
		}

	  }

	  // Wait t(sample_wait)
	  if (didWait) delay(3); //2

	return 1;
}

uint8_t switchWrite(){
  change_to_output(DD);
  return 1;
}

uint8_t read(){
	uint8_t data = 0;
	//change_to_input(DD);
	// Send 8 clock pulses if we are HIGH
	for (uint8_t cnt = 0; cnt<8; cnt++) {
		high(DC);
		delay(3);

		data <<= 1;
		if (GPIOD->IDR & 1<<9)
			data |= 0x01;

		//data |= (uint8_t)((input(DD))>>(2+cnt));
		low(DC);
		delay(3);
	}
	return data;
}

uint8_t updateInstructionTable( uint8_t * newTable ){
  return 1;
}

uint8_t getInstructionTableVersion(){
  return 1;
}

