#include "LightResource.h"
#include "resource_types.h"

LightResource::LightResource() :
    m_hosting(false)
{
    // Not initialized
#ifdef ARM
//    wiringPiSetup();
    m_outputPortPin = -1;
#endif
}

/**
 * @brief LightResource::LightResource
 *
 * @param portPin
 * @param uri       Uri of the new light resource
 */
LightResource::LightResource(int portPin, const std::string &uri) :
    m_hosting(false)
{
    m_uri = uri;

    m_outputPortPin = portPin;

    // Initialize pins
#ifdef ARM
  //  wiringPiSetup();
    pinMode(portPin, OUTPUT);
    std::cout << "TARGET IS ARM" << std::endl;
#endif
}

LightResource::~LightResource()
{
    m_resource.reset();
}

LightResource::LightResource(const LightResource& light) :
    m_resource(light.m_resource),
    m_outputPortPin(light.m_outputPortPin),
    m_uri(light.m_uri)
{}

LightResource::LightResource(LightResource&& light) :
    m_resource(std::move(light.m_resource)),
    m_outputPortPin(std::move(light.m_outputPortPin)),
    m_uri(std::move(light.m_uri))
{}

LightResource& LightResource::operator=(const LightResource& light)
{
    m_resource = light.m_resource;
    m_outputPortPin = light.m_outputPortPin;
    m_uri = light.m_uri;
}

LightResource& LightResource::operator=(LightResource&& light)
{
    m_resource = std::move(light.m_resource);
    m_outputPortPin = std::move(light.m_outputPortPin);
    m_uri = std::move(light.m_uri);
}

/**
 * @brief setOutputPortPin
 *
 * @param portPin
 */
void LightResource::setOutputPortPin(int portPin)
{
#ifdef ARM
    pinMode(portPin, OUTPUT);
    m_outputPortPin = portPin;
#endif ARM
}

/**
 * @brief getOutputPortPin
 *
 * @return
 */
int LightResource::getOutputPortPin()
{
    return m_outputPortPin;
}

RPIRCSResourceObject::Ptr LightResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int LightResource::createResource()
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resourceTypes = {OIC_DEVICE_LIGHT, OIC_TYPE_BINARY_SWITCH};

    if(m_hosting)
    {
        resourceTypes.push_back(OIC_TYPE_RESOURCE_HOST);
    }

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resourceTypes), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT})));

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&LightResource::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));

    // Set the attributes
    this->setAttributes();

    return 1;
}

/**
 * @brief LightResource::setUri
 * @param uri
 */
void LightResource::setUri(std::string& uri)
{
    m_uri = uri;
}

/**
 * @brief LightResource::getUri
 * @return
 */
std::string LightResource::getUri()
{
    return m_uri;
}


/**
 * @brief setHostingResource
 */
void LightResource::setHostingResource()
{
    m_hosting = true;
}

/**
 * @brief setRequestHandler
 *
 * @param request
 * @param attr
 */
void LightResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    // Lookup the power attribute
    std:cout << "============================================ \n";
    if(m_outputPortPin >=    0)
    {
        if(attrs["power"] == true)
        {
            std::cout << "\t Key: Power is set to TRUE" << std::endl;
#ifdef ARM
            digitalWrite(m_outputPortPin, HIGH);
#endif
        }
        else if (attrs["power"] == false)
        {
            std::cout << "\t Key: State is set to FALSE" << std::endl;
#ifdef ARM
            digitalWrite(m_outputPortPin, LOW);
#endif
        }
        else
        {
            std::cerr << "Unable to find attribute power" << std::endl;
        }
    }
    else
    {
        std::cout << "Output pin has not been initialized. Please call setOutputPortPin(...)" << std::endl;
    }
    std::cout << "============================================ \n";
}


/**
 * @brief setAttributes
 */
void LightResource::setAttributes()
{
    RCSResourceAttributes::Value power((bool) false);
    m_resource->addAttribute("power", power);
}
