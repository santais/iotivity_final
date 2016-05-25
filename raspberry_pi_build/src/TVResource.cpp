#include "TVResource.h"
#include "resource_types.h"

TVResource::TVResource() {}

/**
 * @brief TVResource::TVResource
 *
 * @param portPin
 * @param uri       Uri of the new light resource
 */
TVResource::TVResource(const std::string &uri)
{
    m_uri = uri;
}

TVResource::~TVResource()
{
    m_resource.reset();
}

TVResource::TVResource(const TVResource& light) :
    m_resource(light.m_resource),
    m_uri(light.m_uri)
{}

TVResource::TVResource(TVResource&& light) :
    m_resource(std::move(light.m_resource)),
    m_uri(std::move(light.m_uri))
{}

TVResource& TVResource::operator=(const TVResource& light)
{
    m_resource = light.m_resource;
    m_uri = light.m_uri;
}

TVResource& TVResource::operator=(TVResource&& light)
{
    m_resource = std::move(light.m_resource);
    m_uri = std::move(light.m_uri);
}

RPIRCSResourceObject::Ptr TVResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int TVResource::createResource()
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resource_types = {OIC_DEVICE_TV, OIC_TYPE_MEDIA_SOURCE, OIC_TYPE_BINARY_SWITCH};

    try
    {
    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resource_types), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT})));
    }
    catch(RCSInvalidParameterException e)
    {
        std::cout << e.what() << std::endl;
    }

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&TVResource::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));

    // Set the attributes
    this->setAttributes();

    return 1;
}

/**
 * @brief TVResource::setUri
 * @param uri
 */
void TVResource::setUri(std::string& uri)
{
    m_uri = uri;
}

/**
 * @brief TVResource::getUri
 * @return
 */
std::string TVResource::getUri()
{
    return m_uri;
}


/**
 * @brief setRequestHandler
 *
 * @param request
 * @param attr
 */
void TVResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    // Lookup the power attribute
    std::cout << "============================================ \n";
        std::cout << "New attributes are:" << std::endl;
        for(const auto& attr : attrs)
        {
            std::cout << " \tKey: " << attr.key() << " Value: " << attr.value().toString() << std::endl;
        }

        std::cout << "\n";

        if(attrs["tvMode"].toString().compare("TV_OFF") == 0)
        {
            std::cout << "\tTV mode is set to OFF" << std::endl;
        }
        else if(attrs["tvMode"].toString().compare("TV_ON") == 0)
        {
            std::cout << "\tTV mode is set to ON" << std::endl;
        }
    std::cout << "============================================ \n";
}


/**
 * @brief setAttributes
 */
void TVResource::setAttributes()
{
    //mediaSource
    RCSResourceAttributes::Value sourceName("HDMI_CEC");
    RCSResourceAttributes::Value sourceNumber((int) 1);
    RCSResourceAttributes::Value sourceType((int) 1);
    RCSResourceAttributes::Value status((bool) false);

    //binarySwitch
    RCSResourceAttributes::Value state((bool) false);


    m_resource->addAttribute("sourceName", sourceName);
    m_resource->addAttribute("sourceNumber", sourceNumber);
    m_resource->addAttribute("sourceType", sourceType);
    m_resource->addAttribute("status", status);

    // Binary State
    m_resource->addAttribute("state", state);
}
