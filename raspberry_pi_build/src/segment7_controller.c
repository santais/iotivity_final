#include "segment7_controller.h"


// Bit Mask
//static int INPUT_BITS_MASK = 0;

/*********************/
/* Private Variables */
/*********************/

// Segment7 structure linked list
static Segment7* m_segment7 = NULL;

// Callback
static SegmentValueCallback m_callback;

// Boolean to determine if the segment 7 thread is running
static uint8_t m_isRunning = 0;

// Thread to be run.
static pthread_t m_pThread;

// SN75HC165 structure
static SN74HC165* m_SN74HC165;

// SN75HC595 structure
static SN74HC595* m_SN74HC595;

// Frequency
static unsigned long m_freq;

// Number of segments
static uint8_t m_numOfSegmentPairs = 0;

// Mutex
pthread_mutex_t m_mutex;

// Test Data. DEBUG
uint8_t m_testData[4] = {};


// 7 Segment Display List
const static uint8_t m_segmentLookupTable[10] = {
    0x40,
    0xF9,
    0x24,
    0x30,
    0x19,
    0x12,
    0x02,
    0x78,
    0x00,
    0x10
};


/*********************/
/* Private Funcitons */
/*********************/

/**
 * @brief Hamming Weight bit counter. Counts the number of bits
 *	      in a data string and returns the number of '1's in the
 *	      string
 * 
 * @param x 	Input data string 
 *
 * @return Number of '1' bits
 */
uint8_t hammingWeightBitCount(unsigned x) {
   x = x - ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x + (x >> 4)) & 0x0f0f0f0f;
   x = x + (x >> 8);
   x = x + (x >> 16);
   return x & 0x0000003f;
}

/**
 * @brief Segments the data into the individually segments.
 *		  I.e. will an input of 10 be divided into 1 and 0.
 *
 * @param dataInput 	Input digit
 */
uint8_t* fragmentSegmentDigits(uint8_t dataInput)
{
	// Size is 2 because the segments are in pairs
	static uint8_t dataOutput[2] = {};

	dataOutput[0] = m_segmentLookupTable[dataInput / 10];
	dataOutput[1] = m_segmentLookupTable[dataInput % 10];

	printf("dataOutput: %i %i \n", dataOutput[0], dataOutput[1]);

	return dataOutput;
}

/**
 * @brief Transmits the data to the SN74HC595 Driver
 *
 * @param data 		Data to be transmitted
 */
void sendDataToSegment(uint8_t* dataInput, uint8_t len)
{
	// Split the data into 2 bytes (for 10 digit segments).
	uint8_t* dataOutput = (uint8_t*) malloc(sizeof(uint8_t) * (len * 2));

	for(size_t i = 0; i < len; i++)
	{
		uint8_t* tempData = fragmentSegmentDigits(dataInput[i]);

		dataOutput[i * 2] = tempData[0];
		dataOutput[i * 2 + 1] = tempData[1];
	}

	// Send the data to the SN74HC595 driver
	printf("Return value of SPI Read is: %i\n", SN74HC595Write(dataOutput));
}

/**
 * @brief Thread that continuously checks inputs
 *		  from the SN74HC165 iinput
 */
void* inputThread()
{
	static uint8_t* dataInput = NULL;
	static uint8_t* dataOutput = NULL;
	static uint8_t bitCount = 0;
	static Segment7* current = NULL;

	// Used to announce when a change in the value occured
	static uint8_t newInputVal = 0;

	// Initialize the dataOutput
	dataOutput = (uint8_t*) malloc(sizeof(uint8_t) * m_numOfSegmentPairs);
	memset(dataOutput, 0x00, sizeof(uint8_t) * m_numOfSegmentPairs);

	while(m_isRunning)
	{	
		// Reset inputval
		newInputVal = 0;

		// Read the current value
		//dataInput =  SN74HC165Read();
		pthread_mutex_lock(&m_mutex);	// DEBUG

		// Count the number of 1 bits and compare
		// them with the current value.
		// Iterate through the 7segment structures
		current = m_segment7;
		while(current != NULL)
		{
			bitCount = hammingWeightBitCount(m_testData[current->shiftRegisterID1]) + 
				hammingWeightBitCount(m_testData[current->shiftRegisterID2] & INPUT_BITS_MASK);

			if(bitCount != current->value)
			{
				current->value = bitCount;

				// Announce the program which id changed
				m_callback(current);

				// Announce new value
				newInputVal = 1;
			}

			// Used to segments ID to store the value
			if(current->id < sizeof(dataOutput)) 
			{
				dataOutput[current->id] = current->value;
			}

			current = current->next;
		}

		// If the value changed
		if(newInputVal > 0)
		{
			// Send the new data to the SN74HC595 display
			sendDataToSegment(dataOutput, m_numOfSegmentPairs);
		}

		pthread_mutex_unlock(&m_mutex);	 // DEBUG
		usleep(m_freq);
	}

	printf("Terminating thread\n");
}

/**
 * @brief Initializes the segment7 structur
 *
 * @param numOfSegmentPairs number of 7 Segment pairs
 */
int8_t initializeSegmentStruct(uint8_t numOfSegmentPairs)
{
	// Initialize the list
	if(m_segment7 == NULL)
	{
		if( NULL == (m_segment7 = malloc(sizeof(Segment7) * numOfSegmentPairs)))
		{
			printf("Not enough memory\n");
			return -1;
		}

		// Clear all variables
		memset(m_segment7, 0x00, sizeof(Segment7) * numOfSegmentPairs);

		// Set the next and id fields
		uint8_t i = 0;
		for(i = 0; i < numOfSegmentPairs - 1; i++)
		{
			m_segment7[i].next = &m_segment7[i+1];
			m_segment7[i].shiftRegisterID1  = i*2;
			m_segment7[i].shiftRegisterID2  = i*2 + 1;
			m_segment7[i].id = i;
			m_segment7[i].value = 0;
		}

		// Set last elements
		m_segment7[i].next = NULL;
		m_segment7[i].shiftRegisterID1  = i*2;
		m_segment7[i].shiftRegisterID2  = i*2 + 1;
		m_segment7[i].id = i;
		m_segment7[i].value = 0;
	}

	return 1;
}


/**
 * @brief Setup tghe 7segment controller
 *
 * @param numOfSegmentPairs		Number of 2x1 7 segments in the system
 * @param cb 					Callback
 *
 * @return 1 if success. -1 otherwise
 */
int segment7Setup(uint8_t numOfSegmentPairs, SegmentValueCallback cb)
{
	// Overwrite the current callback
	if(cb == NULL)
	{
		printf("Callback is null!\n");
		return -1;	
	}

	// Overwrite current callback
	m_callback = cb;
	m_numOfSegmentPairs = numOfSegmentPairs;

	// Set the frequency
	float freq = (1/(float)DEFAULT_FREQUENCY_HZ) * (float)HZ_TO_MICROSECONDS_MULTIPLIER;
	m_freq = (unsigned long) freq;

	// Initialize the segmen7 struct
	if(initializeSegmentStruct(numOfSegmentPairs) <0 )
	{
		return -1;	
	}

	// Setup the SN74HC165 modules
	if(SN74HC165Setup(SN74HC165_CLOCK_PIN, SN74HC165_CLOCK_EN_PIN, SN74HC165_LATCH_PIN,
		SN74HC165_DATA_PIN, numOfSegmentPairs * 2) < 0)
	{
		printf("Unable to setup SN74HC165\n");
		return -1;
	}


	if(SN74HC595Setup(SN74HC595_CLOCK_PIN, SN74HC595_CLOCK_EN_PIN, SN74HC595_DATA_PIN,
		 numOfSegmentPairs * 2) < 0)
	{
		printf("Unable to setup SN74HC595\n");
		return -1;
	}

	return 1;
}

/**
 * @brief set the requency
 */
void setFrequencyHz(int frequency)
{
	if(frequency < 1000)
	{
		printf("FREQUENCY IS TOO HIGH! MAX IS 1000 HZ\n");
	}
	else
	{
		m_freq = frequency;
	}
}

/**
 * @brief Returns the segment7 linked list
 */
Segment7* getSegment7List()
{
	return m_segment7;
}

/**
 * @brief get a copy of the 74hc195 object
 */
SN74HC165 get74HC165Obj()
{
	return *m_SN74HC165;
}

/**
 * @brief get a copy of the 74hc595 object
 */
SN74HC595 get74HC595Obj()
{
	return *m_SN74HC595;
}

/**
 * @brief Starts the segment7 input listening thread
 */
void startSegment7()
{
	int resultCode = pthread_create(&m_pThread, NULL, inputThread, NULL);

	if(resultCode != 0)
	{
		printf("Create pthread failed with result code: %i\n", resultCode);
		return;
	}

	// Detach thread from main thread
	resultCode = pthread_detach(m_pThread);	

	if(resultCode != 0)
	{
		printf("Unable to deatch thread. Result code: %i\n", resultCode);
		return;
	}

	m_isRunning = 1;
}

/**
 * @brief Stops the segment7 input listening thread
 */
void stopSegment7()
{
	m_isRunning = 0;
}

/**
 * @brief Get the running status of the thread
 */
uint8_t getRunningStatus()
{
	return m_isRunning;
}

void setTestData(uint8_t data[4])
{
	pthread_mutex_lock(&m_mutex);
	m_testData[0] = data[0];
	m_testData[1] = data[1];
	m_testData[2] = data[2];
	m_testData[3] = data[3];
	pthread_mutex_unlock(&m_mutex);
}
