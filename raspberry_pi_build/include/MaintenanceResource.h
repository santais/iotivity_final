#ifndef MAINTENANCERESOURCE_H_
#define MAINTENANCERESOURCE_H_

#include "RCSRemoteResourceObject.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"
#include "RCSDiscoveryManager.h"
#include "RCSAddress.h"

#include "ConfigurationResource.h"

#include <string>
#include <unistd.h>
#include <sys/reboot.h>

#include "OCApi.h"
#include "OCPlatform.h"

using namespace OIC;
using namespace OIC::Service;

namespace
{
    static const std::string MAINTENANCE_URI                = "/oic/mnt";
    static const std::string MAINTENANCE_RESOURCE_TYPE      = "oic.wk.mnt";

    static const std::string MAINTENANCE_FACTORY_RESET      = "fr";
    static const std::string MAINTENANCE_REBOOT             = "rb";
    static const std::string MAINTENANCE_STAT_COLLECTION    = "ssc";
    static const std::string MAINTENANCE_SHUTDOWN           = "sd";
}

class MaintenanceResource
{
public:
    typedef std::unique_ptr<MaintenanceResource> Ptr;
    typedef std::unique_ptr<const MaintenanceResource> ConstPtr;

    typedef std::function <void () > FactoryResetCallback;
    typedef std::function <void () > RebootCallback;
    typedef std::function <void () > ShutdownCallback;
    typedef std::function <void () > StatCollectionCallback;
public:

    /**
     * @brief getInstance
     * @return The instance of the class
     */
    static MaintenanceResource* getInstance();

    /**
     * Destructor
     */
    ~MaintenanceResource();

    /**
     * @brief Retrieve the attributes
     *
     * @return The attributes
     */
    RCSResourceAttributes getAttributes();

    /**
     * @brief Configure the attributes
     *
     * @param attr  New attributes
     */
    void setAttributes(const RCSResourceAttributes &attr);
    void setAttributes(RCSResourceAttributes &&attr);

    /**
     * @brief getUri
     *
     * @return The uri
     */
    std::string getUri();

    /**
     * @brief getResourceType
     *
     * @return The Resource Type
     */
    std::string getResourceType();

    /**
     * @brief Set the configuration resource
     *
     * @param resource  Pointer to the resource
     */
    void setConfigurationResource(ConfigurationResource::Ptr resource);

    /**
     * @brief Set the factory callback
     *
     * @param cb        Callback to overwrite default
     */
    void setFactoryResetCallback(FactoryResetCallback cb);

    /**
     * @brief Set the reboot callback
     *
     * @param cb        Callback to overwrite default
     */
    void setRebootCallback(RebootCallback cb);

    /**
     * @brief Set the stat collection callback
     *
     * @param cb        Callback to overwrite default
     */
    void setStatCollectionCallback(StatCollectionCallback cb);

    /**
     * @brief setShutdownCallback
     *
     * @param cb
     */
    void setShutdownCallback(ShutdownCallback cb);

    /**
     * @brief Creates a /oic/mnt resource instance
     */
    void createResource();

private:
    /**
     * @brief Default constructor
     */
    MaintenanceResource();

    /**
     * @brief Overwrites the standard setRequestHandler
     *
     * @param request   Request from client
     * @param attr      New attributes to overwrite current
     *
     * @return Response of the POST method
     */
    RCSSetResponse setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs);

    /**
     * @brief Perform a factory reset of the parameters
     */
    void factoryReset();

    /**
     * @brief Reboot the device
     */
    void rebootDevice();

    /**
     * @brief Start statistical collection
     */
    void statCollection();

    /**
     * @brief shutdown the controller
     */
    void shutdown();



private:

    /**
     * Configuration attribute
     */
    RCSResourceAttributes m_attributes;

    /**
     * Object of the Configuration Resource.
     */
    RCSResourceObject::Ptr m_resource;

    /**
     * @brief Reference to the configuration resource
     */
    ConfigurationResource::Ptr m_configurationResource;

    /**
     * URI of the Configuration Resource
     */
    std::string m_uri;

    /**
     * Resource Type of the Configuration Resource
     */
    std::string m_resourceType;

    /**
     * @brief Callback for factory reset
     */
    FactoryResetCallback m_factoryResetCallback;

    /**
     * @brief Callback for reboot
     */
    RebootCallback m_rebootCallback;

    /**
     * @brief Callback for stat collection
     */
    StatCollectionCallback m_statCollectionCallback;

    /**
     * @brief Callback for shutdown
     */
    ShutdownCallback m_shutdownCallback;

};

#endif /* MAINTENANCERESOURCE_H_ */
