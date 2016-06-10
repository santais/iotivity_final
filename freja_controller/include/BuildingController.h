#ifndef _BUILDINGCONTROLLER_H_
#define _BUILDINGCONTROLLER_H_

/* C++ Libraries */
#include <thread>
#include <iostream>
#include <chrono>


/* IoTivity Libraries */
#include "RCSRemoteResourceObject.h"
#include "RCSDiscoveryManager.h"
#include "SceneList.h"
#include "PrimitiveResource.h"
#include "OCApi.h"
#include "OCPlatform.h"


/* IoTivity Object Libraries */
#include "ResourceObject.hpp"
#include "resource_types.h"

namespace OIC { namespace Service {

    /**
     * System FSM machine
     */
    enum class SystemState
    {
        IDLE,
        SAVE_TEMPEARTURE,
        ALARM,
        ALARM_STARTED,
        ALARM_STOPPED_THREAD,
        ALARM_STOPPED
    };

    enum class LightState
    {
        ON,
        OFF
    };

    const std::string HOST_TAG = "/hosting";
    const auto HOSTING_TAG_SIZE = HOST_TAG.size();

    const static std::string GRAPH_URI = "/oic/graph";
    const static std::string TEMPERATURE_URI = "/arduino/temperatureSensor";

    const static double TEMPERATURE_UPPER_THRES = 30;  // Degrees
    const static double TEMPERATURE_LOWER_THRES = 28.8;  // Degrees

    const static int TEMPERATURE_THREAD_FREQ_HZ   = 1; // HERTZ
    const static int TEMPERATURE_THREAD_FREQ_MS   = (1 / TEMPERATURE_THREAD_FREQ_HZ) * 1000; // Ms


    class BuildingController
    {
    public:
        typedef std::unique_ptr<BuildingController> Ptr;
        typedef std::unique_ptr<const BuildingController> ConstPtr;

    public:
        /**
         * @brief getInstance
         * @return
         */
        static BuildingController* getInstance();

        /**
          *
          */
        ~BuildingController();

        /**
         * @brief start
         * @return
         */
        OCStackResult start();

        /**
         * @brief stop
         * @return
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


        /**
         * @brief setMaxTemperatureThreshold Set min and max threshold
         * @param threshold
         */
        void setMaxTemperatureThreshold(const double &threshold);
        void setMinTemperatureThreshold(const double &threshold);

        /**
         * @brief getMaxTemperatureThreshold
         * @return
         */
        double getMaxTemperatureThreshold();
        double getMinTemperatureThreshold();
    private:
        /**
          * Map containing all discovered resources.
          */
        std::unordered_map<std::string, ResourceObject::Ptr> m_resourceList;

        /**
         * Resource used to notify when to store a new graph
         */
        RCSRemoteResourceObject::Ptr m_graphResource;

        /**
          * Temperature Resource
          */
        RCSRemoteResourceObject::Ptr m_temperatureResource;

        /**
          * Mutex locking a discovered resource until it has been added to the map.
          */
        std::mutex m_resourceMutex;
        std::mutex m_fsmMutex;

        /**
          * DiscoveryTask used to cancel and observe the discovery process.
          */
        RCSDiscoveryManager::DiscoveryTask::Ptr m_discoveryTask;
        RCSDiscoveryManager::ResourceDiscoveredCallback m_discoveryCallback;

        /**
         * @brief m_sceneCollection
         * Collection of the scene. In this case the office
         */
        SceneCollection::Ptr m_sceneCollection;

        /**
         * @brief m_sceneStart
         * Scene environments with the specified actions for each individual resource
         */
        Scene::Ptr m_sceneLightsOn;
        Scene::Ptr m_sceneLightsOff;

        /**
         * @brief m_systemState Current active scene state.
         */
        SystemState m_systemState;

        /**
         * State of the led lights
         */
        LightState m_lightState;

        /**
          * Callback inovked during a change in a registered resource;
          */
        ResourceObject::ResourceObjectCacheCallback m_resourceObjectCacheCallback;
        ResourceObject::ResourceObjectStateCallback m_resourceObjectStateCallback;

        /**
          * Min and Max threshold
          */
        double m_maxThreshold;
        double m_minThreshold;

    private:

        /**
         * @brief BuildingController
         */
        BuildingController();

        /**
         * @brief stateMachineThread
         */
        void checkStateMachine();

        /**
         * @brief toggleLights
         * @param onTimeMs
         * @param offTimeMs
         */
        void toggleLights(int onTimeMs, int offTimeMs, SystemState state);

        /**
          * @brief Function callback for found resources
          *
          * @param resource     The discovered resource.
          */
        void foundResourceCallback(std::shared_ptr<RCSRemoteResourceObject> resource);

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

        /**
         * @brief printAttributes Prints the attributes
         *
         * @param attrs
         */
        void printAttributes(const RCSResourceAttributes &attrs);

        /**
         * @brief getTemperatureLoop Continuously get the temperature readings
         */
        void getTemperatureLoop();

        /**
         * @brief onGetTemperatureReading GET response of temperature reading
         *
         * @param attrs attributes
         * @param eCode result of GEt
         */
        void onGetTemperatureReading(const RCSResourceAttributes &attrs, int eCode);

    };



} }




#endif /* _BUILDINGCONTROLLER_H_ */
