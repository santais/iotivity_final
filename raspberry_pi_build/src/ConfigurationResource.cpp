#include "ConfigurationResource.h"

using namespace OIC;
using namespace OIC::Service;

static const std::string TAG = "ConfigurationResource ";

void onRemoteAttributesReceived(RCSResourceAttributes &attributes)
{
    std::cout << "onRemoteAttributesReceived callback" << std::endl;

    //printAttributes(attributes);
}

ConfigurationResource::ConfigurationResource() : m_uri(CONFIGURATION_URI), m_resourceType(CONFIGURATION_RESOURCE_TYPE)
{
    std::cout << TAG << __func__ << std::endl;

    // Find the bootstrap server
    //findBootstrapServer();
}

ConfigurationResource::~ConfigurationResource()
{

}

/**
 * @brief Returns an instance of the ConfigurationResource class
 *
 * @return
 */
ConfigurationResource* ConfigurationResource::getInstance()
{
    static ConfigurationResource* instance(new ConfigurationResource);
    return instance;
}

/**
 * @brief Retrieve the configuration attributes
 *
 * @return Attributes
 */
RCSResourceAttributes ConfigurationResource::getConfigurationAttributes()
{
    return m_attributes;
}

/**
 * @brief Set the configuration attributes
 *
 * @param attr New attributes to be setConfigurationAttributes();
 */
void ConfigurationResource::setConfigurationAttributes(const RCSResourceAttributes &attr)
{
    m_attributes = std::move(attr);
}

/**
 * @brief Ǵet the configuration uri
 *
 * @return Configuration uri
 */
std::string ConfigurationResource::getUri()
{
    return m_uri;
}

/**
 * @brief Get the Configuration Resource Resource Type
 *
 * @return The Resource Type
 */
std::string ConfigurationResource::getResourceType()
{
    return m_resourceType;
}

void ConfigurationResource::bootstrap(BootstrapCallback cb)
{
    std::cout << __func__ << std::endl;
    m_bootstrapCallback = std::move(cb);

    // Discover the bootstrap server
    std::cout << TAG << __func__ << std::endl;
    m_discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                                                "bootstrap", std::bind(&ConfigurationResource::onBootstrapServerFound,
                                                                          this, std::placeholders::_1));
}

/**
 * @brief GET handler when a GET method is requested
 *
 * @param request   The request from the client
 * @param attr      Attributes of the configuration resource
 *
 * @return Response of the request
 */
RCSGetResponse ConfigurationResource::getRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr)
{
    std::cout << TAG << __func__ << std::endl;

    printAttributes(attr);

    return RCSGetResponse::defaultAction();
}

/**
 * @brief SET handler when a SET method is requested
 *
 * @param request   The request from the client
 * @param attr      Attributes to be set for the configuration resource
 *
 * @return Response of the SET request.
 */
RCSSetResponse ConfigurationResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr)
{
    std::cout << TAG << __func__ << std::endl;

    m_attributes = std::move(attr);

    RCSSetResponse::defaultAction();
}

/**
 * @brief Creates the configuration resource
 */
void ConfigurationResource::createResource()
{
    m_resource = RCSResourceObject::Builder(m_uri, m_resourceType, OC_RSRVD_INTERFACE_DEFAULT)
            .setDiscoverable(true)
            .setObservable(true)
            .setSecureFlag(false)
            .build();

    if(m_resource == nullptr)
    {
        std::cerr << "Unable to create resource" << std::endl;
    }
    else
    {
        m_resource->setAttribute(CONFIGURATION_DEVICE_NAME, m_attributes[CONFIGURATION_DEVICE_NAME]);
        m_resource->setAttribute(CONFIGURATION_LOCATION, m_attributes[CONFIGURATION_LOCATION]);
        m_resource->setAttribute(CONFIGURATION_LOCATION_NAME, m_attributes[CONFIGURATION_LOCATION_NAME]);
        m_resource->setAttribute(CONFIGURATION_CURRENCY, m_attributes[CONFIGURATION_CURRENCY]);
        m_resource->setAttribute(CONFIGURATION_REGION, m_attributes[CONFIGURATION_REGION]);

       /* m_resource->setGetRequestHandler(std::bind(&ConfigurationResource::getRequestHandler, this, std::placeholders::_1,
                                                   std::placeholders::_2));
        */m_resource->setSetRequestHandler(std::bind(&ConfigurationResource::setRequestHandler, this, std::placeholders::_1,
                                                   std::placeholders::_2));
    }
}

/**
 * @brief Search for a bootstrap server
 */
/*
void ConfigurationResource::boostrap()
{
    // Discover the bootstrap server
    std::cout << TAG << __func__ << std::endl;
    m_discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                                                "bootstrap", std::bind(&ConfigurationResource::onBootstrapServerFound,
                                                                          this, std::placeholders::_1));
}*/

/**
 * @brief Method invoked when a resource has been found
 *
 * @param resource  The found resource
 */
void ConfigurationResource::onBootstrapServerFound(RCSRemoteResourceObject::Ptr resource)
{
    std::cout << TAG << __func__ << std::endl;
    std::cout << "\t Found resource with uri: " << resource->getUri() << std::endl;

    m_remoteResource = resource;

    resource->getRemoteAttributes(std::bind(&ConfigurationResource::onGetBootstrapServer, this,
                                            std::placeholders::_1, std::placeholders::_2));

  /*  resource->get(std::bind(&ConfigurationResource::getCallback, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3));
*/

    // Cancel and delete the discovery task
    m_discoveryTask->cancel();
    m_discoveryTask.reset();
}

 void ConfigurationResource::getRemoteAttributes(RCSRemoteResourceObject::RemoteAttributesGetCallback cb)
 {
     if(cb && m_remoteResource)
     {
         m_remoteResource->getRemoteAttributes(cb);
     }
 }

void ConfigurationResource::getCallback(const HeaderOpts &, const RCSRepresentation &, int eCode)
{
    std::cout << __func__ << std::endl;
}

/**
 * @brief Method invoked through a GET request to the bootstrap server
 *
 * @param attrs     The received attributes
 * @param eCode     Result of the GET request
 */
void ConfigurationResource::onGetBootstrapServer(const RCSResourceAttributes &attrs, int eCode)
{
    m_bootstrapCallback(attrs);

    std::cout << "Result of get Request: " << eCode << std::endl;

    printAttributes(attrs);

    for(const auto& attr : attrs)
    {
        if(attr.key().compare(CONFIGURATION_DEVICE_NAME) == 0)
        {
            std::cout << "Setting device name to: "  << attr.value().toString() << std::endl;
            m_attributes[attr.key()] = attr.value().toString();
        }
        else if(attr.key().compare(CONFIGURATION_LOCATION) == 0)
        {
            std::cout << "Setting location to: "  << attr.value().toString() << std::endl;
            m_attributes[attr.key()] = attr.value().toString();
        }
        else if(attr.key().compare(CONFIGURATION_LOCATION_NAME) == 0)
        {
            std::cout << "Setting location name to: "  << attr.value().toString() << std::endl;
            m_attributes[attr.key()] = attr.value().toString();
        }
        else if(attr.key().compare(CONFIGURATION_CURRENCY) == 0)
        {
            std::cout << "Setting currency to: "  << attr.value().toString() << std::endl;
            m_attributes[attr.key()] = attr.value().toString();
        }
        else if(attr.key().compare(CONFIGURATION_REGION) == 0)
        {
            std::cout << "Setting region to: "  << attr.value().toString() << std::endl;
            m_attributes[attr.key()] == attr.value().toString();
        }
    }

    // Create the new resource
    this->createResource();
}

void ConfigurationResource::printAttributes(const RCSResourceAttributes &attrs)
{
    std::cout << __func__ << std::endl;
    for(const auto& attr : attrs)
    {
        std::cout << attr.key() <<  " " << attr.value().toString() << std::endl;
    }
}

void ConfigurationResource::printAttributes(RCSResourceAttributes &&attrs)
{
    std::cout << __func__ << std::endl;
    for(const auto& attr : attrs)
    {
        std::cout << attr.key() <<  " " << attr.value().toString() << std::endl;
    }
}

