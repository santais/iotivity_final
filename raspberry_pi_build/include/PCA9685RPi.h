/*************************************************/
/* Copyright Mark Povlsen						 */
/* Description:..								 */
/*												 */
/*												 */
/*												 */
/*												 */
/*												 */
/*************************************************/
#ifndef _PCA9685RPI_H_
#define _PCA9685RPI_H_

#ifdef __cplusplus
    extern "C" {
#endif


//#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#ifdef RPI
#include <wiringPi.h>
#include <wiringPiI2C.h>
#endif

/******* Register Addresses ********/
#define MODE1           0x00
#define MODE2           0x01

#define SUBADR1         0x02
#define SUBADR2         0x03
#define SBUADR3         0x04
#define ALLCALLADR      0x05

#define LED0_ON_L       0x06

#define ALL_LED_ON_L    0xFA
#define ALL_LED_ON_H    0xFB
#define ALL_LED_OFF_L   0xFC
#define ALL_LED_OFF_H   0xFD

#define PRE_SCALE       0xFE
#define CLOCK_FREQ      25000000
#define FREQ_RESOLUTION 4096

#define TEST_MODE       0xFF

/******* Register Bit Masks ********/
#define MODE1_AR_MASK           0x20
#define MODE1_SLEEP_MASK        0x10
#define MODE1_WAKE_MASK         0xEF
#define MODE1_RESTART_MASK      0x80
#define MODE1_SETUP_MASK        0x7F

#define LED_FULL_ON_OFF_MASK    0x10
#define LED_FULL_NEGATED        0xEF

#define LED_H_SHIFT_MASK        8
#define LED_L_MASK              0xFF
#define LED_NEXT_MASK           4

/******** PCA Structures  *********/

typedef struct
{
    // Associated Output pin on the RPI
    uint8_t pin;

    // 4096 ON Value
    uint16_t value;
} PinOutput;

typedef struct PinOutputList
{
    struct PinOutputList* next;

    PinOutput pinOutput;
} PinOutputList;

/**
  * Linked list of all RGB LED ouptuts
  */
typedef struct RBGLEDOutput
{
    // Linked List. Next type
    struct RGBLEDOutput *next;

    // Red LED
    PinOutput red;

    // Green LED
    PinOutput green;

    // Blue LED
    PinOutput blue;

} RGBLEDOutput_t;

/**
  * Linked list of all PCA9685
  */
typedef struct PCA9685
{
    // Linked list. Next In list
    struct PCA9685* next;

    // I2C Address of the PCA9685 module
    uint8_t i2cAddress;

    // File Descriptor of the open device on Linux
    int fileDescriptor;

    // Frequency of the given PCA9685
    uint16_t frequency;

    // Attached RGBLED outputs.
    PinOutputList* pinOutputList;

} PCA9685;


/**
 * @brief Setup the PCA9865
 *
 * @param i2cAddress    Hex Address of te I2C connection
 * @param frequency     Frequency of the PCA9865
 *
 * @return fileDescriptor
 */
int PCA9685Setup(const uint8_t i2cAddress, uint16_t freq);

/**
 * @brief Reset PCA9685 to default settings.
 */
void PCA9685Reset();

/**
 * @brief Set the frequency of the PCA9685
 *
 * @param freq  Frequency to be set. Range 24 - 1526 Hz
 *
 * @return Prescale value
 */
uint16_t PCA9685SetFreq(uint16_t freq);

/**
 * @brief Set the PWM Duty Cycle of a specific pin
 *
 * @param LEDPin        Pin to set the Duty Cycle on. Range 0 - 15
 * @param duty_cycle    Duty cycle. Range 0 - 100 %
 */
void PCA9685SetPWMDC(uint8_t LEDPin, uint8_t duty_cycle);

/**
 * @brief Set the PWM of a specific pin
 *
 * @param LEDPin        Pin to set the pwm for. Range 0 - 15
 * @param onTime        On time of the LED
 * @param offTime       Off time of the LED
 */
void PCA9685SetPWM(uint8_t LEDPin, uint16_t onTime, uint16_t offTime);

/**
 * @brief Retrieve the PWM for a specific pin
 *
 * @param LEDPin        Pin to get the PWM signal from. Range 0 - 15
 *
 * @return
 */
uint16_t PCA9685GetPWM(uint8_t LEDPin);

/**
 * @brief Turn LED pin fully on
 *
 * @param LEDPin        Pin to turn all on. Range 0 - 15
 */
void PCA9685LEDOn(uint8_t LEDPin);

/**
 * @brief Turn LED pin fully off
 *
 * @param LEDPin        Pin to tunr all off. Range 0 - 15
 */
void PCA9685LEDOff(uint8_t LEDPin);

/**
 * @brief Turns on all 16 LEDs output to max 100% duty cycle
 */
void PCA9685AllLEDsOn();

/**
 * @brief Turn off all 16 LEDs out to min 0% duty cycle.
 */
void PCA9685AllLEDsOff();

/**
 * @brief Get the active PCA9685 module;
 *
 * @return
 */
PCA9685* getActivePCA9685Struct();
  
#ifdef __cplusplus
}
#endif

#endif /* _PCA9685RPI_H_ */
