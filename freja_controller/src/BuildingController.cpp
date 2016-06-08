#include "BuildingController.h"


using namespace OIC;
using namespace OIC::Service;

/**
 * @brief getInstance
 * @return
 */
BuildingController* BuildingController::getInstance()
{
    static BuildingController* instance(new BuildingController);
    return instance;
}

/**
  *
  */
BuildingController::~BuildingController()
{
    m_resourceList.clear();

    this->stop();
}

/**
 * @brief start
 * @return
 */
OCStackResult BuildingController::start()
{
    // Start the discoveryManager
    const std::vector<std::string> types{OIC_DEVICE_LIGHT, OIC_DEVICE_BUTTON, OIC_DEVICE_SENSOR,
                OIC_DEVICE_FAN, OIC_DEVICE_TV, OIC_TYPE_GRAPH};
    m_discoveryTask = BuildingController::discoverResource(m_discoveryCallback, types);

    // Start the FSM


    return OC_STACK_OK;
}

/**
 * @brief stop
 * @return
 */
OCStackResult BuildingController::stop()
{
    // Stop the discovery task
    try
    {
        m_discoveryTask->cancel();
        m_systemState == SystemState::IDLE;
    }
    catch (RCSException e)
    {
        std::cerr << e.what() << std::endl;
    }
}

/**
  * @brief Prints the data of an resource object
  *
  * @param resurce  Pointer holding the resource data
  *
  * @return OC_NO_RESOURCE if the resource doesn't exist.
  */
OCStackResult BuildingController::printResourceData(RCSRemoteResourceObject::Ptr resource)
{
    std::cout << "===================================================" << std::endl;
    std::cout << "\t Uri of the resources: " << resource->getUri() << std::endl;
    std::cout << "\t Host address of the resources: " << resource->getAddress() << std::endl;
    std::cout << "\t Types are: " << std::endl;

    for (auto type : resource->getTypes())
    {
        std::cout << "\t\t type " << type << std::endl;
    }

    std::cout << "\t Interfaces are: " << std::endl;
    for (auto interface : resource->getInterfaces())
    {
        std::cout << "\t\t interface " << interface << std::endl;
    }
}


/**
 * @brief getControllerResourceObjCallback  Called by the ResourceObject to invoke a change
 *                                          in the specific resource
 * @return
 */
ResourceObject::ResourceObjectCacheCallback BuildingController::getControllerResourceCacheObjCallback()
{
    return this->m_resourceObjectCacheCallback;
}

ResourceObject::ResourceObjectStateCallback BuildingController::getControllerResourceStateObjCallback()
{
    return this->m_resourceObjectStateCallback;
}



/**
 * @brief BuildingController
 */
BuildingController::BuildingController() :
    m_discoveryCallback(std::bind(&BuildingController::foundResourceCallback, this, std::placeholders::_1)),
    m_resourceList(),
    m_resourceObjectCacheCallback(std::bind(&BuildingController::resourceObjectCacheCallback, this, std::placeholders::_1, std::placeholders::_2,
                                            std::placeholders::_3)),
    m_resourceObjectStateCallback(std::bind(&BuildingController::resourceObjectStateCallback, this, std::placeholders::_1, std::placeholders::_2,
                                            std::placeholders::_3))
{
    // Set up the scenes
    SceneList::getInstance()->setName("Building Controller");

    m_sceneCollection = SceneList::getInstance()->addNewSceneCollection();
    m_sceneCollection->setName("Control Room");

    m_sceneLightsOn  = m_sceneCollection->addNewScene("Lights On");
    m_sceneLightsOff = m_sceneCollection->addNewScene("Lights Off");

    // Set the system state
    m_systemState = SystemState::IDLE;
    m_lightState = LightState::OFF;
}

/**
 * @brief stateMachineThread
 */
void BuildingController::checkStateMachine()
{
    switch(m_systemState)
    {
    case SystemState::IDLE:

        break;

    case SystemState::SAVE_TEMPEARTURE:
        // TODO: Save temperature
        if(m_graphResource != nullptr)
        {
            std::cout << "Saving temperature" << std::endl;
            RCSResourceAttributes attrs;
            attrs["save"] = true;
            try
            {
                m_graphResource->setRemoteAttributes(attrs, [&](RCSResourceAttributes attrs, int eCode) {
                    if(eCode != OC_STACK_OK)
                    {
                        std::cerr << "Set Attributes of OIC Graph resource failed with ecode: " <<  eCode << std::endl;
                    }
                });
            }
            catch(RCSInvalidParameterException e)
            {
                std::cerr << "Invalid parameter exception" << std::endl;
            }
        }
        else
        {
            std::cerr << "Graph Resource not yet discovered" << std::endl;
        }

        m_systemState = SystemState::IDLE;
        break;

    case SystemState::ALARM:
    {
        // Start light thread
        std::thread thread(&BuildingController::toggleLights, this, 1000, 1000, SystemState::ALARM_STARTED);
        thread.detach();
        m_systemState = SystemState::ALARM_STARTED;
        break;
    }

    case SystemState::ALARM_STARTED:
    {
        // Do Nothing
        break;
    }

    case SystemState::ALARM_STOPPED_THREAD:
    {
        // Start light thread
        std::thread thread(&BuildingController::toggleLights, this, 1600, 400, SystemState::ALARM_STOPPED);
        thread.detach();
        m_systemState = SystemState::ALARM_STOPPED;
        break;
    }

    default:

        break;
    }
}

/**
 * @brief toggleLights
 * @param onTimeMs
 * @param offTimeMs
 */
void BuildingController::toggleLights(int onTimeMs, int offTimeMs, SystemState state)
{
    while(m_systemState == state)
    {
        switch(m_lightState)
        {
        case LightState::ON:
            m_sceneLightsOn->execute(std::bind(&BuildingController::executeSceneCallback, this, std::placeholders::_1));
            std::this_thread::sleep_for(chrono::milliseconds(onTimeMs));
            m_lightState = LightState::OFF;
            break;

        case LightState::OFF:
            m_sceneLightsOff->execute(std::bind(&BuildingController::executeSceneCallback, this, std::placeholders::_1));
            std::this_thread::sleep_for(chrono::milliseconds(offTimeMs));
            m_lightState = LightState::ON;
            break;
        }
    }
    std::cout << "Exiting " << __func__ << " thread" << std::endl;
}


/**
  * @brief Function callback for found resources
  *
  * @param resource     The discovered resource.
  */
void BuildingController::foundResourceCallback(RCSRemoteResourceObject::Ptr resource)
{
   std::lock_guard<std::mutex> lock(m_resourceMutex);
   std::cout << __func__ << std::endl;

   try
   {
       if(resource->getUri().compare(GRAPH_URI) == 0)
       {
           std::cout << "Found graph resource" << std::endl;
           m_graphResource = resource;
       }
       else if(resource->getUri().compare(TEMPERATURE_URI) == 0)
       {
           std::cout << "\t Found temperature sensor" << std::endl;
           m_temperatureResource = resource;

           // Create new thread to run the GET commands
           std::thread thread(std::bind(&BuildingController::getTemperatureLoop, this));
           thread.detach();
       }
       else if(this->isResourceLegit(resource))
       {
           std::unordered_map<std::string, ResourceObject::Ptr>::const_iterator itr = m_resourceList.find (resource->getUri() + resource->getAddress());

           if ( itr == m_resourceList.end() )
           {
               // Make new ResourceObject
               ResourceObject::Ptr resourceObject = ResourceObject::Ptr(new ResourceObject(resource));

               m_resourceList.insert({resource->getUri() + resource->getAddress(), resourceObject});

               this->printResourceData(resource);
               this->addResourceToScene(resource);

               std::cout << "\tAdded device: " << resource->getUri() + resource->getAddress() << std::endl;
               std::cout << "\tDevice successfully added to the list" << std::endl;

           }
           else
           {
              std::cout << "Resource is already in the list" << std::endl;
           }

       }
   }
   catch (RCSException e)
   {
       std::cerr << e.what() << std::endl;
   }
}

/**
  *  @brief Disovery of resources
  *
  *  @param address 	mutlicast or unicast address using RCSAddress class
  *  @param cb 			Callback to which discovered resources are notified
  *  @param uri 		Uri to discover. If null, do not include uri in discovery
  *  @param type        Resource type used as discovery filter
  *
  *  @return Pointer to the discovery task.
  */
RCSDiscoveryManager::DiscoveryTask::Ptr BuildingController::discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
    RCSAddress address, const std::string& uri, const std::string& type)
{
    RCSDiscoveryManager::DiscoveryTask::Ptr discoveryTask;

    try
    {
        if (type.empty() && uri.empty())
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResource(address, cb);
        }
        else if (type.empty() && !(uri.empty()))
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResource(address, uri, cb);
        }
        else if (!(type.empty()) && uri.empty())
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(address, type, cb);
        }
        else
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(address, uri, type, cb);
        }
    }
    catch(const RCSPlatformException& e)
    {
         std::cout << e.what() << std::endl;
    }
    catch(const RCSException& e)
    {
        std::cout << e.what() << std::endl;
    }

    return discoveryTask;
}

/**
  *  @brief Disovery of resources
  *
  *  @param address 	mutlicast or unicast address using RCSAddress class
  *  @param cb 			Callback to which discovered resources are notified
  *  @param uri 		Uri to discover. If null, do not include uri in discovery
  *  @param types       Resources types used as discovery filter
  *
  *  @return Pointer to the discovery task.
  */
RCSDiscoveryManager::DiscoveryTask::Ptr BuildingController::discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
    const std::vector<std::string> &types, RCSAddress address, const std::string& uri)
{
    RCSDiscoveryManager::DiscoveryTask::Ptr discoveryTask;

    try
        {
        if(uri.empty())
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByTypes(address, types, cb);
        }
        else
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByTypes(address, uri, types, cb);
        }
    }
    catch(const RCSPlatformException& e)
    {
         std::cout << e.what() << std::endl;
    }
    catch(const RCSException& e)
    {
        std::cout << e.what() << std::endl;
    }

    return discoveryTask;
}

/**
  * @brief Looks up the list of known resources type
  *
  * @param resource     Pointer to the resource object
  *
  * @return True if the type is found, false otherwise.
  */
bool BuildingController::isResourceLegit(RCSRemoteResourceObject::Ptr resource)
{
    // Filter platform and device resources
    std::string uri = resource->getUri();
    std::vector<std::string> types = resource->getTypes();

    if (uri == "/oic/p" || uri == "/oic/d")
    {
        return false;
    }
    else if(uri.size() > HOSTING_TAG_SIZE)
    {
        if (uri.compare(
                uri.size()-HOSTING_TAG_SIZE, HOSTING_TAG_SIZE, HOST_TAG) == 0)
        {
            std::cout << "Device: " << uri << " is not a legit device. Device is hosting" << std::endl;
            return false;
        }
        return true;
    }
    else if (std::find_if(types.begin(), types.end(), [](const std::string &type) {return type == OIC_TYPE_RESOURCE_HOST;}) != types.end())
    {
        std::cout << "Resource type is Hosting. Not adding an additional monitoring state" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}


/**
 * @brief addResourceToScene Adds a resource to the two scenes
 *
 * @param resource THe resource to be added
 */
void BuildingController::addResourceToScene(RCSRemoteResourceObject::Ptr resource)
{
    try
    {
        // Search through the resource types
        for(const auto& type : resource->getTypes())
        {
            if(type.compare(OIC_DEVICE_LIGHT) == 0)
            {
                m_sceneLightsOn->addNewSceneAction(resource, "power", true);
                m_sceneLightsOff->addNewSceneAction(resource, "power", false);

            }
           // TODO: Add Speaker
        }
    }
    catch (RCSException e)
    {
        std::cerr << e.what() << std::endl;
    }
}


/**
 * @brief executeSceneCallback Cb invoked when a scene is executed
 *
 * @param eCode Result of the scene execution.
 */
void BuildingController::executeSceneCallback(int eCode)
{
    /*std::cout << "===================================" << std::endl;
    std::cout << "\t Executed scene with eCode: " << eCode << std::endl;
    std::cout << "===================================" << std::endl;
*/
    if(eCode != OC_STACK_OK)
    {
        std::cerr << "\n\t Failed to execute Scene! " << std::endl;
    }
}


/**
 * @brief resourceObjectCallback Callback invoked when a new request for a resource is invoked.
 * @param resource      The resource that has been changed
 * @param state         The type of change that occured
 */
void BuildingController::resourceObjectCacheCallback(const RCSResourceAttributes &attrs, const ResourceObjectState &state, const ResourceDeviceType &deviceType)
{
    // Itterate through the attributes
    for(auto const &attr : attrs)
    {
        const std::string key = attr.key();
        const RCSResourceAttributes::Value value = attr.value();
        // If the device is a button, search for the current state
        if(deviceType == ResourceDeviceType::OIC_BUTTON && key.compare("state") == 0 &&
                value.toString().compare("true") == 0)
        {
            std::cout << "A button was pressed" << std::endl;
            switch(m_systemState)
            {
            case SystemState::IDLE:
                m_systemState = SystemState::SAVE_TEMPEARTURE;
                checkStateMachine();
                break;

            case SystemState::SAVE_TEMPEARTURE:
                std::cerr << "System is currently saving the temperature!" << std::endl;
                break;

            case SystemState::ALARM:
            case SystemState::ALARM_STARTED:
                std::cout << "Setting state to ALARM_STOPPED" << std::endl;
                m_systemState = SystemState::ALARM_STOPPED_THREAD;
                checkStateMachine();
                break;

            case SystemState::ALARM_STOPPED:
                // No functionality
                break;

            default:

                break;

            }
        }
    }

    //printAttributes(attrs);
}

/**
 * @brief State called when the resource's state changes
 *
 * @param state New state of the resource
 * @param resourceKey Key of the resource to find it in the map
 */
void BuildingController::resourceObjectStateCallback(const ResourceState &state, const std::string &uri, const std::string &address)
{
  //  std::cout << "===================================" << std::endl;
    switch(state)
    {
    case ResourceState::ALIVE:
            //std::cout << "Resource with uri " << uri << " is ALIVE" << std::endl;
        break;
    case ResourceState::LOST_SIGNAL:
    case ResourceState::DESTROYED:
        {
            //std::cout << "Lost Signal to " << uri << std::endl;

            // Find the resource and remove it from the list
           /* ResourceKey resourceKey = uri + address;

            // Find the object
            std::unordered_map<ResourceKey, ResourceObject::Ptr>::const_iterator object = m_resourceList.find(resourceKey);
            if( object == m_resourceList.end())
            {
                std::cout << "Object not found!" << std::endl;
            }
            else
            {
                // Destroy the object
                std::cout << "Resetting resource object " << std::endl;
                ResourceObject::Ptr resource = m_resourceList.erase(object)->second;
                resource.reset();
            }*/
        }
        break;

    default:
            std::cout << "Unsupported resource state" << std::endl;
        break;
   }
  // std::cout << "===================================" << std::endl;
}


/**
 * @brief printAttributes     Prints the attributes.
 */
void BuildingController::printAttributes(const RCSResourceAttributes &attrs)
{
    std::cout << "=================================" << std::endl;
    if(attrs.empty())
    {
        std::cout << "\t No attributes present" << std::endl;
    }
    else
    {
        std::cout << "\t Attributes: " << std::endl;

        for (const auto& attribute : attrs)
        {
            std::cout << "\t\t Key: " << attribute.key() << std::endl;
            std::cout << "\t\t Value: " << attribute.value().toString() << std::endl;
        }
    }
     std::cout << "=================================\n" << std::endl;
}


/**
 * @brief getTemperatureLoop Continuously get the temperature readings
 */
void BuildingController::getTemperatureLoop()
{
    while(OCProcess() == OC_STACK_OK)
    {
        // Initiate a GET request
        if(m_temperatureResource != nullptr)
        {
            m_temperatureResource->getRemoteAttributes(std::bind(&BuildingController::onGetTemperatureReading, this,
                                                                 std::placeholders::_1, std::placeholders::_2));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(TEMPERATURE_THREAD_FREQ_MS));
    }
}


/**
 * @brief onGetTemperatureReading GET response of temperature reading
 *
 * @param attrs attributes
 * @param eCode result of GEt
 */
void BuildingController::onGetTemperatureReading(const RCSResourceAttributes &attrs, int eCode)
{
    // Get the temperature attribute
    static const std::string tempString = "Temperature";

    try {
        if(attrs.at(tempString) != nullptr)
        {
            double temperature = attrs.at(tempString).get<double>();
            std::cout << "Temperature is: " << temperature << std::endl;
            if(m_systemState == SystemState::IDLE && temperature > TEMPERATURE_UPPER_THRES)
            {
                std::cout << "Temperature has risen above " << TEMPERATURE_UPPER_THRES << " and the alarm is started" << std::endl;
                std::cout << "ALARM IS STARTED!!!" << std::endl;
                m_systemState = SystemState::ALARM;
            }
            else if((m_systemState == SystemState::ALARM_STARTED || m_systemState == SystemState::ALARM_STOPPED) &&
                    temperature < TEMPERATURE_LOWER_THRES)
            {
                std::cout << "Temperature has droppbed below " << TEMPERATURE_LOWER_THRES << " and the alarm is stopped\n";
                std::cout << "ALARM IS STOPPED!!!" << std::endl;
                m_systemState = SystemState::IDLE;
            }
            checkStateMachine();

        }
    }
    catch(RCSInvalidKeyException e)
    {
        std::cout << e.what() << std::endl;
    }

   /* for (const auto &attr : attrs)
    {
        if(attr.key().compare("Temperature") == 0)
        {
            std::cout << "Temperature is: " << attr.value().toString() << std::endl;

            double temperature = value.get<double>();

            if(m_systemState == SystemState::IDLE && temperature > TEMPERATURE_UPPER_THRES)
            {
                std::cout << "Temperature has risen above " << TEMPERATURE_UPPER_THRES << " and the alarm is started" << std::endl;
                std::cout << "ALARM IS STARTED!!!" << std::endl;
                m_systemState = SystemState::ALARM;
            }
            else if((m_systemState == SystemState::ALARM || m_systemState == SystemState::ALARM_STOPPED) &&
                    temperature < TEMPERATURE_LOWER_THRES)
            {
                std::cout << "Temperature has droppbed below " << TEMPERATURE_LOWER_THRES << " and the alarm is stopped\n";
                std::cout << "ALARM IS STOPPED!!!" << std::endl;
                m_systemState = SystemState::IDLE;
            }
            checkStateMachine();

        }
    }*/
}
