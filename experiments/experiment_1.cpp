#include "RCSRemoteResourceObject.h"
#include "RCSDiscoveryManager.h"
#include "PrimitiveResource.h"
#include "RCSResourceObject.h"

#include <chrono>
#include <thread>
#include <iostream>

#define MAX_TIMEOUT_SECONDS 0.5
#define MAX_TIMEOUT_MILLISECONDS MAX_TIMEOUT_SECONDS * 1000
#define MAX_TIMEOUT_MICROSECONDS MAX_TIMEOUT_MILLISECONDS * 1000

#define NUMBER_OF_ITERATIONS 50

typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::microseconds microseconds;

using namespace OIC;
using namespace OIC::Service;

// Discovery Task
RCSDiscoveryManager::DiscoveryTask::Ptr g_discoveryTask;

// Termination criteria
bool g_terminateDiscovery = false;

// Mutex for time reading
std::mutex g_timeMutex;

// Timers
double g_beginTimer = 0;
double g_startTimer = 0;
double g_stopTimer = 0;

// Resource counter
int g_discoveredResources = 0;

std::unordered_map<std::string, RCSRemoteResourceObject::Ptr> g_foundResourceMap;

double getChronoTimeNowAsDouble()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ms = std::chrono::time_point_cast<microseconds>(now);
    auto epoch = now_ms.time_since_epoch();

    return(std::chrono::duration_cast<microseconds>(epoch).count());
}

void foundResourceCallback(std::shared_ptr<RCSRemoteResourceObject> resource)
{
    std::string key = resource->getUri() + resource->getAddress();

    if(g_foundResourceMap.insert({key, resource}).second)
    {
        g_timeMutex.lock();
        g_discoveredResources++;
        //std::cout << "Discovered resources: " << g_discoveredResources << std::endl;

        // Restart timer
        g_startTimer = getChronoTimeNowAsDouble();
        g_timeMutex.unlock();
    }
}


int main() 
{
    int iterations = 0;
    double timeDifference = 0;

    std::cout << "Click any key to start" << std::endl;

    std::string input;
    std::cin >> input;

    std::cout << "Starting Discovery" << std::endl;

    g_discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                                        "oic.d.light", foundResourceCallback);

    // Start the timer
    g_startTimer = g_beginTimer = getChronoTimeNowAsDouble();
    std::cout << "Start timer:  " << g_beginTimer << " μs" << std::endl;

    while(iterations < NUMBER_OF_ITERATIONS)
    {
        while(!g_terminateDiscovery)
        {
            g_stopTimer = getChronoTimeNowAsDouble();
            g_timeMutex.lock();
            double difference = g_stopTimer - g_startTimer;
            g_timeMutex.unlock();

            if(difference >= MAX_TIMEOUT_MICROSECONDS)
            {
                g_terminateDiscovery = true;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        // Calculate total time
        std::cout << "Total discovery time: " << g_startTimer - g_beginTimer << " μs" << std::endl;
        g_discoveryTask->cancel();
        g_foundResourceMap.clear();

        // Save the time difference
        timeDifference += g_startTimer - g_beginTimer;

        // Restart the timers and while condition
        g_terminateDiscovery = false;
        g_startTimer = g_beginTimer = getChronoTimeNowAsDouble();
        g_discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                                            "oic.d.light", foundResourceCallback);

        // Iterate
        iterations++;
    }

    timeDifference /= NUMBER_OF_ITERATIONS;

    std::cout << "\n\n Total average discovery time: " << timeDifference;

	return 0;
}
