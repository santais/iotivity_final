//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#include "ocbaseresource.h"
#include "resource_types.h"
#include "easysetup.h"

static const int DELAY_TIME_INPUT_THREAD = 1000;      // ms
//static const int DELAY_RD_DISCOVERY = 3000;

// Blinking LED
static const char LED_PIN = 13;
static const char TEST_LED_PIN = 5; // PWM Pin

// RD variables
//static char g_rdAddress[MAX_ADDR_STR_SIZE];
//static uint16_t g_rdPort;

// Light Resource
OCBaseResourceT *g_lightResource;
OCBaseResourceT *g_buttonResource;

/**
 * @var g_OnBoardingSucceeded
 * @brief This variable will be set if OnBoarding is successful
 */
static bool g_OnBoardingSucceeded = false;

/**
 * @var g_ProvisioningSucceeded
 * @brief This variable will be set if Provisioning is successful
 */
static bool g_ProvisioningSucceeded = false;

/**
 * @var g_rdInitialized
 * @brief is set once a RD has been discovered
 */
//static bool g_rdInitialized = false;


#define TAG "ArduinoServer"

// Functions
void EventCallbackInApp(ESResult esResult, ESEnrolleeState enrolleeState);
ESResult StartEasySetup();
void ESInitResources();
void createLightResource();

// Arduino WiFi Shield
// Note : Arduino WiFi Shield currently does NOT support multicast and therefore
// this server will NOT be listening on 224.0.1.187 multicast address.

/// WiFi network info and credentials
char g_ssid[] = "EasySetup123";
char g_pass[] = "EasySetup123";


void lightIOHandler(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation)
{
    if(port->type == OUTPUT)
    {
        bool power(false);
        int brightness(0);
       // OIC_LOG(DEBUG, TAG, "LightIOHandler: OUTPUT");
        OCRepPayloadValue *current = attribute;
        while(current != NULL)
        {
            //OIC_LOG_V(DEBUG, TAG, "Attribute name: %s", current->name);
            //OIC_LOG(DEBUG, TAG, "Searching light");
            if(strcmp(current->name, "power") == 0)
            {
                power = current->b;
            }
            else if (strcmp(current->name, "brightness") == 0)
            {
                brightness = current->i;
            }

            current = current->next;
        }

        if(power)
        {
            analogWrite(port->pin, brightness);
        }
        else
        {
            analogWrite(port->pin, 0);
        }

        if(*underObservation)
        {
            OIC_LOG(DEBUG, TAG, "LIGHT: Notifying observers");
            if(OCNotifyAllObservers(handle, OC_LOW_QOS) == OC_STACK_NO_OBSERVERS)
            {
                OIC_LOG(DEBUG, TAG, "No more observers!");
                *underObservation = false;
            }
        }
    }
}

void EventCallbackInApp(ESResult esResult, ESEnrolleeState enrolleeState)
{
    Serial.println("callback!!! in app");

    if(esResult == ES_OK)
    {
        if(!g_OnBoardingSucceeded){
            Serial.println("Device is successfully OnBoarded");
            g_OnBoardingSucceeded = true;
        }
        else if(g_OnBoardingSucceeded & enrolleeState == ES_ON_BOARDED_STATE){
            Serial.println("Device is successfully OnBoared with SoftAP");
            g_ProvisioningSucceeded = true;
        }

        if(enrolleeState == ES_PROVISIONED_STATE)
        {
            // Create the Light resource.
            createLightResource();
        }
    }
    else if (esResult == ES_ERROR)
    {
        if(g_OnBoardingSucceeded)
        {
            OIC_LOG_V(ERROR, TAG, "Failure in Provisioning. \
                                        Current Enrollee State: %d",enrolleeState);
            g_OnBoardingSucceeded = false;
        }
        else if(g_ProvisioningSucceeded)
        {
            OIC_LOG_V(ERROR, TAG, "Failure in connect to target network. \
                                        Current Enrollee State: %d",enrolleeState);
            g_ProvisioningSucceeded = false;
        }
    }
}

void startProvisioning()
{
    OIC_LOG(DEBUG, TAG, "ESInitResources is invoked...");

    delay(2000);
    // Initialize the OC Stack in Server mode
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "OCStack init error!!");
        return;
    }

    if (ESInitProvisioning() == ES_ERROR)
    {
        OIC_LOG(ERROR, TAG, "Init Provisioning Failed!!");
        return;
    }
}

ESResult StartEasySetup()
{
    OIC_LOG(DEBUG, TAG, "OCServer is starting...");

    //ESInitEnrollee with sercurity mode disabled for arduino
    if(ESInitEnrollee(CT_ADAPTER_IP, g_ssid, g_pass, false, EventCallbackInApp) == ES_ERROR)
    {
        OIC_LOG(ERROR, TAG, "OnBoarding Failed");
        return ES_ERROR;
    }

    OIC_LOG_V(ERROR, TAG, "OnBoarding succeded. Successfully connected to ssid : %s",ssid);
    return ES_OK;
}

void createLightResource()
{
    // DEBUG PIN
    pinMode(LED_PIN, OUTPUT);

    OIC_LOG(DEBUG, TAG, "Creating resource");

    OCIOPort portLight;
    portLight.pin = TEST_LED_PIN; // LED_PIN for debug
    portLight.type = OUT;

    // Light resource
    g_lightResource = createResource("/a/light", OIC_DEVICE_LIGHT, OC_RSRVD_INTERFACE_DEFAULT,
                                              (OC_DISCOVERABLE | OC_OBSERVABLE), lightIOHandler, &portLight);

    if(g_lightResource != NULL)
    {
        OIC_LOG(INFO, TAG, "Light resource created successfully");
        Serial.println((int)g_lightResource->handle, HEX);
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "Unable to create light resource");
    }
    g_lightResource->name = "Mark's Light";

    addType(g_lightResource, OIC_TYPE_BINARY_SWITCH);
    addType(g_lightResource, OIC_TYPE_LIGHT_BRIGHTNESS);

    OCRepPayloadValue powerValue;
    powerValue.b = true;
    powerValue.name = "power";
    powerValue.next = NULL;
    powerValue.type = OCREP_PROP_BOOL;
    addAttribute(&g_lightResource->attribute, &powerValue);

    OCRepPayloadValue brightnessValue;
    brightnessValue.i = 255;
    brightnessValue.name = "brightness";
    brightnessValue.next = NULL;
    brightnessValue.type = OCREP_PROP_INT;
    addAttribute(&g_lightResource->attribute, &brightnessValue);

    //printResource(resourceLight);

    // Start presence of the resource
    if(OCStartPresence(OC_MAX_PRESENCE_TTL_SECONDS - 1) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "Unable to start presence server");
    }

    OIC_LOG(DEBUG, TAG, "Finished setup");
}

// Not yet implemented for arduino
/*
int rdDiscoverCallback(char addr[MAX_ADDR_STR_SIZE], uint16_t port)
{
    OIC_LOG_V(DEBUG, TAG, "RD Address is: %s : %i", address, port);

    g_rdAddress = addr;
    g_rdPort = port;

    // Anounce to the API that a RD has been found
    g_rdInitialized = true;

    return 0;
}*/


//The setup function is called once at startup of the sketch
void setup()
{
    // Add your initialization code here
    // Note : This will initialize Serial port on Arduino at 115200 bauds
   	OIC_LOG_INIT();
    OIC_LOG(DEBUG, TAG, ("OCServer is starting..."));

    delay(1000);
    // Start the onboarding process
    while(StartEasySetup() != ES_OK);

    // Start provisioning
    startProvisioning();

}

// The loop function is caplled in an endless loop
void loop()
{
    // This artificial delay is kept here to avoid endless spinning
    // of Arduino microcontroller. Modify it as per specific application needs.
    delay(DELAY_TIME_INPUT_THREAD);
    //checkInputThread();

    // This call displays the amount of free SRAM available on Arduino
    //PrintArduinoMemoryStats();

    // Give CPU cycles to OCStack to perform send/recv and other OCStack stuff
    if (OCProcess() != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, ("OCStack process error"));
        return;
    }

    //yield();
}
