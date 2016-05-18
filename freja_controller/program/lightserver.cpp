//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

///
/// This sample provides steps to define an interface for a resource
/// (properties and methods) and host this resource on the server.
///

#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"
#include "resource_types.h"

#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"
#include "ConfigurationCollection.h"
#include "MaintenanceCollection.h"
//#include "FactorySetCollection.h"

using namespace OC;
using namespace OIC;
using namespace std;
namespace PH = std::placeholders;

int gObservation = 0;
void * ChangeLightRepresentation(void *param);
void * handleSlowResponse(void *param, std::shared_ptr< OCResourceRequest > pRequest);
bool gUnderObservation = false;

// Specifies secure or non-secure
// false: non-secure resource
// true: secure resource
bool isSecure = false;

/// Specifies whether Entity handler is going to do slow response or not
bool isSlowResponse = false;

// Configuration resource
static ConfigurationResource* g_configurationResource;
static ThingsConfiguration* g_thingsConf;
std::condition_variable g_bootstrapCondition;
std::mutex g_mutex;
bool g_bootstrapSet = false;
typedef std::function< OCRepresentation(void) > getFunc;

// Forward declaring the entityHandler

/// This class represents a single resource named 'lightResource'. This resource has
/// two simple properties named 'state' and 'power'

class LightResource
{

public:
    /// Access this property from a TB client
    bool m_power;
    std::string m_lightUri;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_lightRep;

public:
    /// Constructor
    LightResource() :
            m_power(false), m_lightUri("/a/light"), m_resourceHandle(0)
    {
        // Initialize representation
        m_lightRep.setUri(m_lightUri);

        m_lightRep.setValue("power", m_power);
    }

    /* Note that this does not need to be a member function: for classes you do not have
     access to, you can accomplish this with a free function: */

    /// This function internally calls registerResource API.
    void createResource()
    {
        std::string resourceURI = m_lightUri; //URI of the resource
        std::string resourceTypeName = "oic.d.light"; //resource type name. In this case, it is light
        std::string resourceInterface = DEFAULT_INTERFACE; // resource interface.

        EntityHandler cb = std::bind(&LightResource::entityHandler, this, PH::_1);

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle, resourceURI,
                resourceTypeName, resourceInterface, cb, OC_DISCOVERABLE | OC_OBSERVABLE);

        result = OCPlatform::bindTypeToResource(m_resourceHandle, OIC_TYPE_BINARY_SWITCH);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }
        else
        {
            cout << "Resource URI : " << resourceURI << endl;
            cout << "\tResource Type Name : " << resourceTypeName << endl;
            cout << "\tResource Interface : " << DEFAULT_INTERFACE << endl;
            cout << "\tResource creation is successful with resource handle : " << m_resourceHandle
                    << endl;
        }

        // Enable prsence
        if(OCStartPresence(OC_MAX_PRESENCE_TTL_SECONDS - 1) != OC_STACK_OK)
        {
            std::cerr << "Unable to start presence" << std::endl;
        }
    }

    OCResourceHandle getHandle()
    {
        return m_resourceHandle;
    }

    // Puts representation.
    // Gets values from the representation and
    // updates the internal state
    void put(OCRepresentation& rep)
    {
        std::cout << "Number of attributes: " << rep.numberOfAttributes() << std::endl;

        // Search through the attributes
        std::unique_ptr<OCRepPayload> payloadPtr(rep.getPayload());

        OCRepPayloadValue* values = payloadPtr->values;

        while(values != NULL)
        {
            std::cout << values->name << " value is: ";
            switch(values->type)
            {
                case OCREP_PROP_INT:
                    std::cout << "INT: " << values->i << std::endl;
                break;
                case OCREP_PROP_DOUBLE:
                    std::cout << "DOUBLE: " << values->d << std::endl;
                break;
                case OCREP_PROP_BOOL:
                    std::cout << "BOOL: " << std::boolalpha <<  values->b << std::endl;
                break;
                case OCREP_PROP_STRING:
                    std::cout << "STRING: " << values->str << std::endl;
                break;

            }
            values = values->next;
        }

        try
        {
            if (rep.getValue("power", m_power))
            {
                cout << "\t\t\t\t" << "power: " << std::boolalpha << m_power << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "power not found in the representation" << endl;
            }

            OCPlatform::notifyAllObservers(m_resourceHandle);
        }
        catch (exception& e)
        {
            cout << e.what() << endl;
        }

    }

    // Post representation.
    // Post can create new resource or simply act like put.
    // Gets values from the representation and
    // updates the internal state
    OCRepresentation post(OCRepresentation& rep)
    {
        put(rep);
        return get();
    }

    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {
        m_lightRep.setValue("power", m_power);

        return m_lightRep;
    }

    void addType(const std::string& type) const
    {
        OCStackResult result = OCPlatform::bindTypeToResource(m_resourceHandle, type);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

    void addInterface(const std::string& interface) const
    {
        OCStackResult result = OCPlatform::bindInterfaceToResource(m_resourceHandle, interface);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

private:
// This is just a sample implementation of entity handler.
// Entity handler can be implemented in several ways by the manufacturer
    OCEntityHandlerResult entityHandler(std::shared_ptr< OCResourceRequest > request)
    {
        //cout << "\tIn Server CPP entity handler:\n";
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if (request)
        {
            // Get the request type and request flag
            std::string requestType = request->getRequestType();
            int requestFlag = request->getRequestHandlerFlag();

            if (requestFlag & RequestHandlerFlag::RequestFlag)
            {
                //cout << "\t\trequestFlag : Request\n";
                auto pResponse = std::make_shared< OC::OCResourceResponse >();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                // If the request type is GET
                if (requestType == "GET")
                {
                    //cout << "\t\t\trequestType : GET\n";
                    if (isSlowResponse) // Slow response case
                    {
                        static int startedThread = 0;
                        if (!startedThread)
                        {
                            std::thread t(handleSlowResponse, (void *) this, request);
                            startedThread = 1;
                            t.detach();
                        }
                        ehResult = OC_EH_SLOW;
                    }
                    else // normal response case.
                    {
                        pResponse->setErrorCode(200);
                        pResponse->setResponseResult(OC_EH_OK);
                        pResponse->setResourceRepresentation(get());
                        if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            ehResult = OC_EH_OK;
                        }
                    }
                }
                else if (requestType == "PUT")
                {
                    //cout << "\t\t\trequestType : PUT\n";
                    OCRepresentation rep = request->getResourceRepresentation();

                    // Do related operations related to PUT request
                    // Update the lightResource
                    put(rep);
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(get());
                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "POST")
                {
                    //cout << "\t\t\trequestType : POST\n";

                    OCRepresentation rep = request->getResourceRepresentation();

                    // Do related operations related to POST request
                    OCRepresentation rep_post = post(rep);
                    pResponse->setResourceRepresentation(rep_post);
                    pResponse->setErrorCode(200);
                    if (rep_post.hasAttribute("createduri"))
                    {
                        pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                        pResponse->setNewResourceUri(
                                rep_post.getValue< std::string >("createduri"));
                    }

                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "DELETE")
                {
                    // DELETE request operations
                }
                if (requestFlag & RequestHandlerFlag::ObserverFlag)
                {
                    ObservationInfo observationInfo = request->getObservationInfo();
                    if(ObserveAction::ObserveRegister == observationInfo.action)
                    {
                        cout << "\t\trequestFlag : Register Observer\n";
                        gUnderObservation = true;
                    }
                    else if(ObserveAction::ObserveUnregister == observationInfo.action)
                    {
                        gUnderObservation = false;
                        cout << "\t\trequestFlag : Degister Observer\n";
                    }
                }
            }
        }
        else
        {
            std::cout << "Request invalid" << std::endl;
        }

        return ehResult;
    }
};

void * handleSlowResponse(void *param, std::shared_ptr< OCResourceRequest > pRequest)
{
    // This function handles slow response case
    LightResource* lightPtr = (LightResource*) param;
    // Induce a case for slow response by using sleep
    std::cout << "SLOW response" << std::endl;
    sleep(10);

    auto pResponse = std::make_shared< OC::OCResourceResponse >();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(lightPtr->get());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    // Set the slow response flag back to false
    isSlowResponse = false;
    OCPlatform::sendResponse(pResponse);
    return NULL;
}

// callback handler on GET request
void onBootstrap(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    std::cout << __func__ << std::endl;
    std::lock_guard<std::mutex> lock(g_mutex);
    g_bootstrapSet = true;

    if (eCode == OC_STACK_ERROR)
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        g_bootstrapCondition.notify_all();
        return ;
    }

    std::cout << "\n\nGET request was successful" << std::endl;
    std::cout << "\tResource URI: " << rep.getUri() << std::endl;
/*
    std::string defaultDeviceName = rep.getValue< std::string >("n");
    std::string defaultLocation = rep.getValue< std::string >("loc");
    std::string defaultLocationName = rep.getValue< std::string >("locn");
    std::string defaultRegion = rep.getValue< std::string >("r");
    std::string defaultCurrency = rep.getValue< std::string >("c");

    std::cout << "\tDeviceName : " << defaultDeviceName << std::endl;
    std::cout << "\tLocation : " << defaultLocation << std::endl;
    std::cout << "\tLocationName : " << defaultLocationName << std::endl;
    std::cout << "\tCurrency : " << defaultCurrency << std::endl;
    std::cout << "\tRegion : " << defaultRegion << std::endl;
*/
    // Set the configuration resource
    OCRepresentation newRep = rep;
    g_configurationResource->setConfigurationRepresentation(newRep);

    g_bootstrapCondition.notify_all();

}

getFunc getGetFunction(std::string uri)
{
    getFunc res = NULL;

    if (uri == g_configurationResource->getUri())
    {
        res = std::bind(&ConfigurationResource::getConfigurationRepresentation,
                g_configurationResource);
    }
    /*
    else if (uri == myMaintenanceResource->getUri())
    {
        res = std::bind(&MaintenanceResource::getMaintenanceRepresentation,
                myMaintenanceResource);
    }*/

    return res;
}

OCStackResult sendResponseForResource(std::shared_ptr< OCResourceRequest > pRequest)
{
    auto pResponse = std::make_shared< OC::OCResourceResponse >();

    // Check for query params (if any)
    QueryParamsMap queryParamsMap = pRequest->getQueryParameters();

    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());

    getFunc getFunction;
    getFunction = getGetFunction(pRequest->getResourceUri());

    OCRepresentation rep;
    rep = getFunction();

    auto findRes = queryParamsMap.find("if");

    if (findRes != queryParamsMap.end())
    {
        pResponse->setResourceRepresentation(rep, findRes->second);
    }
    else
    {
        pResponse->setResourceRepresentation(rep, DEFAULT_INTERFACE);
    }

    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

// This function prepares a response for any incoming request to Light resource.
bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request)
{
    std::cout << "\tIn Server CPP prepareResponseForResource:\n";
    bool result = false;
    if (request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            std::cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if (requestType == "GET")
            {
                std::cout << "\t\t\trequestType : GET\n";
                // GET operations are directly handled while sending the response
                // in the sendLightResponse function
                result = true;
            }
            else if (requestType == "POST")
            {
                // POST request operations
            }
            else if (requestType == "DELETE")
            {
                // DELETE request operations
            }
        }
        else if (requestFlag == RequestHandlerFlag::ObserverFlag)
        {
            std::cout << "\t\trequestFlag : Observer\n";
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return result;
}

OCEntityHandlerResult entityHandlerConfiguration(std::shared_ptr< OCResourceRequest > request)
{
    std::cout << "\tIn Server CPP (entityHandlerForResource) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    QueryParamsMap test = request->getQueryParameters();

    if (prepareResponseForResource(request))
    {
        if (OC_STACK_OK == sendResponseForResource(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareResponse failed." << std::endl;
    }
    return ehResult;
}

int main()
{
    // Create PlatformConfig object
    PlatformConfig cfg
    { OC::ServiceType::InProc, OC::ModeType::Both, "0.0.0.0",
    // By setting to "0.0.0.0", it binds to all available interfaces
            0,// Uses randomly available port
            OC::QualityOfService::LowQos };

    OCPlatform::Configure(cfg);
    try
    {
        // Create the instance of the resource class
        // (in this case instance of class 'LightResource').
        LightResource myLight;

        // Invoke createResource function of class light.
        myLight.createResource();

        // Create the configuration resource
        g_configurationResource = new ConfigurationResource();
        g_configurationResource->createResources(&entityHandlerConfiguration);
        g_thingsConf = new ThingsConfiguration();

        // Set the bootstrap server
        if( g_thingsConf->doBootstrap(&onBootstrap) == OC_STACK_OK)
        {
            // Hold the condition
            std::cout << "Waiting for boostrap server" << std::endl;
            std::unique_lock<std::mutex> lock(g_mutex);
            g_bootstrapCondition.wait_for(lock, std::chrono::milliseconds(30000), [](){return g_bootstrapSet;});
        }

        std::cout << "Boostrap server has been found" << std::endl;

        while(OCProcess() == OC_STACK_OK)
        {
            sleep(0.5);
        }
    }
    catch (OCException e)
    {
        //log(e.what());
    }

    // No explicit call to stop the platform.
    // When OCPlatform::destructor is invoked, internally we do platform cleanup

    return 0;
}
