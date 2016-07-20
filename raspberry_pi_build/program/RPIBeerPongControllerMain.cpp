#include "RPIBeerPongController.h"

#include <signal.h>
#include <unistd.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OIC;
using namespace Service;

RPIBeerPongController::Ptr controller;

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


int main()
{
    std::cout << "Starting test program" << std::endl;
    controller = RPIBeerPongController::Ptr(RPIBeerPongController::getInstance());

    static char buffer[50] = {};

    while (!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            return 0;
        }

        uint8_t data[4] = {};
        int len, val;

        printf("Input 4 new data values. \n");

        for(size_t i = 0; i < 4; i++)
        {
            fgets(buffer, 50, stdin);

            len = strlen(buffer) - 1;

            for(size_t i = 0; i < len; ++i)
            {
                if(!isdigit(buffer[i]))
                {
                    printf("Invalid input");
                    return 1;
                }
            }
            val = atoi(buffer);
            data[i] = val;
        }


        setTestData(data);

        sleep(1);
    }

	return 0;
}
