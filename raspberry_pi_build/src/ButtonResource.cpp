#include "ButtonResource.h"
#include "resource_types.h"

ButtonResource::ButtonResource()
{
    // Not initialized
#ifdef ARM
//    wiringPiSetup();
    m_inputPortPin = -1;
#endif
}

/**
 * @brief ButtonResource::ButtonResource
 *
 * @param portPin
 * @param uri       Uri of the new light resource
 */
ButtonResource::ButtonResource(int portPin, const std::string &uri)
{
    m_uri = uri;

    m_inputPortPin = portPin;

    // Initialize pins
#ifdef ARM
 //   wiringPiSetup();
    pinMode(portPin, OUTPUT);
#endif
}

ButtonResource::~ButtonResource()
{
    m_resource.reset();
    //m_buttonInputThread.detach();
}

ButtonResource::ButtonResource(const ButtonResource& light) :
    m_resource(light.m_resource),
    m_inputPortPin(light.m_inputPortPin),
    m_uri(light.m_uri)
{}

ButtonResource::ButtonResource(ButtonResource&& light) :
    m_resource(std::move(light.m_resource)),
    m_inputPortPin(std::move(light.m_inputPortPin)),
    m_uri(std::move(light.m_uri))
{}

ButtonResource& ButtonResource::operator=(const ButtonResource& light)
{
    m_resource = light.m_resource;
    m_inputPortPin = light.m_inputPortPin;
    m_uri = light.m_uri;
}

ButtonResource& ButtonResource::operator=(ButtonResource&& light)
{
    m_resource = std::move(light.m_resource);
    m_inputPortPin = std::move(light.m_inputPortPin);
    m_uri = std::move(light.m_uri);
}

/**
 * @brief setInputPortPin
 *
 * @param portPin
 */
void ButtonResource::setInputPortPin(int portPin)
{
#ifdef ARM
    pinMode(portPin, OUTPUT);
    m_inputPortPin = portPin;
#endif
}

/**
 * @brief getInputPortPin
 *
 * @return
 */
int ButtonResource::getInputPortPin()
{
    return m_inputPortPin;
}

RPIRCSResourceObject::Ptr ButtonResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int ButtonResource::createResource()
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resource_types = {OIC_DEVICE_BUTTON};

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resource_types), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT,
                                                                 OC_RSRVD_INTERFACE_READ, "oic.if.a"})));

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&ButtonResource::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));


    // Set the attributes
    this->setAttributes();

    // Start listener thread
#ifdef ARM
    std::thread m_buttonInputThread = std::thread(&ButtonResource::readInputThread, this);
    m_buttonInputThread.join();
#endif

    return 1;
}

void ButtonResource::setUri(std::string& uri)
{
    m_uri = uri;
}

std::string ButtonResource::getUri()
{
    return m_uri;
}

/**
 * @brief setRequestHandler
 *
 * @param request
 * @param attr
 */
void ButtonResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    // Lookup the state attribute
    std::cout << "============================================ \n";
    if(m_inputPortPin >= 0)
    {
        if(attrs["state"] == true)
        {
            std::cout << "\t Key: State is set to TRUE" << std::endl;
#ifdef ARM
 //           digitalRead(m_inputPortPin, HIGH);
#endif ARM
        }
        else if (attrs["state"] == false)
        {
            std::cout << "\t Key: State is set to FALSE" << std::endl;
#ifdef ARM
//            digitalRead(m_inputPortPin, LOW);
#endif
        }
        else
        {
            std::cerr << "Unable to find attribute state" << std::endl;
        }
    }
    else
    {
        std::cout << "Input pin has not been initialized. Please call setInputPortPin(...)" << std::endl;
    }
    std::cout << "============================================ \n";
}


/**
 * @brief setAttributes
 */
void ButtonResource::setAttributes()
{
    RCSResourceAttributes::Value power((bool) false);
    m_resource->addAttribute("state", power);
}

/**
 * @brief readInputThread
 */
void ButtonResource::readInputThread()
{
    std::string state = "state";
    RCSResourceAttributes::Value value;
    while(true)
    {
#ifdef ARM
        static bool prevReading = false;

        // Debounce
        bool newReading = digitalRead(m_inputPortPin);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        newReading = digitalRead(m_inputPortPin);
        if(newReading != prevReading)
        {
            // Set the new attribute
            value = RCSResourceAttributes::Value(newReading);
            m_resource->setAttribute(state, value);

            m_resource->getResourceObject()->notify();
            prevReading = newReading;
        }
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
