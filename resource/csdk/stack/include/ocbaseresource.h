/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** <OCBaseResource>.h
** <Base resource structure holding all the information required to create
** and manage a resource>
**
** Author: <Mark Povlsen>
** Date: 06/01/16
** -------------------------------------------------------------------------*/

#ifndef _OCBASERESOURCE_H
#define _OCBASERESOURCE_H


// include directives

// Do not remove the include below

#ifdef WITH_ARDUINO
  #define TAG "ArduinoServer"
  #include "Arduino.h"
  #ifdef ARDUINOWIFI
  // Arduino WiFi Shield
  #include <SPI.h>
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #else
  // Arduino Ethernet Shield
  #include <Ethernet2.h>
  #include <EthernetServer.h>
  #include <EthernetClient.h>
  #include <Dhcp.h>
  #include <Dns.h>
  #include <EthernetUdp2.h>
  //#include <Twitter.h>
  #include <util.h>
  #endif
#else
  #define TAG "ocbaseresource"
#endif

#include "logger.h"
#include "ocstack.h"
#include "ocpayload.h"
#include "ocresource.h"
#include "octypes.h"
//#include <vector>
/*
#ifdef __cplusplus
extern "C" {
#endif*/

/****************** MACRO *********************/
static inline bool VERIFY_MEMORY_ALLOCATION(void* object)
{
  if(object == NULL)
  {
    OIC_LOG(ERROR, TAG, "No memory!");
    return false;
  }
  return true;
}


typedef struct OCBaseResourceT OCBaseResourceT;

/**
  * 8 bit variable declaring a port type
  */
typedef enum IOPortType
{
    IN      = 0,
    OUT     = (1 << 0),
    INOUT   = (2 << 0)
} IOPortType;

/**
  * Structure defining which port to assign a value.
  */
typedef struct OCIOPort
{
    IOPortType type;

    uint8_t pin;
} OCIOPort;


/**
  * The application calls this callback, when a PUT request has been initiated. The user
  * has to manually set what how the vlaues are sent to the ports (PWM, PPM etc.):
  */
typedef void (*OCIOHandler)
(OCRepPayloadValue *attribute, OCIOPort *port, OCResourceHandle handle, bool *underObservation);


/**
  * BaseResource containing the necessary parameters for a resource.
  * Somehow similar to OCResource internal struct with minor changes.
  */
typedef struct OCBaseResourceT
{
    /** Points to the next resource. Used to forma  list of created resources */
    OCBaseResourceT *next;

    /** Handle to handle the resource current data to the connectivity layer */
    OCResourceHandle handle;

    /** Relative path on the device. */
    char *uri;

    /** Human friendly name */
    char* name;

    /** Resource type(s). Linked list */
    OCResourceType *type;

    /** Resource interface(s). Linked list */
    OCResourceInterface *interface;

    /** Resource attributes. Linked list */
    OCRepPayloadValue *attribute;

    /** Resource Properties */
    uint8_t resourceProperties;

    /** Bool indicating if it is being observed */
    bool underObservation;

    /** Callback called when a PUT method has been called */
    OCIOHandler OCIOhandler;

    /** I/O Port associated with the resource */
    OCIOPort* port;

} OCBaseResourceT;

/*********************** REVISED VERSION 1.1 **************************/

/**
  * @brief Initializes and creates a resource
  *
  * @param uri          Path and name of the resource
  * @param interface    Resource interfaces
  * @param type         Resource types
  * @param properties   Byte of allowed properties of the resources
  * @param outputHandler Callback called when a PUT request is received
  *
  * @return Pointer to the created resource
  */
OCBaseResourceT * createResource(char* uri, OCResourceType* type, OCResourceInterface* interface,
                         uint8_t properties, OCIOHandler outputHandler, OCIOPort* port);


/**
  * @brief createResource
  *
  * @param uri           Path and name of the resource
  * @param type          Resource interface
  * @param interface     Resource type
  * @param properties    Allowed properties of the resource
  * @param outputHandler Callback called when a PUT request is received
  *
  * @return
  */
OCBaseResourceT * createResource(char* uri, const char* type, const char* interface, uint8_t properties,
                                 OCIOHandler outputHandler, OCIOPort* port);

/**
 * @brief Initializes and creates a resource
 *
 * @param resource      Resource to be initialized
 */
void createResource(OCBaseResourceT *resource);


/**
 * @brief addType Adds and bind a type to a resource
 *
 * @param handle        The handle of the resource
 * @param type          Type to be bound and added
 *
 * @return              OC_STACK_OK if successfully bound
 */
OCStackResult addType(OCBaseResourceT *resource, OCResourceType *type);
OCStackResult addType(OCBaseResourceT *resource, const char *typeName);

/**
 * @brief addInterface Adds and bind a interface to a resource
 *
 * @param handle        The handle of the resource
 * @param interface     Type to be bound and added
 *
 * @return              OC_STACK_OK if successfully bound
 */
OCStackResult addInterface(OCBaseResourceT *resource, OCResourceInterface *interface);
OCStackResult addInterface(OCBaseResourceT *resource, const char* interfaceName);

/**
 * @brief addAttribute  Adds an attribute to the resource
 *
 * @param resource      The resource to add an attribute to
 * @param attribute     The attribute to be added
 */
OCStackResult addAttribute(OCRepPayloadValue **head, OCRepPayloadValue *attribute);
//OCStackResult addAttribute(OCRepPayloadValue **head, OCRepPayloadValue *value);

/**
 * @brief getResourceList Returns the list of registered devices
 *
 * @return The list of registered devices
 */
OCBaseResourceT * getResourceList();

/*
#ifdef __cplusplus
}
#endif*/

#endif /* _OCBASERESOURCE_H */

