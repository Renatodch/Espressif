/*
 * dev.c
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#include <ccDebugger.h>
#include "stm32f4xx.h"
#include "periph.h"
#include "string.h"
#include "dev.h"

// Initialize properties
uint8_t inByte, bAns, bIdle;
uint8_t c1, c2, c3;
uint16_t s1;
int iLen, iRead;
struct STREAM stream;

void periphInit()
{
	setSysClock();

	portInit();

	/*entramos en debug mode*/
	GPIOF->BSRR = 1<<(LED+0x10);
	delay(224000000);
	debugEnter();
	delay(56000000);
	GPIOF->BSRR = 1<<(LED);

	/*FIFO + UART*/
	stream.head = stream.tail = stream.i = 0;
	memset(stream.buf,0x00,MAX_BUF_SIZE);
	uartConfig();
}

void loop(){

	if(stream.i<4) return;

	/*
	while(stream.i){
		USART1->DR = rcv();
		while(!(USART1->SR & USART_SR_TC));
	}
	*/

	inByte = rcv();
	c1 = rcv();
	c2 = rcv();
	c3 = rcv();

	// Handle commands
	if (inByte== CMD_PING) {
		send( ANS_OK, 0x00, 0x00 );

	} else if (inByte == CMD_ENTER) {
		bAns = debugEnter();
		//if (handleError()) return;
		send( ANS_OK, 0x00, 0x00 );

	} else if (inByte == CMD_EXIT) {
		bAns = debugExit();
		//if (handleError()) return;
		send( ANS_OK, 0x00, 0x00 );

	} else if (inByte == CMD_CHIP_ID) {
		s1 = getChipID();
		//if (handleError()) return;
		send( ANS_OK,
			   s1 & 0xFF,       // LOW first
			   (s1 >> 8) & 0xFF // HIGH second
			  );

	} else if (inByte == CMD_PC) {
		s1 = getPC();
		//if (handleError()) return;
		send( ANS_OK,
				   s1 & 0xFF,       // LOW first
				   (s1 >> 8) & 0xFF // HIGH second
				  );

	} else if (inByte == CMD_STATUS) {
		bAns = getStatus();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_HALT) {
		bAns = halt();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_EXEC_1) {

		bAns = exec1( c1 );
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_EXEC_2) {

		bAns = exec2( c1, c2 );
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_EXEC_3) {
		bAns = exec3( c1, c2, c3 );
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_BRUSTWR) {

		// Calculate the size of the incoming brust
		iLen = (c1 << 8) | c2;

		// Validate length
		if (iLen > 2048) {
		  send( ANS_ERROR, 3, 0x00 );
		  return;
		}

		// Confirm transfer
		send( ANS_READY, 0x00, 0x00 );

		// Prepare for brust-write
		write( 0x80 | (c1 & 0x07) ); // High-order bits
		write( c2 ); // Low-order bits

		//delay(100000);//wait to receicve data from pc

		// Start serial loop
		iRead = iLen;
		bIdle = 0;
		while (iRead > 0) {
		  // When we have data, forward them to the debugger
		  if (stream.i >= 1) {
			inByte = rcv(); //Serial.read();
			write(inByte);
			bIdle = 0;
			iRead--;
		  }

		  // If we don't have any data, check for idle timeout
		  else {
			// If we are idle for more than 1s, drop command
			if (++bIdle > 2000) { //200
			  // The PC was disconnected/stale for too long
			  // complete the command by sending 0's
			  while (iRead > 0) {
				write(0);
				iRead--;
			  }

			  // Read debug status to complete the command sequence
			  switchRead(255);
			  bAns = read();
			  switchWrite();

			  // Send error
			  send( ANS_ERROR, 4, 0x00 );
			  return;
			}

			// Wait for some time
			delay(100); //50

		  }
		}

		// Read debug status
		switchRead(255);
		bAns = read();
		switchWrite();

		// Handle response
		//if (handleError()) return;
		send( ANS_OK, bAns,0x00 );

	} else if (inByte == CMD_RD_CFG) {
		bAns = getConfig();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_WR_CFG) {
		bAns = setConfig(c1);
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_CHPERASE) {
		bAns = chipErase();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_STEP) {
		bAns = step();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_RESUME) {
		bAns = resume();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_INSTR_VER) {
		bAns = getInstructionTableVersion();
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00 );

	} else if (inByte == CMD_INSTR_UPD) {
		// Acknowledge transfer
		send( ANS_READY,0x00,0x00 );

		//delay(2800000);
		// Read 16 bytes from the input
		uint8_t instrBuffer[16];
		iRead = 0;
		while (iRead < 16) {
		  if (stream.i >= 1) {
			instrBuffer[iRead] = rcv();
		  }
		}

		// Update instruction buffer
		bAns = updateInstructionTable( instrBuffer );
		//if (handleError()) return;
		send( ANS_OK, bAns, 0x00);

	} else {
		send( ANS_ERROR, 0xFF, 0x00);

	}

}
