/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** <OCBaseResource>.c
** <Base resource structure holding all the information required to create
** and manage a resource>
**
** Author: <Mark Povlsen>
** Date: 06/01/16
** -------------------------------------------------------------------------*/

#include "ocbaseresource.h"

/*********** CONSTANT VARIABLES ****************/
static const int URI_MAXSIZE = 19;
static const int MAX_NUM_OF_RESOURCES = 10;
static const int MAXIMUM_OBSERVERS = 2;

/*********** PRIVATE VARIABLES ****************/
static OCBaseResourceT *m_resourceList = NULL;
static int numOfResources = 0;
static int numOfObservers = 0;

/*********************** REVISED VERSION 1.1 **************************/

/**********************************************************************/
/* PRIVATE FUCNTIONS
/**********************************************************************/

/**
  * @brief Entity handler handling all incoming requests
  *
  * @param flag Entityhandler   flag (Request or Observer)
  * @param entityHandlerRequest contains the payload and other data types
  * @param callbackParam        The requested resource
  *
  * @return The result of the resource request handling.
  */
OCEntityHandlerResult EntityHandlerCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest,
                            void *callbackParam);
/**
 * @brief printResourceData
 *
 * @param resource
 */
void printResourceData(OCBaseResourceT *resource);

/**
 * @brief printAttributes Print a list of the attributes
 *
 * @param attributes    Linked list of attributes
 */
void printAttributes(OCRepPayloadValue *attributes);

/**
 * @brief Create and loads the payload
 *
 * @return the created payload
 */
OCRepPayload *getPayload(OCEntityHandlerRequest *ehRequest, OCBaseResourceT *resource);

 /**
  * @brief handles the response to the entity handler
  *
  * @param response   The entityhandler response
  * @param EntityHandlerRequest
  * @param resource   The base resource attributes
  *
  * @parma result of the entityhandler;
  */
OCEntityHandlerResult responseHandler(OCEntityHandlerResponse *response, OCEntityHandlerRequest *entityHandlerRequest, OCRepPayload *payload, OCEntityHandlerResult ehResult);

 /**
  * @brief Handles what request was instantiated and the corrensponding action
  *
  * @param handler  The EntityHandler
  * @param resource   Base resource
  *
  * @return the result of the request
  */
OCEntityHandlerResult requestHandler(OCEntityHandlerRequest *ehRequest,
                                      OCBaseResourceT *resource, OCRepPayload **payload);

/**
 * @brief observerHandler
 *
 * @param ehRequest     Request information from the client
 * @param resource      Pointer to the request resource
 */
OCEntityHandlerResult observerHandler(OCEntityHandlerRequest *ehRequest, OCBaseResourceT *resource);

/**
 * @brief Called when a REST GET is request
 *
 * @param OCBaseResource base resource attributes
 *
 * @return result of the entityHandler
 */
 OCEntityHandlerResult getRequest(OCBaseResourceT *resource, OCRepPayload *payload);

 /**
 * @brief Called when a REST PUT is request
 *
 * @param OCBaseResource base resource attributes
 *
 * @return result of the entityHandler
 */
 OCEntityHandlerResult putRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload* payload, OCBaseResourceT *resource);


 /**
  * @brief postRequest Called when a RESTful POST request is called
  *
  * @param ehRequest    Request parameters
  * @param payload      Payload from the client
  * @param resource     Resource the call was refered to
  * @return
  */
 OCEntityHandlerResult postRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload* payload, OCBaseResourceT *resource);

 /**
   * @brief Search for a resource in the resource list
   *
   * @param handle     Handle of the resource
   *
   * @return true if found
   */
 bool findResource(OCResourceHandle handle);


 /**
  * @brief Returns the result of a OCStackResult as a string
  *
  * @param OCStackResult The result to be converted to a string
  *
  * @return A string with the result
 */
 const char * getOCStackResult(OCStackResult result);

/**
  * @bŕief Prints an error message
  */
 void printNoMemoryMsg();


/**********************************************************************/
/* Functions
/**********************************************************************/

 /**
  * @brief Returns a string corrensponding to the request
  *
  * @param The entity handler request
  *
  * @return the string of the request
  */
 const char * getEntityHandlerRequestResult(OCEntityHandlerRequest *entityHandler);

OCEntityHandlerResult EntityHandlerCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest,
                            void *callbackParam)
{
    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response = { 0, 0, OC_EH_ERROR, 0, 0, { },{ 0 }, false };
    OCBaseResourceT *resource = (OCBaseResourceT*) callbackParam;
    OCRepPayload* payload = NULL;

    // Check the request type
    if(entityHandlerRequest && (flag & OC_REQUEST_FLAG))
    {
        OCRepPayloadDestroy(payload);
        ehResult = requestHandler(entityHandlerRequest, resource, &payload);
    }

    if(ehResult == OC_EH_ERROR)
    {
        OIC_LOG(ERROR, TAG, "ERROR getting request handler and setting paylaod");
        return ehResult;
    }

    // Send the response
    ehResult = responseHandler(&response, entityHandlerRequest, payload, ehResult);

    if(ehResult == OC_EH_OK)
    {
        OIC_LOG(DEBUG, TAG, "Response sent successfully");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "ERROR Sending the response");
    }

    if (entityHandlerRequest && (flag & OC_OBSERVE_FLAG))
    {
        OIC_LOG(DEBUG, TAG, "Observer flag");
        ehResult = observerHandler(entityHandlerRequest, resource);
    }

    OCRepPayloadDestroy(payload);

    return ehResult;
}

/***************** PRIVATE FUNCTIONS *******************/
void pushResorcetoList(OCBaseResourceT **head)
{
    if(m_resourceList == NULL)
    {
        m_resourceList = *head;
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "Resource added as n element");
        OCBaseResourceT *current = m_resourceList;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = *head;
        current->next->next = NULL;
        OIC_LOG_V(DEBUG, TAG, "Resource added as n element: %s", current->next->uri);
    }
}

/***************** PUBLIC FUNCTIONS *******************/


/**
  * @brief Initializes and creates a resource
  *
  * @param uri          Path and name of the resource
  * @param interface    Resource interfaces
  * @param type         Resource types
  * @param properties   Byte of allowed properties of the resources
  * @param OCIOhandler Callback called when a PUT request is received
  *
  * @return Pointer to the created resource
  */
OCBaseResourceT * createResource(char* uri, OCResourceType* type, OCResourceInterface* interface,
                         uint8_t properties, OCIOHandler outputHandler, OCIOPort* port)
{
    // Create the resource
    OIC_LOG_V(DEBUG, TAG, "Creating resource with uri: %s\n", uri);
    OIC_LOG(DEBUG, TAG, "Entering createResource...");

    OCBaseResourceT *resource = (OCBaseResourceT*)malloc(sizeof(OCBaseResourceT));
    if(resource == NULL) {printNoMemoryMsg(); return NULL;}

    resource->uri = uri;

    resource->type = (OCResourceType*)malloc(sizeof(OCResourceType));
    if(resource->type == NULL) {printNoMemoryMsg(); return NULL;}
    memcpy(resource->type, type, sizeof(OCResourceType));

    resource->interface = (OCResourceInterface*)malloc(sizeof(OCResourceInterface));
    if(resource->interface == NULL) {printNoMemoryMsg(); return NULL;}
    memcpy(resource->interface, interface, sizeof(OCResourceInterface));

    resource->resourceProperties = properties;

    resource->OCIOhandler = outputHandler;

    resource->underObservation = false;

    resource->attribute = NULL;
    resource->next = NULL;
    OCStackResult res = OCCreateResource(&resource->handle,
            resource->type->resourcetypename,
            resource->interface->name,
            resource->uri,
            EntityHandlerCb,
            resource,
            resource->resourceProperties);
    OIC_LOG_V(DEBUG, TAG, "Created resource with OCStackResult: %s", res);

    if(res != OC_STACK_OK) 
    {
      OIC_LOG(ERROR, TAG, "Error calling OCCreateResource. Error code is: \n");
      Serial.println(getOCStackResult(res));
      return NULL;
    }

    // Add types
    OCResourceType *currentType = resource->type->next;
    while(currentType != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Adding type in createResource: %s", currentType->resourcetypename);
        if(OCBindResourceTypeToResource(resource->handle, currentType->resourcetypename) != OC_STACK_OK)
        {
            OIC_LOG_V(ERROR, TAG, "Unable to bind type to resource: %s", currentType->resourcetypename);
        }
        currentType = currentType->next;
    }


    // Add interfaces
    OCResourceInterface *currentInterface = resource->interface->next;
    while(currentInterface != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Adding interface in createResource: %s", currentInterface->name);
        if(OCBindResourceInterfaceToResource(resource->handle, currentInterface->name) != OC_STACK_OK)
        {
            OIC_LOG_V(ERROR, TAG, "Unable to bind interface to resource: %s", currentInterface->name);
        }
        currentInterface = currentInterface->next;
    }

    //#ifdef WITH_ARDUINO

    if(port)
    {
        resource->port = (OCIOPort*)malloc(sizeof(OCIOPort));
        if(resource->port == NULL) {printNoMemoryMsg(); return NULL;}
        memcpy(resource->port, port, sizeof(OCIOPort));
        pinMode(resource->port->pin, resource->port->type);

        OIC_LOG_V(DEBUG, TAG, "New port initalized at port %i", (int) port->pin);
    }

    //#endif

    // Add the resource to the list of available resources
    pushResorcetoList(&resource);

    return resource;
}

/**
  * @brief createResource
  *
  * @param uri           Path and name of the resource
  * @param type          Resource interface
  * @param interface     Resource type
  * @param properties    Allowed properties of the resource
  * @param OCIOhandler Callback called when a PUT request is received
  *
  * @return
  */
OCBaseResourceT * createResource(char* uri, const char* type, const char* interface, uint8_t properties,
                                 OCIOHandler outputHandler, OCIOPort* port)
{
    OCResourceType resourceType;
    resourceType.resourcetypename = (char*) type;
    resourceType.next = NULL;

    OCResourceInterface resourceInterface;
    resourceInterface.name = (char*) interface;
    resourceInterface.next = NULL;

    return(createResource(uri, &resourceType, &resourceInterface, properties, outputHandler, port));
}


/**
 * @brief Initializes and creates a resource
 *
 * @param resource      Resource to be initialized
 */
void createResource(OCBaseResourceT *resource)
{
    createResource(resource->uri, resource->type, resource->interface,
                   resource->resourceProperties, resource->OCIOhandler, resource->port);
}

/**
 * @brief addType Adds and bind a type to a resource
 *
 * @param resource      The resource to bind a type to
 * @param type          Type to be bound and added
 *
 * @return OC_STACK_OK if successfully bound
 */
OCStackResult addType(OCBaseResourceT *resource, OCResourceType *type)
{
    OCResourceType *current = resource->type;

    // Loop through the linked list to find the tail element
    while(current->next != NULL)
    {
        current = current->next;
    }
    current->next = (OCResourceType*)malloc(sizeof(OCResourceType));
    if(current->next == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}
    memcpy(current->next, type, sizeof(OCResourceType));

    OCStackResult result = OCBindResourceTypeToResource(resource->handle, type->resourcetypename);

    //OIC_LOG_V(INFO, TAG, "Result of type binding: %s", getOCStackResult(result));
    OIC_LOG_V(INFO, TAG, "Type added: %s", type->resourcetypename);

    free(type);

    return result;
}

/**
 * @brief addType Adds and bind a type to a resource
 *
 * @param resource      The resource to bind a type to
 * @param typeName      String of the type name
 *
 * @return OC_STACK_OK if successfully
 */
OCStackResult addType(OCBaseResourceT *resource, const char *typeName)
{
    OCResourceType *type = (OCResourceType*)malloc(sizeof(OCBaseResourceT));
    if(type == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}

    type->resourcetypename = (char*) typeName;
    type->next = NULL;

    return(addType(resource, type));
}

/**
 * @brief addInterface Adds and bind a interface to a resource
 *
 * @param resource      The resource to bind a type to
 * @param interface     Type to be bound and added
 *
 * @return OC_STACK_OK if successfully bound
 */
OCStackResult addInterface(OCBaseResourceT *resource, OCResourceInterface *interface)
{
    OCResourceInterface *current = resource->interface;

    // Loop through the linked list of ind the tail element
    while(current->next != NULL)
    {
        current = current->next;
    }

    current->next = (OCResourceInterface*)malloc(sizeof(OCResourceInterface));
    if(current->next == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}
    memcpy(current->next, interface, sizeof(OCResourceInterface));

    OCStackResult result = OCBindResourceInterfaceToResource(resource->handle, interface->name);

    //OIC_LOG_V(INFO, TAG, "Result of type binding: %s", getOCStackResult(result));
    OIC_LOG_V(INFO, TAG, "Interface added: %s", interface->name);


    free(interface);

    return result;
}

/**
 * @brief addInterface  Adds and bind a interface to a resource
 *
 * @param resource      The resource to bind a type to
 * @param interfaceName Interface name to bind.
 *
 * @return OC_STACK_OK if successfully bound
 */
OCStackResult addInterface(OCBaseResourceT *resource, const char *interfaceName)
{
    OCResourceInterface *interface = (OCResourceInterface*)malloc(sizeof(OCResourceInterface));
    if(interface == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}

    interface->name = (char*) interfaceName;
    interface->next = NULL;

    return(addInterface(resource, interface));
}

/**
 * @brief addAttribute  Adds an attribute to the resource
 *
 * @param resource      The resource to add an attribute to
 * @param attribute     The attribute to be added
 */
OCStackResult addAttribute(OCRepPayloadValue **head, OCRepPayloadValue *attribute)
{
    // Create new node and assign it a memory address
    OCRepPayloadValue *new_node = (OCRepPayloadValue*)calloc(1, sizeof(OCRepPayloadValue));
    //new_node->value.data.str = (char*)malloc(sizeof(attribute->value.data.str));
    if(new_node == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}

    if(*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        OCRepPayloadValue *current = *head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }

    memcpy(new_node, attribute, sizeof(OCRepPayloadValue));
    //memcpy(&new_node->value, &attribute->value, sizeof(attribute->value));
    new_node->next = NULL;

    // If a port has been declared, initialize it and copy the memory
    /*if(port)
    {
        new_node->port = (OCIOPort*)malloc(sizeof(OCIOPort));
        if(new_node->port == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}
        memcpy(new_node->port, port, sizeof(OCIOPort));
        pinMode(new_node->port->pin, new_node->port->type);

        OIC_LOG_V(DEBUG, TAG, "New port initalized at port %i", (int) port->pin);
    }*/

    OIC_LOG_V(DEBUG, TAG, "Added attribute with name: %s", (*head)->name);

    return OC_STACK_OK;
}

/**
 * @brief addAttribute  Adds an attribute to the resources
 *
 * @param resource      The resource to add an attribute to
 * @param name          The name of the attribute
 * @param value         Value of the attribute
 * @param type          DataTypes type of the attribute
 */
 /*
OCStackResult addAttribute(OCAttributeT **head, OCRepPayloadValue* value, OCIOPort *port)
{
    OCAttributeT *attribute = (OCAttributeT*)malloc(sizeof(OCAttributeT));
    if(attribute == NULL) {printNoMemoryMsg(); return OC_STACK_NO_MEMORY;}

    attribute->port = port;
    attribute->name = value->name;
    attribute->value.dataType = value->type;
    attribute->value.data = value;
    attribute->next = NULL;

    OCStackResult result = addAttribute(head, attribute, port);

    free(attribute);
    return result;
}
*/
/**
 * @brief getResourceList Returns the list of registered devices
 *
 * @return The list of registered devices
 */
OCBaseResourceT * getResourceList()
{
    return m_resourceList;
}

/**
 * @brief printResourceData Prints the data of a resource
 *
 * @param resource      The resource's data to print
 */
void printResourceData(OCBaseResourceT *resource)
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

    printAttributes(resource->attribute);
    OIC_LOG(DEBUG, TAG, "=============================");
}

/**
 * @brief printAttributes Print a list of the attributes
 *
 * @param attributes    Linked list of attributes
 */
void printAttributes(OCRepPayloadValue *attributes)
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

/**
 * @brief Create and loads the payload
 *
 * @return the created payload
 */
 OCRepPayload *getPayload(OCEntityHandlerRequest *ehRequest, OCBaseResourceT *resource)
 {
     OIC_LOG(DEBUG, TAG, "Getting Payload");
     if(ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
     {
         OIC_LOG(ERROR, TAG, "Incoming payload not a representation");
         return NULL;
     }

    OCRepPayload *payload = OCRepPayloadCreate();

    if(!payload)
    {
        OIC_LOG(ERROR, TAG, "Failed to allocate Payload");
        return NULL;
    }

    OCRepPayloadSetUri(payload, resource->uri);
    //OCRepPayloadSetPropString(payload, "name", resource->name);

    return payload;
 }

/**
* @brief handles the response to the entity handler
*
* @param responrce the entityhandler response
*
* @parma result of the entityhandler;
*/
OCEntityHandlerResult responseHandler(OCEntityHandlerResponse *response, OCEntityHandlerRequest *entityHandlerRequest, OCRepPayload *payload, OCEntityHandlerResult ehResult)
{
  OIC_LOG(DEBUG, TAG, "Sending a response");

  response->requestHandle = entityHandlerRequest->requestHandle;
  response->resourceHandle = entityHandlerRequest->resource;
  response->ehResult = ehResult;
  response->payload = (OCPayload*)payload;
  // Indicate that response is NOT in a persistent buffer
  response->persistentBufferFlag = 0;
  /*response->numSendVendorSpecificHeaderOptions = 0;
  memset(response->sendVendorSpecificHeaderOptions, 0,
          sizeof response->sendVendorSpecificHeaderOptions);
  memset(response->resourceUri, 0, sizeof response->resourceUri);*/

  // Send the response
  OCStackResult stackResult = OCDoResponse(response);
  if (stackResult != OC_STACK_OK)
  {
      OIC_LOG_V(ERROR, TAG, "Error sending response with error code: %i", stackResult);
      return(OC_EH_ERROR);
  }

  return OC_EH_OK;
}

/**
 * @brief Handles what request was instantiated and the corrensponding action
 *
 * @param handler The EntityHandler
 * @param resource Base resource
 *
 * @return the result of the request
 */
OCEntityHandlerResult requestHandler(OCEntityHandlerRequest *ehRequest,
                                     OCBaseResourceT *resource, OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    // Prepare the payload
    OCRepPayload *constructedPayload = getPayload(ehRequest, resource);

    if(!constructedPayload)
    {
        OIC_LOG(ERROR, TAG, "Unable to construct a payload.");
        return OC_EH_ERROR;
    }

    // Check what request was instantiated
    switch(ehRequest->method)
    {
        case OC_REST_GET:
            OIC_LOG(DEBUG, TAG, "GET Request");
            ehResult = getRequest(resource, constructedPayload);
            break;
        case OC_REST_PUT:
            OIC_LOG(DEBUG, TAG, "PUT Request");
            ehResult = putRequest(ehRequest, constructedPayload, resource);
            break;
        case OC_REST_POST:
            OIC_LOG(DEBUG, TAG, "POST Request");
            ehResult = postRequest(ehRequest, constructedPayload, resource);
            break;
        case OC_REST_DELETE:
            //ehResult = deleteRequest(resource);
            break;
        case OC_REST_DISCOVER:
            OIC_LOG(DEBUG, TAG, "Request is discover!");
            break;
        default:
            ehResult = OC_EH_ERROR;
    }

    *payload = constructedPayload;

    return ehResult;
}


/**
 * @brief observerHandler
 *
 * @param ehRequest     Request information from the client
 * @param resource      Pointer to the request resource
 */
OCEntityHandlerResult observerHandler(OCEntityHandlerRequest *ehRequest, OCBaseResourceT *resource)
{
    OCEntityHandlerResult result = OC_EH_OK;

    OIC_LOG(DEBUG, TAG, "Inside observerHandler");

    if(OC_OBSERVE_REGISTER == ehRequest->obsInfo.action)
    {
        OIC_LOG(INFO, TAG, "Received OC_OBSERVE_REGISTER from client");
        resource->underObservation = true;
    }
    else if(OC_OBSERVE_DEREGISTER == ehRequest->obsInfo.action)
    {
        OIC_LOG(INFO, TAG, "Received OC_OBSERVER_DEREGISTER from client");
        //resource->underObservation = false;
    }
    else
    {
        OIC_LOG(ERROR, TAG, "Unknown Observation request");
        result = OC_EH_ERROR;
    }


    return result;
}

/**
 * @brief Called when a REST GET is request
 *
 * @param OCBaseResource base resource attributes
 *
 * @return result of the entityHandler
 */
 OCEntityHandlerResult getRequest(OCBaseResourceT *resource, OCRepPayload *payload)
 {
    // Get the attributes and set them.
    OCRepPayloadValue *current = resource->attribute;
    while(current != NULL)
    {
        // Check type
        switch(current->type)
        {
        case OCREP_PROP_INT:
            OCRepPayloadSetPropInt(payload, current->name, current->i);// *((int*)current->value.data));
            break;
        case OCREP_PROP_DOUBLE:
            OCRepPayloadSetPropDouble(payload, current->name, current->d);// *((double*)current->value.data));
            break;
        case OCREP_PROP_STRING:
            OCRepPayloadSetPropString(payload, current->name, current->str);// *((char**)current->value.data));
            break;
        case OCREP_PROP_BOOL:
            OCRepPayloadSetPropBool(payload, current->name, current->b);// *((bool*)current->value.data));
            break;
        default:
            OIC_LOG(ERROR, TAG, "Unknown OcRepPayloadValue Type");
            break;
        }

        current = current->next;
    }
    return OC_EH_OK;
 }

 /**
 * @brief Called when a REST PUT is request
 *
 * @param OCBaseResource base resource attributes
 *
 * @return result of the entityHandler
 */
 OCEntityHandlerResult putRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload* payload, OCBaseResourceT *resource)
 {
    // Set the new states
    OCRepPayload* inputPayload = (OCRepPayload*)(ehRequest->payload);

    OCRepPayloadValue *current = resource->attribute;
    while(current != NULL)
    {
        switch(current->type)
        {
        case OCREP_PROP_INT:
        {
            int64_t value(0);
            if(OCRepPayloadGetPropInt(inputPayload, current->name, &value))
            {
                //OIC_LOG_V(DEBUG, TAG, "PUT: Type is int: %i", (int) value);
                //*((int*)current->value.data) = (int) value;
                current->i = value;
            }
            OCRepPayloadSetPropInt(payload, current->name, value);
            break;
        }
        case OCREP_PROP_DOUBLE:
        {
            double value(0);
            if(OCRepPayloadGetPropDouble(inputPayload, current->name, &value))
            {
                //OIC_LOG_V(DEBUG, TAG, "PUT: type is double: &d", value);
                //*((double*)current->value.data) = value;
                current->d = value;
            }
            OCRepPayloadSetPropDouble(payload, current->name, value);
            break;
        }
        case OCREP_PROP_STRING:
        {
            char* value("");
            if(OCRepPayloadGetPropString(inputPayload, current->name, &value))
            {
                //OIC_LOG_V(DEBUG, TAG, "PUT: type is string: %s", value);
                //*((char**)current->value.data) = value;
                current->str = value;
            }
            OCRepPayloadSetPropString(payload, current->name, value);
        }
        case OCREP_PROP_BOOL:
        {
            bool value(false);
            if(OCRepPayloadGetPropBool(inputPayload, current->name, &value))
            {
                //OIC_LOG_V(DEBUG, TAG, "PUT: Type is bool: %s", value ? "true" : "false");
                //*((bool*)current->value.data) = value;
                current->b = value;
            }
            OCRepPayloadSetPropBool(payload, current->name, value);
            break;
        }

        }

        current = current->next;
    }

    // Set the output pins
    if(resource->OCIOhandler != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Value of underObservation is: %s", resource->underObservation ? "true" : "false");
        resource->OCIOhandler(resource->attribute, resource->port, resource->handle, &resource->underObservation);
    }
    else
    {
        OIC_LOG(ERROR, TAG, "Resource OutputHandler has not been set");
    }

    OIC_LOG(DEBUG, TAG, "Leaving putRequest");
    return OC_EH_OK;
 }

 /**
  * @brief postRequest Called when a RESTful POST request is called
  *
  * @param ehRequest    Request parameters
  * @param payload      Payload from the client
  * @param resource     Resource the call was refered to
  * @return
  */
 OCEntityHandlerResult postRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload* payload, OCBaseResourceT *resource)
 {
     // See if the esource exist
     if(findResource(resource->handle))
     {
        // Standard put request
        return(putRequest(ehRequest, payload, resource));
     }
     else
     {
        OIC_LOG(DEBUG, TAG, "Creating new resource");

        OCRepPayload* inputPayload = (OCRepPayload*)(ehRequest->payload);

        // Create the resource
        OCBaseResourceT *newResource = createResource(inputPayload->uri, inputPayload->types->value,
          inputPayload->interfaces->value, OC_OBSERVABLE | OC_DISCOVERABLE, NULL, NULL);

        // Add the remaining types and interfaces
        OCStringLL *type = inputPayload->types->next;
        while(type != NULL) 
        {
          addType(newResource, type->value);
          type = type->next;
        }

        OCStringLL *interface = inputPayload->interfaces->next;
        while(interface != NULL)
        {
          addInterface(newResource, interface->value);
          interface = interface->next;
        }

        // Add Attributes
        OCRepPayloadValue *value = inputPayload->values;
        while(value != NULL)
        { 
          addAttribute(&newResource->attribute, value);
        }

        return OC_EH_OK;
     }
 }

  /**
   * @brief Search for a resource in the resource list
   *
   * @param handle     Handle of the resource
   *
   * @return true if found
   */
 bool findResource(OCResourceHandle handle)
 {
    OCBaseResourceT *current = m_resourceList;
    while(current != NULL)
    {
      if(handle == current->handle)
      {
        return true;
      }
      current = current->next;
    }
    return false;
 }

/**
 * @brief Returns the result of a OCStackResult as a string
 *
 * @param OCStackResult The result to be converted to a string
 *
 * @return A string with the result
*/
 
const char * getOCStackResult(OCStackResult result)
{
   switch (result) {
    case OC_STACK_OK:
        return "OC_STACK_OK";
    case OC_STACK_INVALID_URI:
        return "OC_STACK_INVALID_URI";
    case OC_STACK_INVALID_QUERY:
        return "OC_STACK_INVALID_QUERY";
    case OC_STACK_INVALID_IP:
        return "OC_STACK_INVALID_IP";
    case OC_STACK_INVALID_PORT:
        return "OC_STACK_INVALID_PORT";
    case OC_STACK_INVALID_CALLBACK:
        return "OC_STACK_INVALID_CALLBACK";
    case OC_STACK_INVALID_METHOD:
        return "OC_STACK_INVALID_METHOD";
    case OC_STACK_NO_MEMORY:
        return "OC_STACK_NO_MEMORY";
    case OC_STACK_COMM_ERROR:
        return "OC_STACK_COMM_ERROR";
    case OC_STACK_INVALID_PARAM:
        return "OC_STACK_INVALID_PARAM";
    case OC_STACK_NOTIMPL:
        return "OC_STACK_NOTIMPL";
    case OC_STACK_NO_RESOURCE:
        return "OC_STACK_NO_RESOURCE";
    case OC_STACK_RESOURCE_ERROR:
        return "OC_STACK_RESOURCE_ERROR";
    case OC_STACK_SLOW_RESOURCE:
        return "OC_STACK_SLOW_RESOURCE";
    case OC_STACK_NO_OBSERVERS:
        return "OC_STACK_NO_OBSERVERS";
    case OC_STACK_ERROR:
        return "OC_STACK_ERROR";
    default:
        return "UNKNOWN";
    }
}

/**
 * @brief Returns a string corrensponding to the request
 *
 * @param The entity handler request
 *
 * @return the string of the request
 */
const char * getEntityHandlerRequestResult(OCEntityHandlerRequest *entityHandler)
{
    switch(entityHandler->method)
    {
        case OC_REST_NOMETHOD:
            return "OC_ENTITY_HANDLER_NO_METHOD";
        case OC_REST_GET:
            return "OC_ENTITY_HANDLER_GET";
        case OC_REST_PUT:
            return "OC_ENTITY_HANDLER_PUT";
        case OC_REST_POST:
            return "OC_ENTITY_HANDLER_POST";
        case OC_REST_DELETE:
            return "OC_ENTITY_HANLDER_DELETE";
        case OC_REST_OBSERVE:
            return "OC_ENTITY_HANDLER_OBSERVE";
        case OC_REST_OBSERVE_ALL:
            return "OC_ENTITY_HANDLER_OBSERVE_ALL";
        case OC_REST_CANCEL_OBSERVE:
            return "OC_ENTITY_HANDLER_CANCEL_OBSERVE";
        case OC_REST_DISCOVER:
            return "OC_ENTITY_HANDLER_DISCOVER";
        default:
            return "OC_ENTITY_HANDLER_UKNOWN";
    }
}

/**
  * @bŕief Prints an no memory error message
  */
 void printNoMemoryMsg()
 {
 	OIC_LOG(ERROR, TAG, "No Memory");
 }



