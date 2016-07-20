/*************************************************/
/* Copyright Mark Povlsen						 */
/* Description:..								 */
/*												 */
/*												 */
/*												 */
/*												 */
/*												 */
/*************************************************/
#ifndef _SN74HC165_H_
#define _SN74HC165_H_

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef ARM
#include "wiringPi.h"
#endif

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct 
{
	// CLK Pin
    uint8_t clkPin;
    
    // CLK INH Pin
    uint8_t clkEnPin;
    
    // SH/LD Pin
    uint8_t latchPin;
    
    // Data Pin
    uint8_t dataPin;
    
	// Number of registered shift registers
	uint8_t numOfShiftRegisters;

	// FileDescriptor
	int fileDescriptor;
} SN74HC165;

/**
 * @brief
 *
 * @param clkPin                CLK Pin
 * @param sclkEnPin             CLK INH Pin
 * @param latchPin              SH/LD Pin
 * @param dataPin               Data Pin
 * @param numOfShiftRegister    Number of shift registers
 *
 * @return fileDescriptor
 */
int SN74HC165Setup(const uint8_t clkPin, const uint8_t clkEnPin,
                   const uint8_t latchPin, const uint8_t dataPin, const uint8_t numOfShiftRegisters);

/**
 * @brief Read the data from the 165. The number of bytes returned depends on numOfShiftRegisters registered
 *
 * @return Read Data
 */
uint8_t* SN74HC165Read();

/**
 * @brief Returns the SN75HC595 structure
 *
 * @return 
 */
SN74HC165 getSN74HC165fStruct();

/**
 * @brief Returns the active frequency
 */
int getFrequency();

#ifdef __cplusplus
}
#endif

#endif /* _SN74HC165_H_ */