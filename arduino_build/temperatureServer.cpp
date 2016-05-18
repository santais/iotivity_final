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

static const int DELAY_TIME_INPUT_THREAD = 100;      // ms

void temperatureIOHandler(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation);
void checkInputThread();



// Blinking LED
static const char LED_PIN = 13;
static const char TEMPERATURE_PIN_IN = A1;
static const float TEMPERATURE_CONSTANT = 9.31;

double tempPrevValue = 0;

#define TAG "ArduinoServer"

/*#ifdef ARDUINOWIFI
// Arduino WiFi Shield
// Note : Arduino WiFi Shield currently does NOT support multicast and therefore
// this server will NOT be listening on 224.0.1.187 multicast address.

static const char ARDUINO_WIFI_SHIELD_UDP_FW_VER[] = "1.1.0";

/// WiFi Shield firmware with Intel patches
static const char INTEL_WIFI_SHIELD_FW_VER[] = "1.2.0";

/// WiFi network info and credentials
char ssid[] = "mDNSAP";
char pass[] = "letmein9";

int ConnectToNetwork()
{
    char *fwVersion;
    int status = WL_IDLE_STATUS;
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
    while (status != WL_CONNECTED)
    {
        OIC_LOG_V(INFO, TAG, "Attempting to connect to SSID: %s", ssid);
        status = WiFi.begin(ssid,pass);

        // wait 10 seconds for connection:
        delay(10000);
    }
    OIC_LOG(DEBUG, TAG, ("Connected to wifi"));

    IPAddress ip = WiFi.localIP();
    OIC_LOG_V(INFO, TAG, "IP Address:  %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return 0;
}
#else*/
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
//#endif //ARDUINOWIFI

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


void checkInputThread()
{
    //OIC_LOG(DEBUG, TAG, "Checking input thread");

    // Search through added resources
    OCBaseResourceT *current = getResourceList();

    while(current != NULL)
    {
        if(current->port->type == IN)
        {
            //OIC_LOG_V(DEBUG, TAG, "Found resource with name: %s", current->name);
            //OIC_LOG_V(DEBUG, TAG, "checkInputThread Observation: %s", current->underObservation ? "true" : "false");
            current->OCIOhandler(current->attribute, current->port, current->handle, &current->underObservation);
        }
        current = current->next;
    }
}

/*void printAttribute(OCAttributeT *attributes)
{
    OIC_LOG(DEBUG, TAG, "Attributes :");
    OCAttributeT *current = attributes;
    while(current != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Name: %s", current->name);
        switch(current->value.dataType)
        {
        case INT:
            OIC_LOG_V(DEBUG, TAG, "Value int: %i", current->value.data.i);
            //OIC_LOG_V(DEBUG, TAG, "Value: %i", *((int*)current->value.data));
            break;
        case DOUBLE:
            Serial.print("Value double: ");
            Serial.print(current->value.data.d);
            Serial.print("\n");

            //OIC_LOG_V(DEBUG, TAG, "Value double: %f", (float) current->value.data.d);
            //OIC_LOG_V(DEBUG, TAG, "Value: %f", *((double*)current->value.data));
            break;
            break;
        case BOOL:
            OIC_LOG_V(DEBUG, TAG, "Value bool: %s", current->value.data.b ? "true" : "false");
            bool boolean = *((bool*) current->value.data);
            OIC_LOG_V(DEBUG, TAG, "Value: %s", boolean ? "true" : "false");
            break;
        case STRING:
            OIC_LOG_V(DEBUG, TAG, "Value string: %s", current->value.data.str);
            //OIC_LOG_V(DEBUG, TAG, "Value: %s", *((char**)current->value.data));
            break;
        }
        current = current->next;
    }
    OIC_LOG(DEBUG, TAG, "Done printing attributes!");
}*/

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

    //printAttribute(resource->attribute);
    OIC_LOG(DEBUG, TAG, "=============================");
}

void temperatureIOHandler(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation)
{
    // Read the ADC value
    if(port != NULL)
    {
        int reading = analogRead(port->pin);
        double temperature = reading / TEMPERATURE_CONSTANT;

        dtostrf(temperature, 5, 2, attribute->str);
        //Serial.println(attribute->str);
    }
}



//The setup function is called once at startup of the sketch
void setup()
{
    // Add your initialization code here
    // Note : This will initialize Serial port on Arduino at 115200 bauds
    OIC_LOG_INIT();
    OIC_LOG(DEBUG, TAG, ("OCServer is starting..."));

    OCStackResult result = OC_STACK_ERROR;


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
        OIC_LOG(ERROR, TAG, ("OCStack init error"));
        return;
    }

    // DEBUG PIN
    pinMode(LED_PIN, OUTPUT);

    OCIOPort port;
    port.pin = TEMPERATURE_PIN_IN;
    port.type = IN;

    // Temperature resource
    OCBaseResourceT *temperatureResource = createResource("/a/temperatureSensor", OIC_DEVICE_SENSOR, OC_RSRVD_INTERFACE_DEFAULT,
                                                      (OC_DISCOVERABLE), temperatureIOHandler, &port);

    if(temperatureResource != NULL)
    {
        OIC_LOG(DEBUG, TAG, "Temperature successfully created");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "Error in creating the temperature sensor");
        return;
    }

    temperatureResource->name = "LM35 Temperature Sensor";

    result = addType(temperatureResource, OIC_TYPE_TEMPERATURE);

    // READ only interface
    result = addInterface(temperatureResource, OC_RSRVD_INTERFACE_READ);

    // Setup ADC
    analogReference(INTERNAL1V1);

    OCRepPayloadValue value;
    value.name = "Temperature";
    value.next = NULL;
    value.str = "00.00";
    value.type = OCREP_PROP_STRING;
    result = addAttribute(&temperatureResource->attribute, &value);

    result = OCStartPresence(OC_MAX_PRESENCE_TTL_SECONDS - 1);

    if(result != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "Failed to initialize resources");
        return;
    } else
    {
        OIC_LOG(INFO, TAG, "Initialization successful");
    }

    //printResource(temperatureResource);
}

// The loop function is called in an endless loop
void loop()
{
    // This artificial delay is kept here to avoid endless spinning
    // of Arduino microcontroller. Modify it as per specific application needs.
    delay(DELAY_TIME_INPUT_THREAD);
    checkInputThread();
    PrintArduinoMemoryStats();

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
