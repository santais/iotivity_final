
#include "sn74hc595.h"

/****************************/
/*    Private Variables     */
/****************************/
static SN74HC595* m_activeSN74HC595 = NULL;


/****************************/
/*    Private Functions     */
/****************************/


/**
 * @brief
 *
 * @param channel			Channel. Range [0,1]
 * @param speed 			Speed of the SPI interface
 * @param numOfRegisters 	Number of shift registers
 *
 * @return fileDescriptor
 */
int SN74HC595Setup(uint8_t channel, SPIClockSpeed speed, uint8_t numOfShiftRegisters)
{
	int clockSpeed = (int) speed;
	SN74HC595Setup1(channel, clockSpeed, numOfShiftRegisters);
}

/**
 * @brief
 *
 * @param channel			Channel. Range [0,1]
 * @param speed 			Speed of the SPI interface
 * @param numOfRegisters 	Number of shift registers
 *
 * @return fileDescriptor
 */
int SN74HC595Setup1(uint8_t channel, int speed, uint8_t numOfShiftRegisters)
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

	m_activeSN74HC595->mode = 0;		// Default mode is 0
	m_activeSN74HC595->channel &= 1; 	// Channel is either 0 or 1
	m_activeSN74HC595->speed = speed;

#ifdef ARM
	if ((m_activeSN74HC595->fileDescriptor = open (channel == 0 ? spiDev0 : spiDev1, O_RDWR)) < 0)
	{
		printf ("Unable to open SPI device: %s\n", strerror (errno));
		return -1;
	}

	// Set SPI parameters.
	if (ioctl (m_activeSN74HC595->fileDescriptor, SPI_IOC_WR_MODE, &m_activeSN74HC595->mode) < 0)
	{
		printf ("SPI Mode Change failure: %s\n", strerror (errno));
		return -1;
	}

	if (ioctl (m_activeSN74HC595->fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
	{
		printf ("SPI BPW Change failure: %s\n", strerror (errno));
		return -1;
	}

	if (ioctl (m_activeSN74HC595->fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
	{
		printf ("SPI Speed Change failure: %s\n", strerror (errno));
		return -1;
	}
#endif

	return m_activeSN74HC595->fileDescriptor;
}

/**
 * @brief
 *
 * @param data		Data to write to the SPI interface
 * @param len		Number of characters to write. Maximum is 4
 *
 * @return fileDescriptor
 */
int SN74HC595ReadWrite(uint8_t *data, uint8_t len)
{
  	struct spi_ioc_transfer spi;

	// Mentioned in spidev.h but not used in the original kernel documentation
	//	test program )-:

	memset (&spi, 0, sizeof (spi));

	spi.tx_buf        = (unsigned long)data;
	spi.rx_buf        = (unsigned long)data;
	spi.len           = (int) len;
	spi.delay_usecs   = spiDelay;
	spi.speed_hz      = m_activeSN74HC595->speed;
	spi.bits_per_word = spiBPW;

	return ioctl (m_activeSN74HC595->fileDescriptor, SPI_IOC_MESSAGE(1), &spi) ;
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
