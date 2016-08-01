/*************************************************/
/* Copyright Mark Povlsen						 */
/* Description:..								 */
/*												 */
/*												 */
/*												 */
/*												 */
/*												 */
/*************************************************/
#ifndef _SN74HC595_H_
#define _SN74HC595_H_

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef ARM
#include "wiringPi.h"
#endif

#define MAX_SHIFT_REGISTERS 4

typedef struct 
{
	// CLK - SRCLK
	uint8_t clkPin;

	// CE - RCLK
	uint8_t clkEnPin;

	// Data (MOSI) - SER
	uint8_t dataPin;

	// Number of registered shift registers
	uint8_t numOfShiftRegisters;

	// Speed of the sPI
	int speed;

	// FileDescriptor
	int fileDescriptor;
} SN74HC595;

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
int SN74HC595Setup(const uint8_t clkPin, const uint8_t clkEnPin,
                   const uint8_t dataPin, const uint8_t numOfShiftRegisters);

/**
 * @brief Read the data from the 165. The number of bytes returned depends on numOfShiftRegisters registered
 *
 * @return Read Data
 */
int SN74HC595Write(uint8_t* data);


/**
 * @brief Returns the SN74HC595 structure
 *
 * @return 
 */
SN74HC595* getSN74HC595Struct();

#ifdef __cplusplus
}
#endif

#endif /* _SN74HC595_H_ */