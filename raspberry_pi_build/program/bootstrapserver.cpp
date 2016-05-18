#include "RPIRCSController.h"
#include "RPIRCSResourceObject.h"
#include "RCSRequest.h"

#include "ConfigurationResource.h"
#include "MaintenanceResource.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <unistd.h>

#include "resource_types.h"

#include "OCPlatform.h"
#include "OCApi.h"

static const std::string BOOTSTRAP_URI      = "/bootstrap";
static const std::string BOOTSTRAP_TYPE     = "bootstrap";
static const std::string BOOTSTRAP_INTERFACE = OC_RSRVD_INTERFACE_DEFAULT;

static const std::string DEFAULT_DEVICE_NAME        = "Schneider Legacy Device";
static const std::string DEFAULT_LOCATION           = "55.357594 10.294468";
static const std::string DEFAULT_LOCATION_NAME      = "Office Site A10";
static const std::string DEFAULT_CURRENCY           = "DKK";
static const std::string DEFAULT_REGION             = "Odense, Denmark";

RPIRCSResourceObject::Ptr g_bootstrapResource;

int g_quitFlag = false;

void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        g_quitFlag = 1;
    }
}

void createBootstrapServer() {
    g_bootstrapResource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(BOOTSTRAP_URI, std::move(std::vector<std::string>{BOOTSTRAP_TYPE}), std::move(std::vector<std::string>{BOOTSTRAP_INTERFACE})));

    // Create the resources
    g_bootstrapResource->createResource(true, true, false);

    RCSResourceAttributes attributes;
    attributes[CONFIGURATION_DEVICE_NAME]    = DEFAULT_DEVICE_NAME;
    attributes[CONFIGURATION_LOCATION]       = DEFAULT_LOCATION;
    attributes[CONFIGURATION_LOCATION_NAME]  = DEFAULT_LOCATION_NAME;
    attributes[CONFIGURATION_CURRENCY]       = DEFAULT_CURRENCY;
    attributes[CONFIGURATION_REGION]         = DEFAULT_REGION;

    g_bootstrapResource->setAttributes(std::move(attributes));

    std::cout << "Bootstrap server resource created" << std::endl;
}

int main() {
    std::cout << "Starting program..." << std::endl;
    createBootstrapServer();

    signal(SIGINT, handleSigInt);

    while(!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            return 0;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
