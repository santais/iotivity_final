#include "PCA9685RPi.h"

/****************************/
/*    Private Variables     */
/****************************/
static PCA9685* m_activePCA9685 = NULL;


/****************************/
/*    Private Functions     */
/****************************/

/**
 * @brief Check if the pin has been initialized
 *
 * @param pin       Pin to check
 *
 * @return found pin
 */
PinOutput* findPin(PinOutput **head, uint8_t pin)
{
    if(m_activePCA9685 != NULL)
    {
        PinOutputList* current = *head;

        while(current != NULL)
        {
            if(current->pinOutput.pin == pin)
            {
                //printf("Found pin: %i\n", current->pinOutput.pin);
                return &current->pinOutput;
            }
            current = current->next;
        }
    }
    else
    {
        return NULL;
    }

    return NULL;
}

/**
 * @brief insertNewPin
 *
 * @param pin
 *
 * @param value
 */
void insertNewPin(PinOutputList **head, uint8_t pin, uint16_t value)
{
    PinOutputList* newNode = (PinOutputList*) malloc(sizeof(PinOutputList));
    PinOutputList* last = *head;

    if(!newNode)
    {
        printf("Failed to allocate memory for new Pin Node\n");
    }
    else
    {
        // Insert the data
        newNode->next = NULL;
        newNode->pinOutput.pin = pin;
        newNode->pinOutput.value = value;

        if(*head == NULL)
        {
            *head = newNode;
            return;
        }

        while(last->next != NULL)
        {
            last = last->next;
        }

        last->next = newNode;
    }
}


void insertNewPinValue(PinOutput **head, uint8_t LEDPin, uint16_t onTime)
{
    // Insert the new value into the current PCA9686;
    PinOutput* pin = findPin(&m_activePCA9685->pinOutputList, LEDPin);
    if(pin == NULL)
    {
        insertNewPin(&m_activePCA9685->pinOutputList, LEDPin, onTime);
    }
    else
    {
        pin->value = onTime;
    }
}

/**
 * @brief Setup the PCA9865
 *
 * @param i2cAddress    Hex Address of te I2C connection
 * @param frequency     Frequency of the PCA9865
 *
 * @return fileDescriptor
 */
int PCA9685Setup(const uint8_t i2cAddress, uint16_t freq)
{
    // Setup the PCA9685 struct if its null
    if(m_activePCA9685 == NULL)
    {
        // Assign memory to the structure
        m_activePCA9685 = malloc(sizeof(PCA9685));
        m_activePCA9685->next = NULL;
        m_activePCA9685->pinOutputList = NULL;
    }

    m_activePCA9685->frequency = freq;
    m_activePCA9685->i2cAddress = i2cAddress;
    m_activePCA9685->pinOutputList = NULL;

    // Setup the I2C Port
#ifdef ARM
    m_activePCA9685->fileDescriptor = wiringPiI2CSetup(m_activePCA9685->i2cAddress);
    // Read current settings and clear restart bit
    int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, MODE1) & MODE1_SETUP_MASK;
    // Enable auto increment
    settings |= MODE1_AR_MASK;

    // Write to the register
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, settings);

    // Settup the frequency
    PCA9685SetFreq(m_activePCA9685->frequency);
#endif
    
    return m_activePCA9685->fileDescriptor;
}

/**
 * @brief Reset PCA9685 to default settings.
 */
void PCA9685Reset()
{
    // Restart the PCA9685
#ifdef ARM
    int mode1 = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, MODE1) & MODE1_RESTART_MASK;

    if(mode1)
    {
        int settings = mode1 & MODE1_WAKE_MASK;      // Clear SLEEP bit
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, settings);
        usleep(500);                                // Sleep for 500 microseconds
        settings |= MODE1_RESTART_MASK;
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, settings);
    }
#endif
}

/**
 * @brief Set the frequency of the PCA9685
 *
 * @param freq  Frequency to be set. Range 24 - 1526 Hz
 */
uint16_t PCA9685SetFreq(uint16_t freq)
{
    if(freq > FREQ_RESOLUTION)
    {
        m_activePCA9685->frequency = FREQ_RESOLUTION;
    }
    else
    {
        m_activePCA9685->frequency = freq;

    }

    uint16_t prescale = round(CLOCK_FREQ / (FREQ_RESOLUTION * m_activePCA9685->frequency )) - 1;

#ifdef ARM
    // Set the settings byte
    int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, MODE1) & MODE1_SETUP_MASK;
    int sleep = settings | MODE1_SLEEP_MASK;        // Set bit 5 SLEEP
    int wake  = settings & MODE1_WAKE_MASK;         // Clear bit 5 SLEEP
    int restart = settings | MODE1_RESTART_MASK;    // Set bit 7 RESTART

    // Set device to sleep
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, sleep);
    // Set PWM
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, PRE_SCALE, prescale);
    // Wake up the PCA9685
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, wake);

    // Wait 1ms to for oscillator to stabilize and then restart it
    usleep(500);
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, MODE1, restart);
#endif
    return prescale;
}

/**
 * @brief Set the PWM Duty Cycle of a specific pin
 *
 * @param LEDPin        Pin to set the Duty Cycle on. Range 0 - 15
 * @param duty_cycle    Duty cycle. Range 0 - 100 %
 */
void PCA9685SetPWMDC(uint8_t LEDPin, uint8_t dutyCycle)
{
    printf("Current I2C Address is: %i\n", m_activePCA9685->i2cAddress);
    // Calculate max and minimum duty cycle
    uint8_t m_dutyCycle = dutyCycle;
    if(dutyCycle > 100)
    {
        m_dutyCycle = 100;
    }

    printf("Pin %i and duty cycle %i\n", LEDPin, dutyCycle);

    // Get lower and upper values
    uint16_t onTime  = (FREQ_RESOLUTION * dutyCycle) / 100;
    uint16_t offTime = FREQ_RESOLUTION - onTime;

    PCA9685SetPWM(LEDPin, onTime, offTime);
}

/**
 * @brief Set the PWM of a specific pin
 *
 * @param LEDPin        Pin to set the pwm for. Range 0 - 15
 * @param onTime        On time of the LED
 * @param offTime       Off time of the LED
 */
void PCA9685SetPWM(uint8_t LEDPin, uint16_t onTime, uint16_t offTime)
{
    if(LEDPin < 16)
    {
        uint8_t LEDRegister = LED0_ON_L + (LED_NEXT_MASK * LEDPin);
#ifdef ARM
        // Calculate the on and off time and write to the registers
        uint8_t LEDRegisterVal = onTime & LED_L_MASK;               // LED_ON_L
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister, LEDRegisterVal);
        LEDRegisterVal = onTime >> LED_H_SHIFT_MASK;                // LED_ON_H
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 1, LEDRegisterVal);
        LEDRegisterVal = offTime & LED_L_MASK;                      // LED_OFF_L
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 2, LEDRegisterVal);
        LEDRegisterVal = offTime & LED_H_SHIFT_MASK;                // LED_OFF_H
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 3, LEDRegisterVal);

        // DEBUG
        printf("Pin %i with onTime %i and offTime %i\n", LEDPin, onTime, offTime);
#endif

        // Insert the new value into the current PCA9686;
        insertNewPinValue(&m_activePCA9685->pinOutputList, LEDPin, onTime);
    }
    else
    {
        printf("ERROR. LEDPin is above maximum of 15\n");
    }
}

/**
 * @brief Retrieve the PWM for a specific pin
 *
 * @param LEDPin        Pin to get the PWM signal from. Range 0 - 15
 *
 * @return
 */
uint16_t PCA9685GetPWM(uint8_t LEDPin)
{
    uint16_t pwmValue = 0;
    if(LEDPin < 16)
    {
        uint8_t LEDRegister = LED0_ON_L + (LED_NEXT_MASK * LEDPin);
#ifdef ARM
        // Frst retriev LED_ON_H
        uint16_t registerValue = wiringPiI2CReadReg16(m_activePCA9685->fileDescriptor, LEDRegister + 1);
        pwmValue = registerValue & 0xF;         // Only get the 3. byte
        pwmValue <<= LED_H_SHIFT_MASK;          // Shift 8 bytes to make room for LED_ON_L

        // Retrieve LED_ON_L
        registerValue = wiringPiI2CReadReg16(m_activePCA9685->fileDescriptor, LEDRegister);
        pwmValue |= registerValue;
#endif
    }
    else
    {
        printf("ERROR. LEDPin is above maximum of 15\n");
    }
    return pwmValue;
}

/**
 * @brief Turn LED pin fully on
 *
 * @param LEDPin        Pin to turn all on. Range 0 - 15
 */
void PCA9685LEDOn(uint8_t LEDPin)
{
    if(LEDPin < 16)
    {
        uint8_t LEDRegister = LED0_ON_L + (LED_NEXT_MASK * LEDPin);
#ifdef ARM
        // Read current settings from LED_ON_H
        int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, LEDRegister + 1);
        settings |= LED_FULL_ON_OFF_MASK;       // Set full ON mask

        // Write to register
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 1, settings);

        // Ensure that the LED_OFF_H bit 4 is set low aswell
        settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, LEDRegister + 3);
        settings &= LED_FULL_NEGATED;
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 3, settings);
#endif

        // Insert the new value into the current PCA9686;
        insertNewPinValue(&m_activePCA9685->pinOutputList, LEDPin, FREQ_RESOLUTION);
    }
    else
    {
        printf("ERROR. LEDPin is above maximum of 15\n");
    }
}

/**
 * @brief Turn LED pin fully off
 *
 * @param LEDPin        Pin to tunr all off. Range 0 - 15
 */
void PCA9685LEDOff(uint8_t LEDPin)
{
    if(LEDPin < 16)
    {
        uint8_t LEDRegister = LED0_ON_L + (LED_NEXT_MASK * LEDPin);
#ifdef ARM
        // Read current settings from LED_OFF_H
        int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, LEDRegister + 3);
        settings |= LED_FULL_ON_OFF_MASK;

        // Write to register
        wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, LEDRegister + 3, settings);
#endif

        // Insert the new value into the current PCA9686;
        insertNewPinValue(&m_activePCA9685->pinOutputList, LEDPin, 0);
    }
    else
    {
        printf("ERROR. LEDPin is above maximum of 15\n");
    }
}

/**
 * @brief Turns on all 16 LEDs output to max 100% duty cycle
 */
void PCA9685AllLEDsOn()
{
#ifdef ARM
    // Read current settings
    int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, ALL_LED_ON_H);
    settings |= LED_FULL_ON_OFF_MASK;

    // Write to register
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, ALL_LED_ON_H, settings);

    // Ensure that the LED_OFF_H bit 4 is set low aswell
    settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, ALL_LED_OFF_H);
    settings &= LED_FULL_NEGATED;
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, ALL_LED_OFF_H, settings);
#endif

    // Iterate through all pin values and set them high
    PinOutputList* head = m_activePCA9685->pinOutputList;
    while(head != NULL)
    {
        head->pinOutput.value = FREQ_RESOLUTION;
        head = head->next;
    }
}

/**
 * @brief Turn off all 16 LEDs out to min 0% duty cycle.
 */
void PCA9685AllLEDsOff()
{
#ifdef ARM
    // Read current settings
    int settings = wiringPiI2CReadReg8(m_activePCA9685->fileDescriptor, ALL_LED_OFF_H);
    settings |= LED_FULL_ON_OFF_MASK;

    // Write to register
    wiringPiI2CWriteReg8(m_activePCA9685->fileDescriptor, ALL_LED_OFF_H, settings);
#endif

    // Iterate through all pin values and set them high
    PinOutputList* head = m_activePCA9685->pinOutputList;
    while(head != NULL)
    {
        head->pinOutput.value = 0;
        head = head->next;
    }
}

/**
 * @brief Get the active PCA9685 module;
 *
 * @return
 */
PCA9685* getActivePCA9685Struct()
{
    return m_activePCA9685;
}



