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
#include <stdlib.h>
#include <string>

static const int DELAY_TIME_INPUT_THREAD = 1;      // ms

// Blinking LED
static const char LED_PIN = 13;
static const char TEST_LED_PIN = 5; // PWM Pin
static const char TEST_BUT_PIN = 2;	
static const char TEMPERATURE_PIN_IN = A2;

static const float TEMPERATURE_CONSTANT = 0.08;
static const float TEMPERATURE_DIFFERENCE = 0.4;
static float g_prevTempReading;

static int g_prevButtonReading = false;
#define TAG "ArduinoServer"

// Resources
OCBaseResourceT* g_lightResource;
OCBaseResourceT* g_buttonResource;
OCBaseResourceT* g_temperatureResource;

#ifdef ARDUINOWIFI
// Arduino WiFi Shield
// Note : Arduino WiFi Shield currently does NOT support multicast and therefore
// this server will NOT be listening on 224.0.1.187 multicast address.

static const char ARDUINO_WIFI_SHIELD_UDP_FW_VER[] = "1.1.0";
    
/// WiFi Shield firmware with Intel patches
static const char INTEL_WIFI_SHIELD_FW_VER[] = "1.2.0";

/// WiFi network info and credentials
char ssid[] = "EasySetup123";
char pass[] = "EasySetup123";

int ConnectToNetwork()
{
    char *fwVersion;
    int status = WL_IDLE_STATUS;
    pinMode(9, OUTPUT);      // set the LED pin mode
    delay(1000);
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD)
    {
        OIC_LOG(ERROR, TAG, ("WiFi shield not present"));
        return -1;
    }

    // Verify that WiFi Shield is running the firmware with all UDP fixes
    fwVersion = WiFi.firmwareVersion();
    OIC_LOG_V(INFO, TAG, "WiFi Shield Firmware version %s", fwVersion);
    if ( strncmp(fwVersion, ARDUINO_WIFI_SHIELD_UDP_FW_VER, sizeof(ARDUINO_WIFI_SHIELD_UDP_FW_VER)) !=0 )
    {
        OIC_LOG(DEBUG, TAG, ("!!!!! Upgrade WiFi Shield Firmware version !!!!!!"));
        return -1;
    }

    // attempt to connect to Wifi network:
    OIC_LOG(DEBUG, TAG, "Connecting...");
    while (status != WL_CONNECTED)
    {
        OIC_LOG_V(INFO, TAG, "Attempting to connect to SSID: %s", ssid);
        status = WiFi.begin(ssid,pass);

        // wait 10 seconds for connection:
        delay(10000);
        OIC_LOG(DEBUG, TAG, "Retrying...");
    }
    OIC_LOG(DEBUG, TAG, ("Connected to wifi"));

    IPAddress ip = WiFi.localIP();
    OIC_LOG_V(INFO, TAG, "IP Address:  %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return 0;
}
#else
// Arduino Ethernet Shield
int ConnectToNetwork()
{
    // Note: ****Update the MAC address here with your shield's MAC address****
    uint8_t ETHERNET_MAC[] = {0x90, 0xA2, 0xDA, 0x10, 0x29, 0xE2}; 
    uint8_t error = Ethernet.begin(ETHERNET_MAC);
    if (error  == 0)
    {
        OIC_LOG_V(ERROR, TAG, "error is: %d", error);
        return -1;
    }

    IPAddress ip = Ethernet.localIP();
    OIC_LOG_V(INFO, TAG, "IP Address:  %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return 0;
}
#endif //ARDUINOWIFI

// On Arduino Atmel boards with Harvard memory architecture, the stack grows
// downwards from the top and the heap grows upwards. This method will print
// the distance(in terms of bytes) between those two.
// See here for more details :
// http://www.atmel.com/webdoc/AVRLibcReferenceManual/malloc_1malloc_intro.html
void PrintArduinoMemoryStats()
{
    //#ifdef ARDUINO_AVR_MEGA2560
    //This var is declared in avr-libc/stdlib/malloc.c
    //It keeps the largest address not allocated for heap
    extern char *__brkval;
    //address of tmp gives us the current stack boundry
    int tmp;
    OIC_LOG_V(INFO, TAG, "Stack: %u         Heap: %u", (unsigned int)&tmp, (unsigned int)__brkval);
    OIC_LOG_V(INFO, TAG, "Unallocated Memory between heap and stack: %u",
            ((unsigned int)&tmp - (unsigned int)__brkval));
   // #endif
}


void printAttribute(OCRepPayloadValue *attributes)
{
    OIC_LOG(DEBUG, TAG, "Attributes :");
    OCRepPayloadValue *current = attributes;
    while(current != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Name: %s", current->name);
        switch(current->type)
        {
        case OCREP_PROP_INT:
            OIC_LOG_V(DEBUG, TAG, "Value: %i", current->i);
            //OIC_LOG_V(DEBUG, TAG, "Value: %i", *((int*)current->value.data));
            break;
        case OCREP_PROP_DOUBLE:
            OIC_LOG_V(DEBUG, TAG, "Value: %f", current->d);
            //OIC_LOG_V(DEBUG, TAG, "Value: %f", *((double*)current->value.data));
            break;
            break;
        case OCREP_PROP_BOOL:
            OIC_LOG_V(DEBUG, TAG, "Value: %s", current->b ? "true" : "false");
           /* bool boolean = *((bool*) current->value.data);
            OIC_LOG_V(DEBUG, TAG, "Value: %s", boolean ? "true" : "false");*/
            break;
        case OCREP_PROP_STRING:
            OIC_LOG_V(DEBUG, TAG, "Value: %s", current->str);
            //OIC_LOG_V(DEBUG, TAG, "Value: %s", *((char**)current->value.data));
            break;
        }
        current = current->next;
    }
    OIC_LOG(DEBUG, TAG, "Done printing attributes!");
}

void printResource(OCBaseResourceT *resource)
{
    OIC_LOG(DEBUG, TAG, "=============================");
    OIC_LOG_V(DEBUG, TAG, "Resource URI: %s", resource->uri);
    OIC_LOG_V(DEBUG, TAG, "Handle of the resource: %p", (void*) resource->handle);

    OIC_LOG(DEBUG, TAG, "Resource Types: ");
    OCResourceType *currentType = resource->type;
    while(currentType != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "\t%s", currentType->resourcetypename);
        currentType = currentType->next;
    }

    OIC_LOG(DEBUG, TAG, "Resource Interfaces: ");
    OCResourceInterface *currentInterface = resource->interface;
    while(currentInterface != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "\t%s", currentInterface->name);
        currentInterface = currentInterface->next;
    }

    printAttribute(resource->attribute);
    OIC_LOG(DEBUG, TAG, "=============================");
}


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


void temperatureIOHandler(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation)
{
    // Read the ADC value
    if(port != NULL)
    {   
        int reading = analogRead(port->pin);
        attribute->d = reading * TEMPERATURE_CONSTANT;

        if(attribute->d >= g_prevTempReading + TEMPERATURE_DIFFERENCE || attribute->d <= g_prevTempReading - TEMPERATURE_DIFFERENCE)
        {
            if(*underObservation)
            {
                Serial.print("Temperature is: ");
                Serial.println(attribute->d);
                OIC_LOG(DEBUG, TAG, "Notifying temperature");
                OCNotifyAllObservers(handle, OC_MEDIUM_QOS);
            }

            g_prevTempReading = attribute->d;
        }
    }
}

void buttonIOHandler(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation)
{
    int reading = digitalRead(port->pin);

    // Debounce
    if(reading > 0) {
        delay(50);
        reading = digitalRead(port->pin);
    }
    if(*underObservation && (g_prevButtonReading != reading))
    {
        OIC_LOG(DEBUG, TAG, "Notifying observers");
        attribute->b = reading;
        Serial.println((int)handle, HEX);
        if(OCNotifyAllObservers(handle, OC_MEDIUM_QOS) == OC_STACK_NO_OBSERVERS)
        {
            OIC_LOG(DEBUG, TAG, "No more observers!");
            *underObservation = false;
        }
        g_prevButtonReading = reading;
    }

    
}

void checkInputThread()
{
        //OIC_LOG(DEBUG, TAG, "Checking input thread");

    // Search through added resources
    OCBaseResourceT *current = getResourceList();

    while(current != NULL)
    {
        if(current->port->type == INPUT)
        {
            //OIC_LOG_V(DEBUG, TAG, "Found resource with name: %s", current->name);
            //OIC_LOG_V(DEBUG, TAG, "checkInputThread Observation: %s", current->underObservation ? "true" : "false");
            current->OCIOhandler(current->attribute, current->port, current->handle, &current->underObservation);
        }
        current = current->next;
    }
}

void createTemperature()
{
    OCIOPort port;
    port.pin = TEMPERATURE_PIN_IN;
    port.type = IN;

    // Temperature resource
    g_temperatureResource = createResource1("/arduino/temperatureSensor/hosting", OIC_DEVICE_SENSOR, OC_RSRVD_INTERFACE_DEFAULT,
                                                      (OC_DISCOVERABLE | OC_OBSERVABLE), temperatureIOHandler, &port);
    analogReference(AR_DEFAULT);
    if(g_temperatureResource != NULL)
    {
        OIC_LOG(DEBUG, TAG, "Temperature successfully created");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "Error in creating the temperature sensor");
        return;
    }

    g_temperatureResource->name = "LM35 Temperature Sensor";

    OCStackResult result = addType1(g_temperatureResource, OIC_TYPE_TEMPERATURE);
    result = addType1(g_temperatureResource, OIC_TYPE_RESOURCE_HOST);

    // READ only interface
    result = addInterface1(g_temperatureResource, OC_RSRVD_INTERFACE_READ);

    OCRepPayloadValue value;
    value.name = "Temperature";
    value.next = NULL;
    value.d = 0.0;
    value.type = OCREP_PROP_DOUBLE;
    result = addAttribute(&g_temperatureResource->attribute, &value);

    analogReadResolution(12);
}

void createLightResource()
{
    OCIOPort portLight;
    portLight.pin = TEST_LED_PIN; // LED_PIN for debug
    portLight.type = OUT;

    // Light resource
    g_lightResource = createResource1("/arduino/light", OIC_DEVICE_LIGHT, OC_RSRVD_INTERFACE_DEFAULT,
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

    addType1(g_lightResource, OIC_TYPE_BINARY_SWITCH);
    addType1(g_lightResource, OIC_TYPE_LIGHT_BRIGHTNESS);

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

}

void createButtonResource()
{
   /** Create Button Resource **/
    OCIOPort buttonPort;
    buttonPort.pin = TEST_BUT_PIN;
    buttonPort.type = IN;

    g_buttonResource = createResource1("/arduino/button", OIC_DEVICE_BUTTON, OC_RSRVD_INTERFACE_DEFAULT,
                                            (OC_DISCOVERABLE | OC_OBSERVABLE), buttonIOHandler, &buttonPort);

    if(g_buttonResource != NULL)
    {
        OIC_LOG(INFO, TAG, "Button resource created successfully");
        int pointer;
        Serial.println((int)g_buttonResource->handle, HEX);
    }
    else
    {
        OIC_LOG(ERROR, TAG, "Unable to create the button resource");
    }

    OCStackResult result = addType1(g_buttonResource, OIC_TYPE_BINARY_SWITCH);

    result = addInterface1(g_buttonResource, OC_RSRVD_INTERFACE_READ);

    if(result != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "Can't add interface READ");
    }

    OCRepPayloadValue buttonValue;
    buttonValue.b = false;
    buttonValue.name = "state";
    buttonValue.next = NULL;
    buttonValue.type = OCREP_PROP_BOOL;
    addAttribute(&g_buttonResource->attribute, &buttonValue);
}

//The setup function is called once at startup of the sketch
void setup()
{
    // Add your initialization code here
    // Note : This will initialize Serial port on Arduino at 115200 bauds
    OIC_LOG_INIT();
    OIC_LOG(DEBUG, TAG, ("OCServer is starting..."));


    // Connect to Ethernet or WiFi network
    if (ConnectToNetwork() != 0)
    {
        Serial.print("Unable to connect to Network");
        OIC_LOG(ERROR, TAG, ("Unable to connect to network"));
        return;
    }

    // Initialize the OC Stack in Server mode
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        Serial.println("OCStack Init Error");
        OIC_LOG(ERROR, TAG, ("OCStack init error"));
        return;
    }

    // DEBUG PIN
    pinMode(LED_PIN, OUTPUT);

    OIC_LOG(DEBUG, TAG, "Creating resource");

    createTemperature();
    createButtonResource();
    createLightResource();

   /* OCResourceHandle handle;
    OCStackResult res = OCCreateResource(&handle,
            "oic.d.light",
            OC_RSRVD_INTERFACE_DEFAULT,
            "/android/light",
            NULL,
            NULL,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OIC_LOG_V(INFO, TAG, "Created Light resource with result: %s", getResult(res));
    Serial.print("Created resource with OCStackREsult");
    Serial.println(res);*/
  /*  createLightResource();
    createButtonResource();*/

  /*a  if(OCStartPresence(OC_MAX_PRESENCE_TTL_SECONDS - 1) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "Unable to start presence server");
    }
*/


    OIC_LOG(DEBUG, TAG, "Finished setup");
}

// The loop function is caplled in an endless loop
void loop()
{
    // This artificial delay is kept here to avoid endless spinning
    // of Arduino microcontroller. Modify it as per specific application needs.
    //Serial.println("Alive");
    delay(DELAY_TIME_INPUT_THREAD);
    checkInputThread();

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
