#include "resource_types.h"
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <unistd.h>
#include "oic_string.h"

#include "OCPlatform.h"
#include "OCApi.h"
#include <iostream>

#include "rd_client.h"

using namespace OC;

OCResourceHandle g_lightResource;
char g_rdAddress[MAX_ADDR_STR_SIZE];
uint16_t g_rdPort;

int g_quitFlag = false;

void handleSigInt(int signum)
{
    if(signum == SIGINT)
    {
        g_quitFlag = true;
    }
}

void registerLocalResources()
{
    std::string resourceURI_light = "/a/light";
    std::string resourceTypeName_light = "oic.d.light";
    std::string resourceInterface = OC_RSRVD_INTERFACE_DEFAULT;
    uint8_t resourceProperty = OC_DISCOVERABLE;

    OCStackResult result = OC::OCPlatform::registerResource(g_lightResource,
                                          resourceURI_light,
                                          resourceTypeName_light,
                                          resourceInterface,
                                          NULL,
                                          resourceProperty);

    if (OC_STACK_OK != result)
    {
        throw std::runtime_error(
            std::string("Device Resource failed to start") + std::to_string(result));
    }
}

int biasFactorCB(char addr[MAX_ADDR_STR_SIZE], uint16_t port)
{
    OICStrcpy(g_rdAddress, MAX_ADDR_STR_SIZE, addr);
    g_rdPort = port;
    std::cout << "RD Address is : " <<  addr << ":" << port << std::endl;
    return 0;
}
/*
int main()
{
    int in;
    PlatformConfig cfg;

    OCPlatform::Configure(cfg);

    std::cout << "Created Platform..." << std::endl;

    try
    {
        registerLocalResources();
    }
    catch (std::runtime_error e)
    {
        std::cout << "Caught OCException [Code: " << e.what() << std::endl;
    }

    while (1)
    {
        sleep(2);

        if (g_lightResource != NULL)
        {
            continue;
        }

        in = 0;
        std::cin >> in;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input type, please try again" << std::endl;
            continue;
        }

        try
        {
            switch ((int)in)
            {
                case 1:
                    OCRDDiscover(biasFactorCB);
                    break;
                case 2:
                    OCRDPublish(g_rdAddress, g_rdPort, 1, g_lightResource);
                    break;
                case 3:
                    break;
                default:
                    std::cout << "Invalid input, please try again" << std::endl;
                    break;
            }
        }
        catch (OCException e)
        {
            std::cout << "Caught OCException [Code: " << e.code() << " Reason: " << e.reason() << std::endl;
        }
    }
    return 0;
}*/


int main()
{
    std:cout << "Starting platform setup" << std::endl;

    OC::PlatformConfig cfg;

    OC::OCPlatform::Configure(cfg);

    std::cout << "Setup completed" << std::endl;
    signal(SIGINT, handleSigInt);

    while(OCRDDiscover(biasFactorCB) != OC_STACK_OK);

    // RD discovered. Publisher resources
    std::cout << "Creating resource" << std::endl;
    registerLocalResources();
    OCRDPublish(g_rdAddress, g_rdPort, 1, g_lightResource);

    if(OCStartPresence(60 * 60) != OC_STACK_OK)
    {
        //OIC_LOG(ERROR, TAG, "Unable to start presence server");
    }

    while(!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            return 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

}
