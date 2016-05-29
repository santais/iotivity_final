#ifndef CONFIGURATIONRESOURCE_H
#define CONFIGURATIONRESOURCE_H


#include "RCSRemoteResourceObject.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"
#include "RCSDiscoveryManager.h"
#include "RCSAddress.h"

#include <string>

#include "OCApi.h"
#include "OCPlatform.h"

using namespace OIC;
using namespace OIC::Service;

static std::string BOOSTRAP_RESOURCE_TYPE       = "boostrap";
static std::string CONFIGURATION_URI            = "/oic/con";
static std::string CONFIGURATION_RESOURCE_TYPE  = "oic.wk.con";

static std::string CONFIGURATION_DEVICE_NAME    = "n";
static std::string CONFIGURATION_LOCATION       = "loc";
static std::string CONFIGURATION_LOCATION_NAME  = "locn";
static std::string CONFIGURATION_CURRENCY       = "c";
static std::string CONFIGURATION_REGION         = "r";


class ConfigurationResource
{
public:
    typedef std::shared_ptr<ConfigurationResource> Ptr;
    typedef std::shared_ptr<const ConfigurationResource> ConstPtr;

    typedef std::function<void (const RCSResourceAttributes&) > BootstrapCallback;

public:

    /**
     * @brief Returns an instance of the ConfigurationResource class
     *
     * @return
     */
    static ConfigurationResource* getInstance();

    ~ConfigurationResource();

    /**
     * @brief Retrieve the configuration attributes
     *
     * @return Attributes
     */
    RCSResourceAttributes getConfigurationAttributes();

    /**
     * @brief Set the configuration attributes
     *
     * @param attr New attributes to be setConfigurationAttributes();
     */
    void setConfigurationAttributes(const RCSResourceAttributes &attr);

    /**
     * @brief Ǵet the configuration uri
     *
     * @return Configuration uri
     */
    std::string getUri();

    /**
     * @brief Get the Configuration Resource Resource Type
     *
     * @return The Resource Type
     */
    std::string getResourceType();

    void getRemoteAttributes(RCSRemoteResourceObject::RemoteAttributesGetCallback cb);

    void bootstrap(BootstrapCallback cb);

private:

    /**
     * Default constructor
     */
    ConfigurationResource();

    /**
     * @brief GET handler when a GET method is requested
     *
     * @param request   The request from the client
     * @param attr      Attributes of the configuration resource
     *
     * @return Response of the request
     */
    RCSGetResponse getRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr);

    /**
     * @brief SET handler when a SET method is requested
     *
     * @param request   The request from the client
     * @param attr      Attributes to be set for the configuration resource
     *
     * @return Response of the SET request.
     */
    RCSSetResponse setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr);

    /**
     * @brief Creates the configuration resource
     */
    void createResource();

    /**
     * @brief Search for a bootstrap server
     */
   // void findBootstrapServer();

    /**
     * @brief Method invoked when a resource has been found
     *
     * @param resource  The found resource
     */
    void onBootstrapServerFound(RCSRemoteResourceObject::Ptr resource);

    /**
     * @brief Method invoked through a GET request to the bootstrap server
     *
     * @param attrs     The received attributes
     * @param eCode     Result of the GET request
     */
    void onGetBootstrapServer(const RCSResourceAttributes &attrs, int eCode);

    void getCallback(const HeaderOpts &, const RCSRepresentation &, int eCode);

    /**
     * @brief Print the attributes
     *
     * @param attrs Attributes to be printed
     */
    void printAttributes(const RCSResourceAttributes &attrs);
    void printAttributes(RCSResourceAttributes &&attrs);

private:
    /**
     * Configuration attribute
     */
    RCSResourceAttributes m_attributes;

    /**
     * Object of the Configuration Resource.
     */
    RCSResourceObject::Ptr m_resource;
    RCSRemoteResourceObject::Ptr m_remoteResource;

    /**
     * URI of the Configuration Resource
     */
    std::string m_uri;

    /**
     * Resource Type of the Configuration Resource
     */
    std::string m_resourceType;

    /**
     * Discovery task manager to discover configuration resource
     */
    RCSDiscoveryManager::DiscoveryTask::Ptr m_discoveryTask;


    BootstrapCallback m_bootstrapCallback;
};

#endif // CONFIGURATIONRESOURCE_H
