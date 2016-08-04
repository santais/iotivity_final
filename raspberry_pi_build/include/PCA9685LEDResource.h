#ifndef PCA9685LEDRESOURCE_H_
#define PCA9685LEDRESOURCE_H_

#include "RPIRCSResourceObject.h"
#ifdef ARM
    #include "wiringPi.h"
#endif

#include "PCA9685RPi.h"

namespace PCA9685Constants
{
    static const int TYPE_VECTOR = 6;

    static const std::string RGB_ATTRIBUTE_NAME = "rgbValue";
    static const std::string STATE_ATTRIBUTE_NAME = "state";
}

class PCA9685LEDResource
{
public:
        typedef std::shared_ptr<PCA9685LEDResource> Ptr;
        typedef std::shared_ptr<const PCA9685LEDResource> ConstPtr;

public:
    PCA9685LEDResource();

    PCA9685LEDResource(const std::string &uri, int i2cAddress, int frequnecy);

    ~PCA9685LEDResource();

    PCA9685LEDResource(const PCA9685LEDResource&);
    PCA9685LEDResource(PCA9685LEDResource &&);
    PCA9685LEDResource& operator=(const PCA9685LEDResource&);
    PCA9685LEDResource& operator=(PCA9685LEDResource&&);

    /**
     * @brief setOutputPortPin
     *
     * @param portPin
     */
    void setPinOutputs(PinOutput red, PinOutput green, PinOutput blue);

    /**
     * @brief getXXOutputPin
     *
     * @return
     */
    PinOutput getRedOutputPin();
    PinOutput getGreenOutputPin();
    PinOutput getBlueOutputPin();

    /**
     * @brief getResourceObject
     *
     * @return
     */
    RPIRCSResourceObject::Ptr getResourceObject();

    /**
     * @brief createResource
     */
    int createResource(int redPin, int greenPin, int bluePin);

    /**
     * @brief setUri
     * @param uri
     */
    void setUri(std::string& uri);

    /**
     * @brief getUri
     * @return
     */
    std::string getUri();

    /**
     * @brief Set the RGB values manually
     *
     * @param values RGB values (Red, Green, Blue)
     */
    void setRGBValues(std::vector<int> values);

    /**
     * @brief getState
     * @return
     */
    bool getState();

    /**
     * @brief setState
     * @param state
     */
    void setState(bool state);

private:
    /**
     * Resource object
     */
    RPIRCSResourceObject::Ptr m_resource;

    /**
     * @brief m_uri
     */
    std::string m_uri;

    /**
     * @brief m_I2CAddress
     */
    int m_I2CAddress;

    /**
     * @brief pin and value pins
     */
    PinOutput m_redLED;
    PinOutput m_greenLED;
    PinOutput m_blueLED;

    /**
     * @brief frequency of the targeted i2c led driver
     */
    int m_frequency;

    /**
     * @brief State of the cup. ON/OFF
     */
    bool m_state;

private:
    /**
     * @brief setRequestHandler
     *
     * @param request
     * @param attr
     */
    void setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr);

    /**
     * @brief setAttributes
     */
    void setAttributes();

    /**
     * @brief Set the outputs of the PCA9685 moduel
     */
    void setPCA9685Outputs();
};


#endif /* PCA9685LEDRESOURCE_H_ */
