#include "ConfigurationResource.h"
#include "MaintenanceResource.h"
#include "LightResource.h"
#include "ButtonResource.h"

#include <signal.h>
#include <unistd.h>

#include "easysetup.h"

#define GPIO_RPI_PIN_8 15
#define GPIO_RPI_PIN_10 16

ConfigurationResource::Ptr g_configurationResource;
MaintenanceResource::Ptr g_maintenanceResource;
LightResource g_lightResource;
ButtonResource g_buttonResource;

int g_quitFlag = false;

bool g_provisionInitialized = false;
bool g_rdInitialized = false;

void *listeningFunc(void *);

/**
 * @var ssid
 * @brief Target SSID of the Soft Access point to which the device has to connect
 */
static char g_ssid[] = "EasySetup123";

/**
 * @var passwd
 * @brief Password of the Soft Access point to which the device has to connect
 */
static char g_passwd[] = "EasySetup123";

void handleSigInt(int signum)
{
    if(signum == SIGINT)
    {
        g_quitFlag = true;
    }
}


void EventCallbackInApp(ESResult esResult, ESEnrolleeState enrolleeState)
{
    printf("Easy setup event callback\n");

    if(esResult == ES_OK)
    {
        if(enrolleeState == ES_ON_BOARDED_STATE)
        {
            printf("Device is successfully OnBoared on Adhoc network\n");
        }
        else if (enrolleeState == ES_PROVISIONED_STATE)
        {
            printf("Device is provisioned with target network's credentials\n");
            g_provisionInitialized = true;
        }
        else if (enrolleeState == ES_ON_BOARDED_TARGET_NETWORK_STATE)
        {
            printf("Device is onboarded/connected with target network\n");
        g_provisionInitialized = true;
        }
        else
        {
            printf("Wrong state !! Easy setup is failed at Enrollee state = %d\n",enrolleeState);
        }
    }
    else
    {
        printf("Easy stup is failed at Enrollee state = %d\n",enrolleeState);;
    }

}


ESResult startEasySetup()
{
    printf("StartEasySetup and onboarding started..\n");

    if(ESInitEnrollee(CT_ADAPTER_IP, g_ssid, g_passwd, false, EventCallbackInApp) == ES_ERROR)
    {
        printf("StartEasySetup and onboarding Fail!!\n");
        return ES_ERROR;
    }
    ES_OK;
}


void ESInitResources()
{
    std::cout << "Starting Enrollee Provisioning" << std::endl;

    if(OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        std::cerr << "OCSTACK init error!" << std::endl;
        return;
    }

    if(ESInitProvisioning() == ES_ERROR)
    {
        std::cerr << "Init provisioning failed" << std::endl;
        return;
    }

    pthread_t thread_handle;
    if(pthread_create(&thread_handle, NULL, listeningFunc, NULL))
    {
    std::cerr << "Error creating thread" << std::endl;
    }

    std::cout << "Provisiong Resource Init Complete" << std::endl;
}

void *listeningFunc(void* non)
{
    OCStackResult result;

    while(true)
    {
        result = OCProcess();
        if(result != OC_STACK_OK)
    {
         printf("OCSTACK ERROR\n");
    }
    }
    return NULL;
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
    std::cout << "Starting Light and Button Resources program" << std::endl;


    // Initialize easy setup
    if(startEasySetup() != ES_OK)
    {
        std::cout << "Error initialzing starteasy setup";
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return -1;
    }

    std::cout << "OnBoarding complete" << std::endl;
    ESInitResources();

    while(!g_provisionInitialized)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "Waiting on provisioning..." << std::endl;
    }
    std::cout << "Provision complete" << std::endl;

#ifdef ARM
    wiringPiSetup();
#endif
    g_lightResource = LightResource(GPIO_RPI_PIN_8, "/rpi/light/hosting");
    g_lightResource.setHostingResource();
    g_lightResource.createResource();

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
