#include "Controller.h"
#include "RCSRemoteResourceObject.h"

namespace OIC { namespace Service
{

    constexpr unsigned int CONTROLLER_POLLING_DISCOVERY_MS = 5000; // in milliseconds

/****************************************** Controller ************************************************/

    Controller::Controller() :
        m_discoveryCallback(std::bind(&Controller::foundResourceCallback, this, std::placeholders::_1)),
        m_discoveryCallbackBLE(std::bind(&Controller::foundResourceCallbackBLE, this, std::placeholders::_1)),
        m_resourceList(),
        m_resourceObjectCacheCallback(std::bind(&Controller::resourceObjectCacheCallback, this, std::placeholders::_1, std::placeholders::_2,
                                           std::placeholders::_3)),
        m_resourceObjectStateCallback(std::bind(&Controller::resourceObjectStateCallback, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3))
	{
        //this->configurePlatform();

        // Set up the scene and the collection
        SceneList::getInstance()->setName("Office");
        m_sceneCollection = SceneList::getInstance()->addNewSceneCollection();
        m_sceneCollection->setName("Meeting Room");

        m_sceneStart = m_sceneCollection->addNewScene("Start Conference");
        m_sceneStop = m_sceneCollection->addNewScene("Stop Conference");
        m_sceneState = SceneState::STOP_SCENE;
    }

    /**
     * @brief getInstance
     * @return
     */
    Controller* Controller::getInstance()
    {
        static Controller* instance(new Controller);
        return instance;
    }


    Controller::~Controller()
	{
        m_resourceList.clear();

        this->stop();
	}

    /**
      * @brief Start the Controller
      *
      * @return The result of the startup. OC_STACK_OK on success
      */
    OCStackResult Controller::start()
    {
        // Start the discoveryManager
        const std::vector<std::string> types{OIC_DEVICE_LIGHT, OIC_DEVICE_BUTTON, OIC_DEVICE_SENSOR, OIC_DEVICE_FAN};
        m_discoveryTask = Controller::discoverResource(m_discoveryCallback, types);

        // Start the DiscoveryManager for BLE devices
        //m_discoveryManagerBLE.setTimer(TIMER_BLE_DISCOVERY);
        //m_discoveryManagerBLE.discoverResource("", types, m_discoveryCallbackBLE, "", CT_ADAPTER_GATT_BTLE);

        // Start the discovery manager
        OCStackResult result = OC_STACK_OK;

        std::cout << "startRH" << std::endl;
        if(startRH() != OC_STACK_OK)
        {
            result = OC_STACK_ERROR;
        }

        return result;
    }
    /**
      * @brief Stop the Controller
      *
      * @param OC_STACK_OK on success
      */
    OCStackResult Controller::stop()
    {
        OCStackResult result = this->stopRH();

        if(!m_discoveryTask->isCanceled())
        {
            m_discoveryTask->cancel();
        }

        for (auto iterator = m_resourceList.begin(); iterator != m_resourceList.end();)
        {
            try {
                if(iterator->second->getRemoteResourceObject() != nullptr)
                {
                    if(iterator->second->getRemoteResourceObject()->isCaching())
                    {
                        iterator->second->getRemoteResourceObject()->stopCaching();
                    }
                }
                else
                {
                    std::cerr << "Unable to stop caching for device: " << iterator->first << std::endl;
                }
            }
            catch (RCSException e)
            {
                std::cerr << "Failed to stop caching" << std::endl;
            }

            iterator++;
        }

        return result;
    }

    /**
     * @brief configurePlatform Configures the platform
     */
    void Controller::configurePlatform()
    {
        // Create PlatformConfig object
        PlatformConfig cfg {
            OC::ServiceType::InProc,
            OC::ModeType::Both,
            "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
            0,         // Uses randomly available port
            OC::QualityOfService::LowQos 
        };

        OCPlatform::Configure(cfg);
        //OCStackResult result = OCInit(NULL, 0, OC_CLIENT_SERVER);
    }

    /**
      * @brief Prints the data of an resource object
      *
      * @param resurce  Pointer holding the resource data
      *
      * @return OC_NO_RESOURCE if the resource doesn't exist.
      */
    OCStackResult Controller::printResourceData(RCSRemoteResourceObject::Ptr resource)
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
    ResourceObject::ResourceObjectCacheCallback Controller::getControllerResourceCacheObjCallback()
    {
        return this->m_resourceObjectCacheCallback;
    }

    ResourceObject::ResourceObjectStateCallback Controller::getControllerResourceStateObjCallback()
    {
        return this->m_resourceObjectStateCallback;
    }

     /**
       * @brief Function callback for found resources
       *
       * @param resource     The discovered resource.
       */
     void Controller::foundResourceCallback(RCSRemoteResourceObject::Ptr resource)
     {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        std::cout << __func__ << std::endl;

        if(this->isResourceLegit(resource))
        {
            // Make new ResourceObject
            ResourceObject::Ptr resourceObject = ResourceObject::Ptr(new ResourceObject(resource));

            if(m_resourceList.insert({resource->getUri() + resource->getAddress(), resourceObject}).second)
            {
                this->printResourceData(resource);
                this->addResourceToScene(resource);

                std::cout << "\tAdded device: " << resource->getUri() + resource->getAddress() << std::endl;
                std::cout << "\tDevice successfully added to the list" << std::endl;
            }
        }
     }

     /**
      * @brief foundResourceCallbackBLE Callback function when discoverying BLE devices
      *
      * @param resource The discovered resource
      */
     void Controller::foundResourceCallbackBLE(OCResource::Ptr resource)
     {
         std::cout << "===================================================" << std::endl;
         std::cout << "Found BLE device! " << std::endl;
         std::cout << "===================================================" << std::endl;

        Controller::foundResourceCallback(RCSRemoteResourceObject::fromOCResource(resource));
     }



    /**
     * Start the Resource Host. It looks for resource with device type
     * oic.r.resourcehosting
     *
     * @return
     */
    OCStackResult Controller::startRH()
    {
        std::cout << "Starting Resource Hosting service" << std::endl;

        if (OICStartCoordinate() != OC_STACK_OK)
        {
            std::cout << "Resource hosting failed" << std::endl;
            return OC_STACK_ERROR;
        }

        std::cout << "Resource Hosting service started successfully" << std::endl;

  /*          while (true)
    {
        sleep(2);
    }
*/
        return OC_STACK_OK;
    }

    /**
     * Stop the Resource Host.
     *
     * @return
     */
    OCStackResult Controller::stopRH()
    {
        if (OICStopCoordinate() != OC_STACK_OK)
        {
            std::cout << "Resource Hosting service stopped failed" << std::endl;
            return OC_STACK_ERROR;
        }
        else
        {
            std::cout << "Resource Hosting service stopped successfully" << std::endl;
        }
        return OC_STACK_OK;
    }

    /**
     * @brief printAttributes Prints the attributes of a resource
     *
     * @param attr          Attributes to be printed
     */
    void Controller::printAttributes(const RCSResourceAttributes& attr)
    {
        if(attr.empty())
        {
            std::cout << "\tAttributes empty" << std::endl;
        }
        else
        {
            std::cout << "\t Attributes: " << std::endl;

            for (const auto& attribute : attr)
            {
                std::cout << "\t\t Key: " << attribute.key() << std::endl;
                std::cout << "\t\t Value: " << attribute.value().toString() << std::endl;
            }
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
    RCSDiscoveryManager::DiscoveryTask::Ptr Controller::discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
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
    RCSDiscoveryManager::DiscoveryTask::Ptr Controller::discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
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
    bool Controller::isResourceLegit(RCSRemoteResourceObject::Ptr resource)
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
    void Controller::addResourceToScene(RCSRemoteResourceObject::Ptr resource)
    {
        // Search through the resource types
        for(const auto& type : resource->getTypes())
        {
            if(type.compare(OIC_DEVICE_LIGHT) == 0)
            {
                m_sceneStart->addNewSceneAction(resource, "power", true);
                m_sceneStop->addNewSceneAction(resource, "power", false);
            }
            else if(type.compare(MP_TYPE_TV_MODE) == 0)
            {
                m_sceneStart->addNewSceneAction(resource, "tvMode", "TV_ON");
                m_sceneStop->addNewSceneAction(resource, "tvMode", "TV_OFF");
            }
        }
    }

    /**
     * @brief executeSceneCallback Cb invoked when a scene is executed
     *
     * @param eCode Result of the scene execution.
     */
    void Controller::executeSceneCallback(int eCode)
    {
        std::cout << __func__ << std::endl;

        std::cout << "Result of eCode: " << eCode << std::endl;
    }

    /**
     * @brief resourceObjectCallback Callback invoked when a new request for a resource is invoked.
     * @param resource      The resource that has been changed
     * @param state         The type of change that occured
     */
    void Controller::resourceObjectCacheCallback(const RCSResourceAttributes &attrs, const ResourceObjectState &state, const ResourceDeviceType &deviceType)
    {
        // If the device is a button, search for the current state
        if(deviceType == ResourceDeviceType::OIC_BUTTON)
        {
            for(auto const &attr : attrs)
            {
                // Simple test scenario turning on/off the LED.
                const std::string key = attr.key();
                const RCSResourceAttributes::Value value = attr.value();
                if(key == "state" && value.toString() == "true")
                {
                    if(m_sceneState == SceneState::START_SCENE)
                    {
                        std::cout << "\nSetting Scene State: STOP_SCENE\n";
                        m_sceneState = SceneState::STOP_SCENE;
                        m_sceneStop->execute(std::bind(&Controller::executeSceneCallback, this, std::placeholders::_1));
                    }
                    else
                    {
                        std::cout << "\nSetting Scene State: START_SCENE\n";
                        m_sceneState = SceneState::START_SCENE;
                        m_sceneStart->execute(std::bind(&Controller::executeSceneCallback, this, std::placeholders::_1));
                    }
                }
            }
        }
    }


    /**
     * @brief State called when the resource's state changes
     *
     * @param state New state of the resource
     * @param resourceKey Key of the resource to find it in the map
     */
    void Controller::resourceObjectStateCallback(const ResourceState &state, const std::string &uri, const std::string &address)
    {
        std::cout << __func__ << std::endl;

        switch(state)
        {
        case ResourceState::ALIVE:
                std::cout << "Resource with uri " << uri << " is ALIVE again" << std::endl;
            break;
        case ResourceState::LOST_SIGNAL:
        case ResourceState::DESTROYED:
            {
                std::cout << "Lost Signal to " << uri << std::endl;

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
    }

} }
