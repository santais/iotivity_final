#include "RPIRCSController.h"
#include "RPIRCSResourceObject.h"
#include "RCSRequest.h"

#include "ConfigurationResource.h"
#include "MaintenanceResource.h"
#include "LightResource.h"
#include "ButtonResource.h"
#include "TVResource.h"

#include "resource_types.h"

#include "OCPlatform.h"
#include "OCApi.h"

#include <signal.h>
#include <unistd.h>

#define UNUSED __attribute__((__unused__))

ConfigurationResource::Ptr g_configurationResource;
MaintenanceResource::Ptr g_maintenanceResource;
/*
const static std::vector<std::string> g_lightTypes = {OIC_DEVICE_LIGHT, OIC_TYPE_LIGHT_DIMMING,
                                   OIC_TYPE_BINARY_SWITCH};
const static std::vector<std::string> g_buttonTypes ={OIC_DEVICE_BUTTON};
const static std::vector<std::string> g_interfaces = {OC_RSRVD_INTERFACE_DEFAULT};
const static std::vector<std::string> g_buttonInterfaces = {OC_RSRVD_INTERFACE_DEFAULT, OC_RSRVD_INTERFACE_READ};

RPIRCSResourceObject::Ptr g_lightResource;
RPIRCSResourceObject::Ptr g_buttonResource;
*/

LightResource g_lightResource;
ButtonResource g_buttonResource;
TVResource g_tvResource;

int g_quitFlag = false;

void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        g_quitFlag = 1;
    }
}

void setResponse(const RCSRequest& request, RCSResourceAttributes& attributes)
{
    std::cout << "Got a get Request for resource with uri: " << request.getResourceUri() << std::endl;
    for(const auto& attr : attributes)
    {
        std::cout << "\tkey : " << attr.key() << "\n\tvalue : "
                  << attr.value().toString() << std::endl;
    }

    //return RCSSetResponse::defaultAction();
}

void bootstrapCallback(const RCSResourceAttributes &attrs)
{
    std::cout << __func__ << std::endl;
}

/*
void createLightResource()
{
    g_lightResource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject( "/a/light", std::move(g_lightTypes), std::move(g_interfaces)));
    //std::make_shared<RPIRCSResourceObject>( RPIRCSResourceObject { "/a/light", std::move(g_types), std::move(g_interfaces) } ) ;
    g_lightResource->createResource(true, true, false);

    // Add attributes
    RCSResourceAttributes::Value value((int) 0);
    g_lightResource->addAttribute("brightness", value);
    RCSResourceAttributes::Value power((bool) false);
    g_lightResource->addAttribute("power", power);

    std::cout << "Resource has been created" << std::endl;

    g_lightResource->setReqHandler(setResponse);

    std::cout << "Types are: " << std::endl;
    for(std::string& type : g_lightResource->getTypes())
    {
        std::cout << "\t " << type << std::endl;
    }

    std::cout << "Interfaces are: " << std::endl;
    for(std::string& interface : g_lightResource->getInterfaces())
    {
        std::cout << "\t " << interface << std::endl;
    }
}

void createButtonResource()
{
    g_buttonResource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject( "/a/button", std::move(g_buttonTypes), std::move(g_buttonInterfaces)));
    //std::make_shared<RPIRCSResourceObject>( RPIRCSResourceObject { "/a/light", std::move(g_types), std::move(g_interfaces) } ) ;
    g_buttonResource->createResource(true, true, false);

    // Add attributes
    RCSResourceAttributes::Value value((bool) 0);
    g_buttonResource->addAttribute("state", value);

    std::cout << "Resource has been created" << std::endl;

    g_buttonResource->setReqHandler(setResponse);

    std::cout << "Types are: " << std::endl;
    for(std::string& type : g_buttonResource->getTypes())
    {
        std::cout << "\t " << type << std::endl;
    }

    std::cout << "Interfaces are: " << std::endl;
    for(std::string& interface : g_buttonResource->getInterfaces())
    {
        std::cout << "\t " << interface << std::endl;
    }
}*/

int main()
{
    std::cout << "Starting test program" << std::endl;

    // Create new light resource
    g_lightResource = LightResource(3, "/rpi/light/1");
    g_lightResource.createResource();

    // Create new button resource
    g_buttonResource = ButtonResource(3, "/rpi/button/1");
    g_buttonResource.createResource();

    // Create new TV resource
    g_tvResource = TVResource("/rpi/tv/1");
    g_tvResource.createResource();


    // Setup the configuration resource
    g_configurationResource = ConfigurationResource::Ptr(ConfigurationResource::getInstance());

    g_configurationResource->bootstrap(bootstrapCallback);

    // Setup the maintenance resource
    g_maintenanceResource = MaintenanceResource::Ptr(MaintenanceResource::getInstance());

    g_maintenanceResource->setConfigurationResource(g_configurationResource);
    g_maintenanceResource->createResource();

    // Enable prsence
    if(OCStartPresence(OC_MAX_PRESENCE_TTL_SECONDS - 1) != OC_STACK_OK)
    {
        std::cerr << "Unable to start presence" << std::endl;
    }

    std::cout << "Setup completed" << std::endl;
    signal(SIGINT, handleSigInt);
    while(!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            return 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
