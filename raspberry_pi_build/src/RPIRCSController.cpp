#include "RPIRCSController.h"


RPIRCSController::RPIRCSController() : m_uri("/a/raspberryPi"), m_resourceType("raspberry.pi"),
  m_resourceInterface(OC_RSRVD_INTERFACE_DEFAULT)
{
    // Create the resource

    m_controller = RCSResourceObject::Builder(m_uri, m_resourceType, m_resourceInterface)
            .setDiscoverable(true)
            .setObservable(true)
            .setSecureFlag(false)
            .build();

    m_controller->setSetRequestHandler(std::bind(&RPIRCSController::setResponse, this, std::placeholders::_1,
                                                 std::placeholders::_2));
}

RPIRCSController* RPIRCSController::getInstance()
{
    static RPIRCSController* instance(new RPIRCSController);
    return instance;
}

RPIRCSController::~RPIRCSController()
{
    ;
}

RPIRCSController::ControllerMap RPIRCSController::getResourceList()
{
    return m_resourceList;
}

/**
 * @brief setResponse response called whenever a call to the rpi controller is applied.
 * @param request
 * @param attributes
 * @return
 */
RCSSetResponse RPIRCSController::setResponse(const RCSRequest& request, RCSResourceAttributes& attributes)
{
    // Get the query map
    ConstQueryParamsMap queryMap = request.getQueryParams();
    std::string type;
    std::string interface;
    std::string uri;

    for(auto const& mapEntry : queryMap)
    {
        std::cout << "Key: " << mapEntry.first << " " << "Value: " << mapEntry.second << std::endl;
        if(mapEntry.first.compare("rt") == 0)
        {
            type = mapEntry.second;
        }
        else if(mapEntry.first.compare("if") == 0)
        {
            interface = mapEntry.second;
        }
        else if(mapEntry.first.compare("uri") == 0)
        {
            m_uri = mapEntry.second;
        }
        else
        {
            std::cout << "Unknown value" << std::endl;
        }
    }

    RPIRCSResourceObject::Ptr newResource { new RPIRCSResourceObject(uri, type, interface) };
    newResource->createResource(true, true, false);

    if(m_resourceList.insert({uri + type, newResource}).second)
    {
        std::cout << "Inserted new resource successuflly" << std::endl;
    }

    return RCSSetResponse::defaultAction();
}


