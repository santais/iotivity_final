#include "PCA9685LEDResource.h"
#include "resource_types.h"

PCA9685LEDResource::PCA9685LEDResource() :
    m_uri("/pca9685led/0"),
    m_I2CAddress(64),
    m_frequency(500)
{}

/**
 * @brief PCA9685LEDResource::PCA9685LEDResource
 *
 * @param portPin
 * @param uri       Uri of the new PCA9685LED resource
 */
PCA9685LEDResource::PCA9685LEDResource(const std::string &uri, int i2cAddress, int frequnecy) :
    m_uri(uri),
    m_I2CAddress(i2cAddress),
    m_frequency(frequnecy)
{}

PCA9685LEDResource::~PCA9685LEDResource()
{
    m_resource.reset();
}

PCA9685LEDResource::PCA9685LEDResource(const PCA9685LEDResource& PCA9685LED) :
    m_resource(PCA9685LED.m_resource),
    m_uri(PCA9685LED.m_uri)
{}

PCA9685LEDResource::PCA9685LEDResource(PCA9685LEDResource&& PCA9685LED) :
    m_resource(std::move(PCA9685LED.m_resource)),
    m_uri(std::move(PCA9685LED.m_uri))
{}

PCA9685LEDResource& PCA9685LEDResource::operator=(const PCA9685LEDResource& PCA9685LED)
{
    m_resource = PCA9685LED.m_resource;
    m_uri = PCA9685LED.m_uri;
}

PCA9685LEDResource& PCA9685LEDResource::operator=(PCA9685LEDResource&& PCA9685LED)
{
    m_resource = std::move(PCA9685LED.m_resource);
    m_uri = std::move(PCA9685LED.m_uri);
}

/**
 * @brief setOutputPortPin
 *
 * @param portPin
 */
void PCA9685LEDResource::setPinOutputs(PinOutput red, PinOutput green, PinOutput blue)
{
    m_redLED = red;
    m_greenLED = green;
    m_blueLED = blue;
}

/**
 * @brief getXXOutputPin
 *
 * @return
 */
PinOutput PCA9685LEDResource::getRedOutputPin()
{
    return m_redLED;
}

PinOutput PCA9685LEDResource::getGreenOutputPin()
{
    return m_greenLED;
}

PinOutput PCA9685LEDResource::getBlueOutputPin()
{
    return m_blueLED;
}

RPIRCSResourceObject::Ptr PCA9685LEDResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int PCA9685LEDResource::createResource(int redPin, int greenPin, int bluePin)
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resourceTypes = {OIC_DEVICE_LIGHT, OIC_TYPE_COLOUR_RGB};

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resourceTypes), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT,
                                                                "oic.if.rw", "oic.if.a"})));

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&PCA9685LEDResource::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));

    // Set the attributes
    this->setAttributes();

    // Set the RGB LED Pins
    m_redLED.pin   = redPin;
    m_greenLED.pin = greenPin;
    m_blueLED.pin  = bluePin;

    return 1;
}

/**
 * @brief PCA9685LEDResource::setUri
 * @param uri
 */
void PCA9685LEDResource::setUri(std::string& uri)
{
    m_uri = uri;
}

/**
 * @brief PCA9685LEDResource::getUri
 * @return
 */
std::string PCA9685LEDResource::getUri()
{
    return m_uri;
}

/**
 * @brief setRequestHandler
 *
 * @param request
 * @param attr
 */
void PCA9685LEDResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    // Lookup the power attribute
    std:cout << "============================================ \n";
    for(const auto &attr : attrs)
    {
        if(static_cast<int>(attr.value().getType().getId()) == TYPE_VECTOR)
        {
            std::vector<int> rgbValues = attr.value().get<std::vector<int>>();

            // Insert new values into the PinOutput values
            m_redLED.value   = rgbValues[0];
            m_greenLED.value = rgbValues[1];
            m_blueLED.value  = rgbValues[2];

            std::cout << "Red Value: " << m_redLED.value << std::endl;
            std::cout << "Green Value: " << m_greenLED.value << std::endl;
            std::cout << "Blue Value: " << m_blueLED.value << std::endl;
        }
    }
    std::cout << "============================================ \n";

    // Set the outputs
    this->setPCA9685Outputs();
}


/**
 * @brief setAttributes
 */
void PCA9685LEDResource::setAttributes()
{
    const std::vector<int> LEDValues = {m_redLED.value, m_blueLED.value, m_greenLED.value};
    RCSResourceAttributes::Value value(LEDValues);
    m_resource->addAttribute("rgbValue", value);
}


/**
 * @brief Set the outputs of the PCA9685 moduel
 */
void PCA9685LEDResource::setPCA9685Outputs()
{
    // Check if the current I2C address is active
    PCA9685* activePCA9685 = getActivePCA9685Struct();
    if(!(activePCA9685->i2cAddress == static_cast<uint8_t>(m_I2CAddress)))
    {
        std::cout << "Setting the I2C Address of the PCA9685 module" << std::endl;
        // Set the new I2C address
        PCA9685Setup(static_cast<uint8_t>(m_I2CAddress), static_cast<uint16_t>(m_frequency));
    }

    // DEBUG
    std::cout << "Active I2CAddress for this module is: " << m_I2CAddress << std::endl;

    // Set the new pin values
    static const int FREQ_RES_TEMP = 255;
    //  TODO: Replace FREQ_RES_TEMP with FREQUENCY_RESOLOTUION. Update APP
    //  To support 4096 resolution instead of 256
    // Red LED
    float dutyCycle = static_cast<float>(m_redLED.value)/static_cast<float>(FREQ_RES_TEMP) * 100.0;
    PCA9685SetPWMDC(m_redLED.pin, static_cast<uint8_t>(dutyCycle));

    // Green LED
    dutyCycle = static_cast<float>(m_greenLED.value)/static_cast<float>(FREQ_RES_TEMP) * 100.0;
    PCA9685SetPWMDC(m_greenLED.pin, static_cast<uint8_t>(dutyCycle));

    // Blue LED
    dutyCycle = static_cast<float>(m_blueLED.value)/static_cast<float>(FREQ_RES_TEMP) * 100.0;
    PCA9685SetPWMDC(m_blueLED.pin,  static_cast<uint8_t>(dutyCycle));
}
