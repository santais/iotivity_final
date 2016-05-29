#ifndef DISCOVERYMANAGER
#define DISCOVERYMANAGER

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

// Scene-manager
#include "SceneList.h"


namespace OIC { namespace Service
{

    typedef long long cbTimer;

    /**
     * @brief The DiscoveryManagerInfo class
     */
    class DiscoveryManagerInfo
    {
    public:
        /**
         * @brief DiscoveryManagerInfo
         */
        DiscoveryManagerInfo();

        /**
         * @brief DiscoveryManagerInfo
         * @param host
         * @param uri
         * @param types
         * @param cb
         */
        DiscoveryManagerInfo(const std::string& host, const std::string& uri,
                             const std::vector<std::string>& types, FindCallback cb, OCConnectivityType connectivityType);


        /**
         * @brief discover
         */
        void discover() const;

    private:
        std::string m_host;
        std::string m_relativeUri;
        std::vector<std::string> m_resourceTypes;
        FindCallback m_discoveryCb;
        OCConnectivityType m_connectivityType;
    };

    /**
     * @brief The DiscoveryManager class
     *
     * Discovers resource on the network
     */
    class DiscoveryManager
    {

    public:
        /**
         * @brief DiscoveryManager
         * @param time_ms
         */
        DiscoveryManager()                                            = default;
        DiscoveryManager(cbTimer timeMs);
        //DiscoveryManager(const DiscoveryManager& dm)                = default;
        //DiscoveryManager(DiscoveryManager&& dm)                     = default;
        //DiscoveryManager& operator=(const DiscoveryManager& dm)     = default;
        //DiscoveryManager& operator=(DiscoveryManager&& dm)          = default;

        ~DiscoveryManager();

        /**
         * @brief isSearching
         * @return
         */
        bool isSearching() const;

        /**
         * @brief cancel
         */
        void cancel();

        /**
         * @brief setTimer
         * @param time_ms
         */
        void setTimer(cbTimer time_ms);

        /**
         * @brief discoverResource
         * @param types
         * @param cb
         * @param host
         */
        void discoverResource(const std::string& uri, const std::vector<std::string>& types, FindCallback cb,
                              std::string host = "", OCConnectivityType connectivityType = CT_DEFAULT);

        /**
         * @brief discoverResource
         * @param type
         * @param cb
         * @param host
         */
        void discoverResource(const std::string& uri, const std::string& type, FindCallback cb,
                              std::string host = "", OCConnectivityType connectivityType = CT_DEFAULT);
    private:
        /**
         * @brief m_timer
         */
        ExpiryTimer m_timer;

        /**
         * @brief m_timerMs
         */
        cbTimer m_timerMs;

        /**
         * @brief m_isRunning
         */
        bool m_isRunning;

        /**
         * @brief m_discoveryInfo
         */
        DiscoveryManagerInfo m_discoveryInfo;

        /**
         * @brief m_cancelMutex
         */
        std::mutex m_discoveryMutex;

    private:

        /**
         * @brief timeOutCB
         * @param id
         */
        void timeOutCB();
    };
} }



#endif // DISCOVERYMANAGER

