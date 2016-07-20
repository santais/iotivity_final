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

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define MAX_SHIFT_REGISTERS 4

/** SPI Bus Parameters **/
const static char*       spiDev0  = "/dev/spidev0.0";
const static char*       spiDev1  = "/dev/spidev0.1";
const static uint8_t     spiBPW   = 8 ;
const static uint16_t    spiDelay = 0 ;

typedef enum 
{
	SPI_SPEED_500_KHZ = 500000,
	SPI_SPEED_1_MHZ	  = 1000000,
	SPI_SPEED_2_MHZ	  = 2000000,
	SPI_SPEED_4_MHZ	  = 4000000,
	SPI_SPEED_8_MHZ	  = 8000000,
	SPI_SPEED_16_MHZ  = 16000000,
	SPI_SPEED_32_MHZ  = 32000000
} SPIClockSpeed;

typedef struct 
{
	// Chip select of the chip. Is either 0 or 1
	uint8_t channel;

	// Mode of the SPI Interface. Is by default 0
	uint8_t mode;

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
 * @param channel			Channel. Range [0,1]
 * @param speed 			Speed of the SPI interface
 * @param numOfRegisters 	Number of shift registers
 *
 * @return fileDescriptor
 */
int SN74HC595Setup(uint8_t channel, SPIClockSpeed speed, uint8_t numOfShiftRegisters);

/**
 * @brief
 *
 * @param channel			Channel. Range [0,1]
 * @param speed 			Speed of the SPI interface
 * @param numOfRegisters 	Number of shift registers
 *
 * @return fileDescriptor
 */
int SN74HC595Setup1(uint8_t channel, int speed, uint8_t numOfShiftRegisters);

/**
 * @brief
 *
 * @param data		Data to write to the SPI interface
 * @param len		Number of characters to write. Maximum is 4
 *
 * @return fileDescriptor
 */
int SN74HC595ReadWrite(uint8_t *data, uint8_t len);

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