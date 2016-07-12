#include "MaintenanceResource.h"
#include "PCA9685LEDResource.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>

static const int I2CADDRESS_BLUE_1 = 64;
static const int I2CADDRESS_BLUE_2 = 65;
static const int I2CADDRESS_BLUE_3 = 66;

static const int I2CADDRESS_RED_1 = 67;
static const int I2CADDRESS_RED_2 = 68;
static const int I2CADDRESS_RED_3 = 69;

static const int FREQUENCY = 500;

MaintenanceResource::Ptr g_maintenanceResource;

std::vector<PCA9685LEDResource::Ptr> g_PCA9685ResourcesBlue;
std::vector<PCA9685LEDResource::Ptr> g_PCA9685ResourcesRed;

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

/**
 * @brief Setup the blue Ping Pong Side
 */
void setupBlueSide()
{
    // 10 cups are to be initialized = 10 RPi resources.
    /*  Cups setup - ID */
    /* 9  8  7  6 */
    /*  5  4   3  */
    /*   2   1    */
    /*     0      */

    // A maximum of 5 cups are possible per PCA9865 Module.
    // Requires a total of 2 modules per side
    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcablue/led/" << i;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_BLUE_1, FREQUENCY);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        g_PCA9685ResourcesBlue.push_back(resource);
    }

    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcablue/led/" << i + 5;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_BLUE_2, FREQUENCY);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        g_PCA9685ResourcesBlue.push_back(resource);
    }

}

/**
 * @brief Setup the red Ping Pong Side.
 */
void setupRedSide()
{
    // 10 cups are to be initialized = 10 RPi resources.
    /*  Cups setup - ID */
    /* 9  8  7  6 */
    /*  5  4   3  */
    /*   2   1    */
    /*     0      */

    // A maximum of 5 cups are possible per PCA9865 Module.
    // Requires a total of 2 modules per side
    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcared/led/" << i;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_RED_1, FREQUENCY);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        g_PCA9685ResourcesBlue.push_back(resource);
    }

    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcared/led/" << i + 5;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_RED_2, FREQUENCY);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        g_PCA9685ResourcesBlue.push_back(resource);
    }

}

/**
 * @brief Initialize an RPi LED resource
 *
 * @param uri
 * @param pins
 * @param I2CAddress
 * @param frequency
 */
void initRPiResources()
{
    setupBlueSide();
    setupRedSide();
}


int main()
{
    std::cout << "Starting RPI LED Program" << std::endl;

    // Initialize the PCA9685 module
    PCA9685Setup(I2CADDRESS_BLUE_1, FREQUENCY);

    // Initialize all RPi Resources
    initRPiResources();

    g_maintenanceResource = MaintenanceResource::Ptr(MaintenanceResource::getInstance());
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
