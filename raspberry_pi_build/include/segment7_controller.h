/*************************************************/
/* Copyright Mark Povlsen						 */
/* Description:..								 */
/*												 */
/*												 */
/*												 */
/*												 */
/*												 */
/*************************************************/
#ifndef _SEGMENT7_CONTROLLER_H_
#define _SEGMENT7_CONTROLLER_H_

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "sn74hc165.h"
#include "sn74hc595.h"

// Number of input Beer Pong Cups on each side
#define NUMBER_OF_ACTIVE_INPUTS 10

// 10 cups equals only 10 bits from the shift register
// is to be read
#define INPUT_BITS_MASK 0x03

// Static pin numbers. WiringPi configurations
#define SN74HC165_CLOCK_PIN		0
#define SN74HC165_CLOCK_EN_PIN 	        2
#define SN74HC165_LATCH_PIN		3
#define SN74HC165_DATA_PIN		12

#define SN74HC595_CLOCK_PIN		23
#define SN74HC595_CLOCK_EN_PIN		24
#define SN74HC595_DATA_PIN		25

#define HZ_TO_NANOSECONDS_MULTIPLIER 1000000000
#define HZ_TO_MICROSECONDS_MULTIPLIER 1000000
#define DEFAULT_FREQUENCY_HZ		 1

// Clock speed of 

typedef struct Segment7 {

	// Linked List. Next in the list
	struct Segment7* next;

	// ID of the segment7
	uint8_t id;

	// ID of the first SN74HC165 module
	uint8_t shiftRegisterID1;

	// ID of the second SN74HC165 module
	uint8_t shiftRegisterID2;

	// Current value of the 2x1 7-segment display
	uint8_t value;

} Segment7;

/**
 * @brief Callback definition called whenever a change in the
 *		  input is read and a new value of 7 segment is calculated
 */
typedef void (*SegmentValueCallback)(Segment7* segment7);

/**
 * @brief Setup tghe 7segment controller
 *
 * @param numOfSegmentPairs		Number of 2x1 7 segments in the system
 * @param cb 					Callback
 *
 * @return 1 if success. -1 otherwise
 */
int segment7Setup(uint8_t numOfSegmentPairs, SegmentValueCallback cb);

/**
 * @brief set the requency
 */
void setFrequencyHz(int frequency);

/**
 * @brief Returns the segment7 linked list
 */
Segment7* getSegment7List();

/**
 * @brief get a copy of the 74hc195 object
 */
SN74HC165 get74HC165Obj();

/**
 * @brief get a copy of the 74hc595 object
 */
SN74HC595 get74HC595Obj();

/**
 * @brief Starts the segment7 input listening thread
 */
void startSegment7();

/**
 * @brief Stops the segment7 input listening thread
 */
void stopSegment7();

/**
 * @brief Get the running status of the thread
 */
uint8_t getRunningStatus();

void setTestData(uint8_t data[4]);

#ifdef __cplusplus
}
#endif



#endif /* _SEGMENT7_CONTROLLER_H_ */
