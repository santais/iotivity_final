#include "ConfigurationResource.h"
#include "MaintenanceResource.h"
#include "SpeakerResource.h"

#include <signal.h>
#include <unistd.h>

#define GPIO_RPI_PIN_8 15
#define GPIO_RPI_PIN_10 16

ConfigurationResource::Ptr g_configurationResource;
MaintenanceResource::Ptr g_maintenanceResource;
SpeakerResource g_speakerResource;

int g_quitFlag = false;

void handleSigInt(int signum)
{
    if(signum == SIGINT)
    {
        g_quitFlag = true;
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
    std::cout << "\t Found bootstrap server" << std::endl;
}

int main()
{
    std::cout << "Starting Speaker program" << std::endl;

    g_speakerResource = SpeakerResource("/rpi/speaker");

    if(g_speakerResource.createResource() < 0)
    {
        std::cerr << "Unable to create speaker resource!" << std::endl;
        return -1;
    }

    g_configurationResource = ConfigurationResource::Ptr(ConfigurationResource::getInstance());
    g_configurationResource->bootstrap(bootstrapCallback);

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
