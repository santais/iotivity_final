
#include "sn74hc595.h"

#define SHIFT_REGISTER_BIT_SIZE 8

#define SECONDS_TO_MICROSECONDS_MULTIPLIER 1000000
#define DEFAULT_FREQUENCY_HZ            100000
#define DEFAULT_FREQUENCY_PERIOD_MUS    (1/DEFAULT_FREQUENCY_HZ) * SECONDS_TO_MICROSECONDS_MULTIPLIER
/****************************/
/*    Private Variables     */
/****************************/
static SN74HC595* m_activeSN74HC595 = NULL;


/****************************/
/*    Private Functions     */
/****************************/


void pulsePin(uint8_t pin)
{
#ifdef ARM
	digitalWrite(pin, 0);
	usleep(DEFAULT_FREQUENCY_PERIOD_MUS);
	digitalWrite(pin, 1);
	usleep(DEFAULT_FREQUENCY_PERIOD_MUS);
#endif
}


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
                   const uint8_t dataPin, const uint8_t numOfShiftRegisters)
{
	if(m_activeSN74HC595 == NULL)
	{
		m_activeSN74HC595 = (SN74HC595*) malloc(sizeof(SN74HC595));
		if(m_activeSN74HC595 == NULL)
		{
			printf ("Not enough memory to initiate SN74HC595 structure: %s\n", strerror (errno));
			return -1;
		}
	}

	m_activeSN74HC595->clkPin   		   = clkPin;
	m_activeSN74HC595->clkEnPin 		   = clkEnPin;
	m_activeSN74HC595->dataPin  		   = dataPin;
	m_activeSN74HC595->numOfShiftRegisters = numOfShiftRegisters;

#ifdef ARM

	
	pinMode(clkPin, OUTPUT);
	pinMode(clkEnPin, OUTPUT);
	pinMode(dataPin, OUTPUT);

	// Set initial values
	digitalWrite(clkPin, LOW);
	digitalWrite(clkEnPin, LOW);
	digitalWrite(dataPin, LOW);

#endif

	return 1;

}

/**
 * @brief Read the data from the 165. The number of bytes returned depends on numOfShiftRegisters registered
 *
 * @return Read Data
 */
int SN74HC595Write(uint8_t* data)
{
    if(m_activeSN74HC595 == NULL)
    {
        printf ("SN74HC595 not setup. Call SN74HC165Setup first %s\n", strerror (errno));
        return -1;
    }
#ifdef ARM
	
    printf("Number of shift registers: %i\n", m_activeSN74HC595->numOfShiftRegisters);
    for(size_t i = 0; i < m_activeSN74HC595->numOfShiftRegisters; i++)
    {
	printf("Writing data: %#08x\n", data[i]);
    	for(int bits = SHIFT_REGISTER_BIT_SIZE - 1; bits >= 0; bits--)
    	{
    		digitalWrite(m_activeSN74HC595->dataPin, data[i] & (1 << bits));
		printf("Bit is: %i\n", data[i] & (1 << bits));
    		pulsePin(m_activeSN74HC595->clkPin);
    	}
    }

    // Set the latch pin high to store the data
    pulsePin(m_activeSN74HC595->clkEnPin);

#endif
    return 1;
}

/**
 * @brief Returns the SN74HC595 structure
 *
 * @return 
 */
SN74HC595* getSN74HC595Struct()
{
	return m_activeSN74HC595;
}
