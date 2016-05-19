/******************************************************************
 *
 * Copyright 2016 MP All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Rich OIC Server applicable of discovery, device management and
 * complies all the functionalities given by the OC Core.
 ******************************************************************/

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "resource_types.h"

#include "OCPlatform.h"
#include "OCApi.h"
#include "ExpiryTimer.h"
#include "rd_server.h"

#include "PrimitiveResource.h"
#include "RCSResourceObject.h"
#include "RCSRemoteResourceObject.h"
#include "RCSDiscoveryManager.h"

#include "ResourceObject.hpp"
#include "DiscoveryManager.h"

#include "Hosting.h"

// Scene-manager
#include "SceneList.h"

namespace OIC { namespace Service
{

    const std::string HOST_TAG = "/hosting";
    const auto HOSTING_TAG_SIZE = HOST_TAG.size();

    constexpr cbTimer TIMER_BLE_DISCOVERY = 5000; // 5 seconds

    enum class SceneState
    {
        START_SCENE,
        STOP_SCENE
    };

    class Controller
	{
    private:
        typedef std::string ResourceKey;

        typedef std::unordered_map<ResourceKey, RCSDiscoveryManager::DiscoveryTask::Ptr>::const_iterator discoveryMapItr;

    public:
        typedef std::unique_ptr<Controller> Ptr;
        typedef std::unique_ptr<const Controller> ConstPtr;

	public:
        /**
         * @brief getInstance
         * @return
         */
        static Controller* getInstance();

		/**
          * Destructor.
          *
          *	Clear all memory and stop all processes.
          */
        ~Controller();

        /**
          * Start the Controller
          */
        OCStackResult start();

        /**
         '* Stops the Controller
          */
        OCStackResult stop();

        /**
          * @brief Prints the data of an resource object
          *
          * @param resurce      Pointer holding the resource data
          *
          * @return OC_NO_RESOURCE if the resource doesn't exist.
          */
        OCStackResult printResourceData(RCSRemoteResourceObject::Ptr resource);

        /**
         * @brief getControllerResourceObjCallback  Called by the ResourceObject to invoke a change
         *                                          in the specific resource
         * @return
         */
        ResourceObject::ResourceObjectCacheCallback getControllerResourceCacheObjCallback();
        ResourceObject::ResourceObjectStateCallback getControllerResourceStateObjCallback();

    private:
		/**
		  * Map containing all discovered resources. 
		  */
        std::unordered_map<ResourceKey, ResourceObject::Ptr> m_resourceList;

        /**
          * Mutex locking a discovered resource until it has been added to the map.
          */
        std::mutex m_resourceMutex;

        /**
          * DiscoveryTask used to cancel and observe the discovery process.
          */
        RCSDiscoveryManager::DiscoveryTask::Ptr m_discoveryTask;
        RCSDiscoveryManager::ResourceDiscoveredCallback m_discoveryCallback;

        /**
          * DiscoveryManager to discover BLE devices
          */
        FindCallback m_discoveryCallbackBLE;
        DiscoveryManager m_discoveryManagerBLE;

        /**
         * @brief m_sceneCollection
         * Collection of the scene. In this case the office
         */
        SceneCollection::Ptr m_sceneCollection;

        /**
         * @brief m_sceneStart
         * Scene environments with the specified actions for each individual resource
         */
        Scene::Ptr m_sceneStart;
        Scene::Ptr m_sceneStop;

        /**
         * @brief m_sceneState Current active scene state.
         */
        SceneState m_sceneState;

        /**
          * Callback inovked during a change in a registered resource;
          */
        ResourceObject::ResourceObjectCacheCallback m_resourceObjectCacheCallback;
        ResourceObject::ResourceObjectStateCallback m_resourceObjectStateCallback;


	private:
         /**
           *	Default Constructor.
           *
           *	Initialize platform and device info.
           *	Starts by discovering resource hosts and stores them in the resource list
           *	Discovers other resources afterwards.
           */
         Controller();

         /**
          * @brief configurePlatform Configures the platform
          */
         void configurePlatform();

        /**
          * @brief Function callback for found resources
          *
          * @param resource     The discovered resource.
          */
        void foundResourceCallback(std::shared_ptr<RCSRemoteResourceObject> resource);


        /**
         * @brief foundResourceCallbackBLE Callback function when discoverying BLE devices
         *
         * @param resource The discovered resource
         */
        void foundResourceCallbackBLE(OCResource::Ptr resource);

        /**
         * Start the Resource Host. It looks for resource with device type
         * oic.r.resourcehosting
         *
         * @return
         */
        OCStackResult startRH();

        /**
         * Stop the Resource Host.
         *
         * @return
         */
        OCStackResult stopRH();

        /**
         * @brief printAttributes Prints the attributes of a resource
         *
         * @param attr          Attributes to be printed
         */
        void printAttributes(const RCSResourceAttributes& attr);

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
        RCSDiscoveryManager::DiscoveryTask::Ptr discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
            RCSAddress address = RCSAddress::multicast(), const std::string& uri = std::string(""),
            const std::string& type = std::string(""));

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
        RCSDiscoveryManager::DiscoveryTask::Ptr discoverResource(RCSDiscoveryManager::ResourceDiscoveredCallback cb,
            const std::vector<std::string> &types, RCSAddress address = RCSAddress::multicast(), const std::string& uri = std::string(""));

        /**
          * @brief Looks up the list of known resources type
          *
          * @param resource     Pointer to the resource object
          *
          * @return True if the type is found, false otherwise.
          */
        bool isResourceLegit(RCSRemoteResourceObject::Ptr resource);


        /**
         * @brief addResourceToScene Adds a resource to the two scenes
         *
         * @param resource THe resource to be added
         */
        void addResourceToScene(RCSRemoteResourceObject::Ptr resource);


        /**
         * @brief executeSceneCallback Cb invoked when a scene is executed
         *
         * @param eCode Result of the scene execution.
         */
        void executeSceneCallback(int eCode);


        /**
         * @brief resourceObjectCallback Callback invoked when a new request for a resource is invoked.
         * @param resource      The resource that has been changed
         * @param state         The type of change that occured
         */
        void resourceObjectCacheCallback(const RCSResourceAttributes &attrs, const ResourceObjectState &state, const ResourceDeviceType &type);


        /**
         * @brief State called when the resource's state changes
         *
         * @param state New state of the resource
         * @param resourceKey Key of the resource to find it in the map
         */
        void resourceObjectStateCallback(const ResourceState &state, const std::string &uri, const std::string &address);

	protected:

	};
} }

#endif /* _CONTROLLER_H_ */

