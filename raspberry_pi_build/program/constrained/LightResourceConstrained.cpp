//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <iostream>

#include "OCPlatform.h"
#include "OCApi.h"
#include "oic_string.h"

#include "rd_client.h"
#include "easysetup.h"

#include "LightResource.h"

using namespace OC;

OCResourceHandle g_curResource_t = NULL;
OCResourceHandle g_curResource_l = NULL;
char rdAddress[MAX_ADDR_STR_SIZE];
uint16_t rdPort;

bool g_provisionInitialized = false;
bool g_rdInitialized = false;

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

/**
 * @brief LightResource
 */
LightResource g_lightResource;

int biasFactorCB(char addr[MAX_ADDR_STR_SIZE], uint16_t port)
{
    OICStrcpy(rdAddress, MAX_ADDR_STR_SIZE, addr);
    rdPort = port;
    std::cout << "RD Address is : " <<  addr << ":" << port << std::endl;
    g_rdInitialized = true;
    return 0;
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
            g_provisionInitialized = true;
        {
            printf("Device is onboarded/connected with target network\n");
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
}

int main()
{
    int in;
    PlatformConfig cfg;

    OCPlatform::Configure(cfg);

   // LightResource lightResource;
   // lightResource.createResource();
    if(lightResource.getHandle() == NULL) 
    {
        std::cout << "Light resource handle is null" << std::endl;
    }

    std::cout << "Created Platform..." << std::endl;

    while (1)
    {
        sleep(2);

        // Initialize easy setup
        if(startEasySetup() != ES_OK)
        {
            std::cout << "Error initialzing starteasy setup";
            sleep(2);
            return -1;
        }

        std::cout << "OnBoarding complete" << std::endl;
        ESInitResources();

        while(!g_provisionInitialized)
        {
            sleep(1);
        }
        std::cout << "Provision complete" << std::endl;

        if(!g_rdInitialized) 
        {
            try
            {
                while(!g_rdInitialized) 
                {
                    OCRDDiscover(biasFactorCB);
                    sleep(2);
                }

                OCRDPublish(rdAddress, rdPort, 1, g_lightResource.getHandle());

            }
            catch (OCException e)
            {
                std::cout << "Caught OCException [Code: " << e.code() << " Reason: " << e.reason() << std::endl;
            }
        }
    }
    return 0;
}
