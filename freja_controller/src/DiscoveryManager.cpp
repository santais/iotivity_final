#include "DiscoveryManager.h"

namespace OIC { namespace Service
{
    constexpr unsigned int CONTROLLER_POLLING_DISCOVERY_MS = 5000; // in milliseconds

/********************************** DiscoveryMangerInfo *************************************/

    /**
     * @brief Controller::DiscoveryManagerInfo::DiscoveryManagerInfo
     */
    DiscoveryManagerInfo::DiscoveryManagerInfo()
    {
        ;
    }

    /**
     * @brief Controller::DiscoveryManagerInfo::DiscoveryManagerInfo
     * @param host
     * @param uri
     * @param types
     * @param cb
     */
    DiscoveryManagerInfo::DiscoveryManagerInfo(const string&host, const string& uri, const std::vector<std::string>& types, FindCallback cb,
                                               OCConnectivityType connectivityType)
        : m_host(host),
          m_relativeUri(uri),
          m_resourceTypes(std::move(types)),
          m_discoveryCb(std::move(cb)),
          m_connectivityType(connectivityType)
    {
        ;
    }

    /**
     * @brief Controller::DiscoveryManagerInfo::discover
     */
    void DiscoveryManagerInfo::discover() const
    {
        for(auto& type : m_resourceTypes)
        {
            OC::OCPlatform::findResource(m_host, m_relativeUri + "?rt=" + type, m_connectivityType, m_discoveryCb, QualityOfService::NaQos);
        }
    }


/********************************** DsicoveryManager *************************************/

    /**
     * @brief Controller::DiscoveryManager::DiscoveryManager
     * @param time_ms
     */
    DiscoveryManager::DiscoveryManager(cbTimer time_ms) : m_timerMs(time_ms), m_isRunning(false){}


    /**
     * @brief Controller::DiscoveryManager::~DiscoveryManager
     */
    DiscoveryManager::~DiscoveryManager()
    {

    }

    /**
     * @brief isSearching
     * @return
     */
    bool DiscoveryManager::isSearching() const
    {
        return m_isRunning;
    }

    /**
     * @brief cancel
     */
    void DiscoveryManager::cancel()
    {
        std::lock_guard<std::mutex> lock(m_discoveryMutex);
        if(m_isRunning)
        {
           m_isRunning = false;
        }
    }

    /**
     * @brief setTimer
     * @param time_ms
     */
    void DiscoveryManager::setTimer(const cbTimer time_ms)
    {
        m_timerMs = time_ms;
    }

    /**
     * @brief discoverResource
     * @param types
     * @param cb
     * @param host
     */
    void DiscoveryManager::discoverResource(const std::string& uri, const std::vector<std::string>& types, FindCallback cb,
                                std::string host, OCConnectivityType connectivityType)
    {
        std::lock_guard<std::mutex> lock(m_discoveryMutex);

        m_isRunning = true;

        DiscoveryManagerInfo discoveryInfo(host, uri.empty() ? OC_RSRVD_WELL_KNOWN_URI : uri, types,
                                           std::move(cb), connectivityType);

        m_discoveryInfo = std::move(discoveryInfo);

        m_discoveryInfo.discover();

        m_timer.post(m_timerMs, std::bind(&DiscoveryManager::timeOutCB, this));
    }

    /**
     * @brief discoverResource
     * @param type
     * @param cb
     * @param host
     */
    void DiscoveryManager::discoverResource(const std::string& uri, const std::string& type, FindCallback cb,
                                std::string host, OCConnectivityType connectivityType)
    {
        std::lock_guard<std::mutex> lock(m_discoveryMutex);

        m_isRunning = true;

        DiscoveryManagerInfo discoveryInfo(host, uri.empty() ? OC_RSRVD_WELL_KNOWN_URI : uri, std::vector<std::string> { type },
                                           std::move(cb), connectivityType);

        m_discoveryInfo = std::move(discoveryInfo);

        m_discoveryInfo.discover();

        // DEBUG
        std::cout << "Starting timer for DiscoveryManager with timer: " << m_timerMs << std::endl;
        m_timer.post(m_timerMs, std::bind(&DiscoveryManager::timeOutCB, this));
    }


    /**
     * @brief timeOutCB
     * @param id
     */
    void DiscoveryManager::timeOutCB()
    {
        // Check if the mutex is free
        std::lock_guard<std::mutex> lock(m_discoveryMutex);

        // Only restartt he callback timer if the process has not been stopped.
        if(m_isRunning)
        {
            m_discoveryInfo.discover();

            m_timer.post(m_timerMs, std::bind(&DiscoveryManager::timeOutCB, this));
        }
    }
} }
