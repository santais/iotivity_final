#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

class LightResource
{

public:
    /// Access this property from a TB client
    std::string m_name;
    bool m_state;
    int m_power;
    std::string m_lightUri;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_lightRep;
    ObservationIds m_interestedObservers;

public:
    /// Constructor
    LightResource()
        :m_name("John's light"), m_state(false), m_power(0), m_lightUri("/a/light"),
                m_resourceHandle(nullptr) {
        // Initialize representation
        m_lightRep.setUri(m_lightUri);

        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("brightness", m_power);
        m_lightRep.setValue("name", m_name);
    }

    /* Note that this does not need to be a member function: for classes you do not have
    access to, you can accomplish this with a free function: */

    /// This function internally calls registerResource API.
    void createResource()
    {
        //URI of the resource
        std::string resourceURI = m_lightUri;
        //resource type name. In this case, it is light
        std::string resourceTypeName = "oic.d.light";
        // resource interface.
        std::string resourceInterface = DEFAULT_INTERFACE;

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty;
    
        resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        
        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
                                    m_resourceHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }
    }

    OCStackResult createResource1()
    {
        // URI of the resource
        std::string resourceURI = "/a/light1";
        // resource type name. In this case, it is light
        std::string resourceTypeName = "oic.d.light";
        // resource interface.
        std::string resourceInterface = DEFAULT_INTERFACE;

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty;
       
        resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        
        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);

        OCResourceHandle resHandle;

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
                                    resHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }

        return result;
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
        try {
            if (rep.getValue("state", m_state))
            {
                cout << "\t\t\t\t" << "state: " << m_state << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "state not found in the representation" << endl;
            }

            if (rep.getValue("brightness", m_power))
            {
                cout << "\t\t\t\t" << "brightness: " << m_power << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "brightness not found in the representation" << endl;
            }
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
        /*static int first = 1;

        // for the first time it tries to create a resource
        if(first)
        {
            first = 0;

            if(OC_STACK_OK == createResource1())
            {
                OCRepresentation rep1;
                rep1.setValue("createduri", std::string("/a/light1"));

                return rep1;
            }
        }*/

        // from second time onwards it just puts
        put(rep);
        return get();
    }


    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {
        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("brightness", m_power);

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
OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            // Check for query params (if any)
            QueryParamsMap queries = request->getQueryParameters();

            if (!queries.empty())
            {
                std::cout << "\nQuery processing upto entityHandler" << std::endl;
            }
            for (auto it : queries)
            {
                std::cout << "Query key: " << it.first << " value : " << it.second
                        << std:: endl;
            }

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";

                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(get());
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                // Update the lightResource
                put(rep);
                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(get());
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "POST")
            {
                cout << "\t\t\trequestType : POST\n";

                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to POST request
                OCRepresentation rep_post = post(rep);
                pResponse->setResourceRepresentation(rep_post);
                pResponse->setErrorCode(200);
                if(rep_post.hasAttribute("createduri"))
                {
                    pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                    pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
                }
                else
                {
                    pResponse->setResponseResult(OC_EH_OK);
                }

                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "DELETE")
            {
                cout << "Delete request received" << endl;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = request->getObservationInfo();
            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_interestedObservers.erase(std::remove(
                                                            m_interestedObservers.begin(),
                                                            m_interestedObservers.end(),
                                                            observationInfo.obsId),
                                                            m_interestedObservers.end());
            }

            pthread_t threadId;

            cout << "\t\trequestFlag : Observer\n";

            ehResult = OC_EH_OK;
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return ehResult;
}

};
