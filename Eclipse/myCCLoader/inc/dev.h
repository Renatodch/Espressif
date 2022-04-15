/*
 * dev.h
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#ifndef INC_DEV_H_
#define INC_DEV_H_

#define MAX_BUF_SIZE 3333

struct STREAM{
	uint8_t buf[MAX_BUF_SIZE];
	uint32_t i;
	uint32_t tail;
	uint32_t head;
};

// Command constants
#define   CMD_ENTER     (uint8_t)(0x01)
#define   CMD_EXIT      (uint8_t)(0x02)
#define   CMD_CHIP_ID   (uint8_t)(0x03)
#define   CMD_STATUS    (uint8_t)(0x04)
#define   CMD_PC        (uint8_t)(0x05)
#define   CMD_STEP      (uint8_t)(0x06)
#define   CMD_EXEC_1    (uint8_t)(0x07)
#define   CMD_EXEC_2    (uint8_t)(0x08)
#define   CMD_EXEC_3    (uint8_t)(0x09)
#define   CMD_BRUSTWR   (uint8_t)(0x0A)
#define   CMD_RD_CFG    (uint8_t)(0x0B)
#define   CMD_WR_CFG    (uint8_t)(0x0C)
#define   CMD_CHPERASE  (uint8_t)(0x0D)
#define   CMD_RESUME    (uint8_t)(0x0E)
#define   CMD_HALT      (uint8_t)(0x0F)
#define   CMD_PING      (uint8_t)(0xF0)
#define   CMD_INSTR_VER (uint8_t)(0xF1)
#define   CMD_INSTR_UPD (uint8_t)(0xF2)

// Response constants
#define   ANS_OK       (uint8_t)(0x01)
#define   ANS_ERROR    (uint8_t)(0x02)
#define   ANS_READY    (uint8_t)(0x03)

void loop();
void periphInit();

extern struct STREAM stream;
#endif /* INC_DEV_H_ */
