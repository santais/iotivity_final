#include "BuildingController.h"

#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <signal.h>
#include <unistd.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OIC;
using namespace Service;

BuildingController::Ptr controller;


/*
* This is a signal handling function for SIGINT(CTRL+C).
* A Resource Coordinator handle the SIGINT signal for safe exit.
*
* @param[in] signal
*                 signal number of caught signal.
*/
int g_quitFlag = 0;

void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        g_quitFlag = 1;
    }
}

int main(int argc, char** argv)
{
    std::cout << "Starting test program" << std::endl;
    controller = BuildingController::Ptr(BuildingController::getInstance());

    // Set max and minimum threshold
    if(argc > 1)
    {
        if(argv[1])
        {
            std::cout << "Min temperature is set to: " << argv[1] << std::endl;
            controller->setMinTemperatureThreshold(static_cast<double>(atof(argv[1])));
        }
        if(argv[2])
        {
            std::cout << "Max temperature is set to " << argv[2] << std::endl;
            controller->setMaxTemperatureThreshold(static_cast<double>(atof(argv[2])));
        }
    }
    controller->start();
    signal(SIGINT, handleSigInt);
    while (!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            controller->stop();
            std::cout << "OCStack process error" << std::endl;
            return 0;
        }
        std::this_thread::sleep_for(chrono::milliseconds(10));
    }

    controller->stop();

    return 0;
}

