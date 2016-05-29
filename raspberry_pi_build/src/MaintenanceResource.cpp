#include "MaintenanceResource.h"

using namespace OIC;
using namespace OIC::Service;



/**
 * @brief getInstance
 * @return The instance of the class
 */
MaintenanceResource* MaintenanceResource::getInstance()
{
    static MaintenanceResource* instance(new MaintenanceResource);
    return instance;
}

/**
 * Destructor
 */
MaintenanceResource::~MaintenanceResource()
{
    if(m_resource)
    {
        m_resource.reset();
    }
}

/**
 * @brief Retrieve the attributes
 *
 * @return The attributes
 */
RCSResourceAttributes MaintenanceResource::getAttributes()
{
    return m_attributes;
}

/**
 * @brief Configure the attributes
 *
 * @param attr  New attributes
 */
void MaintenanceResource::setAttributes(const RCSResourceAttributes &attr)
{
    m_attributes = attr;
}

void MaintenanceResource::setAttributes(RCSResourceAttributes &&attr)
{
    m_attributes = std::move(attr);
}

/**
 * @brief getUri
 *
 * @return The uri
 */
std::string MaintenanceResource::getUri()
{
    return m_uri;
}

/**
 * @brief getResourceType
 *
 * @return The Resource Type
 */
std::string MaintenanceResource::getResourceType()
{
    return m_resourceType;
}

/**
 * @brief Set the configuration resource
 *
 * @param resource  Pointer to the resource
 */
void MaintenanceResource::setConfigurationResource(ConfigurationResource::Ptr resource)
{
    m_configurationResource = resource;
}

/**
 * @brief Set the factory callback
 *
 * @param cb        Callback to overwrite default
 */
void MaintenanceResource::setFactoryResetCallback(FactoryResetCallback cb)
{
    std::bind(&m_factoryResetCallback, this, std::move(cb));
}

/**
 * @brief Set the reboot callback
 *
 * @param cb        Callback to overwrite default
 */
void MaintenanceResource::setRebootCallback(RebootCallback cb)
{
    std::bind(&m_rebootCallback, this, std::move(cb));
}

/**
 * @brief Set the stat collection callback
 *
 * @param cb        Callback to overwrite default
 */
void MaintenanceResource::setStatCollectionCallback(StatCollectionCallback cb)
{
    std::bind(&m_statCollectionCallback, this, std::move(cb));
}


/**
 * @brief Default constructor
 */
MaintenanceResource::MaintenanceResource() : m_uri(MAINTENANCE_URI), m_resourceType(MAINTENANCE_RESOURCE_TYPE),
    m_factoryResetCallback(std::bind(&MaintenanceResource::factoryReset, this)),
    m_rebootCallback(std::bind(&MaintenanceResource::rebootDevice, this)),
    m_statCollectionCallback(std::bind(&MaintenanceResource::statCollection, this))
{
    m_attributes[MAINTENANCE_FACTORY_RESET]     = false;
    m_attributes[MAINTENANCE_REBOOT]            = false;
    m_attributes[MAINTENANCE_STAT_COLLECTION]   = false;
}

/**
 * @brief Overwrites the standard setRequestHandler
 *
 * @param request   Request from client
 * @param attr      New attributes to overwrite current
 *
 * @return Response of the POST method
 */
RCSSetResponse MaintenanceResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    for(const auto& attr : attrs)
    {
        std::string value = attr.value().toString();

        if(attr.key().compare(MAINTENANCE_FACTORY_RESET) == 0)
        {
            //std::cout << "Factory Key: " << attr.key() << " Values: " << value << std::endl;
            if(value.compare("true") == 0)
            {
                this->factoryReset();
            }
        }
        else if(attr.key().compare(MAINTENANCE_REBOOT) == 0)
        {
            //std::cout << "Reboot Key: " << attr.key() << " Values: " << value << std::endl;
            if(value.compare("true") == 0)
            {
                this->rebootDevice();
            }
        }
        else if(attr.key().compare(MAINTENANCE_STAT_COLLECTION) == 0)
        {
            //std::cout << "Stat Key: " << attr.key() << " Values: " << value << std::endl;
            if(value.compare("true") == 0)
            {
                this->statCollection();
            }
        }
        else
        {
            std::cout << "Invalid Key request" << std::endl;
        }
    }

    return RCSSetResponse::defaultAction();
}

/**
 * @brief Creates a /oic/mnt resource instance
 */
void MaintenanceResource::createResource()
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
        m_resource->setAttribute(MAINTENANCE_FACTORY_RESET, m_attributes[MAINTENANCE_FACTORY_RESET]);
        m_resource->setAttribute(MAINTENANCE_REBOOT, m_attributes[MAINTENANCE_REBOOT]);
        m_resource->setAttribute(MAINTENANCE_STAT_COLLECTION, m_attributes[MAINTENANCE_STAT_COLLECTION]);

        m_resource->setSetRequestHandler(std::bind(&MaintenanceResource::setRequestHandler, this, std::placeholders::_1,
                                                   std::placeholders::_2));
    }
}

/**
 * @brief Perform a factory reset of the parameters
 */
void MaintenanceResource::factoryReset()
{
    std::cout << __func__ << std::endl;

    m_attributes[MAINTENANCE_FACTORY_RESET]     = false;
    m_attributes[MAINTENANCE_REBOOT]            = false;
    m_attributes[MAINTENANCE_STAT_COLLECTION]   = false;
}

/**
 * @brief Reboot the device
 */
void MaintenanceResource::rebootDevice()
{
    std::cout << __func__ << std::endl;
#ifdef __linux__
    int res;
    std::cout << "Reboot will be soon..." << std::endl;
    m_attributes[MAINTENANCE_REBOOT] = false;
    m_resource->notify();

    // Reboot linux
    sync();
    reboot(RB_AUTOBOOT);

    //res = system("/usr/bin/sudo /etc/init.d/reboot start"); // System reboot for linux
    //std::cout << "return: " << res << std::endl;
#else
    std::cerr << "Unsupported Operating System" << std::endl;
#endif
}

/**
 * @brief Start statistical collection
 */
void MaintenanceResource::statCollection()
{
    std::cout << __func__ << std::endl;
}
