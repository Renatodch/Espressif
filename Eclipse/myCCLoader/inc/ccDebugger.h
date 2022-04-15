/*
 * CCDebugger.h
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#ifndef INC_CCDEBUGGER_H_
#define INC_CCDEBUGGER_H_

#include "stm32f4xx.h"
#include "stdbool.h"

/**
 * Instruction table indices
 */
#define INSTR_VERSION   0
#define I_HALT          1
#define I_RESUME        2
#define I_RD_CONFIG     3
#define I_WR_CONFIG     4
#define I_DEBUG_INSTR_1 5
#define I_DEBUG_INSTR_2 6
#define I_DEBUG_INSTR_3 7
#define I_GET_CHIP_ID   8
#define I_GET_PC        9
#define I_READ_STATUS   10
#define I_STEP_INSTR    11
#define I_CHIP_ERASE    12

  /**
   * Activate/deactivate debugger
   */
  void setActive( bool on );

  /**
   * Return the error flag
   */
  uint8_t error();

  ////////////////////////////
  // High-Level interaction
  ////////////////////////////

  /**
   * Enter debug mode
   */
  uint8_t debugEnter();

  /**
   * Exit from debug mode
   */
  uint8_t debugExit();

  /**
   * Execute a CPU instructuion
   */
  uint8_t exec1( uint8_t oc0 );
  uint8_t exec2( uint8_t oc0, uint8_t oc1 );
  uint8_t exec3( uint8_t oc0, uint8_t oc1, uint8_t oc2 );
  uint8_t execi( uint8_t oc0, unsigned short c0 );

  /**
   * Return chip ID
   */
  unsigned short getChipID();

  /**
   * Return PC
   */
  unsigned short getPC();

  /**
   * Return debug status
   */
  uint8_t getStatus();

   /**
   * resume program exec
   */
  uint8_t resume();

  /**
   * halt program exec
   */
  uint8_t halt();

  /**
   * Step a single instruction
   */
  uint8_t step();

  /**
   * Get debug configuration
   */
  uint8_t getConfig();

  /**
   * Set debug configuration
   */
  uint8_t setConfig( uint8_t config );

  /**
   * Massive erasure on the chip
   */
  uint8_t chipErase();

  ////////////////////////////
  // Low-level interaction
  ////////////////////////////

  /**
   * Write to the debugger
   */
  uint8_t write( uint8_t data );

  /**
   * Wait until we are ready to read & Switch to read mode
   */
  uint8_t switchRead( uint8_t maxWaitCycles);

  /**
   * Switch to write mode
   */
  uint8_t switchWrite();

  /**
   * Read from the debugger
   */
  uint8_t read();

  /**
   * Update the debug instruction table
   */
  uint8_t updateInstructionTable( uint8_t *newTable ); //size 16

  /**
   * Get the instruction table version
   */
  uint8_t getInstructionTableVersion();



#endif /* INC_CCDEBUGGER_H_ */
