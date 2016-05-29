#include "RPIRCSController.h"

#include "OCPlatform.h"
#include "OCApi.h"

#include <signal.h>
#include <unistd.h>

int g_quitFlag = false;

void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        g_quitFlag = 1;
    }
}

int main()
{
    RPIRCSController::Ptr controller = RPIRCSController::Ptr(RPIRCSController::getInstance());

    signal(SIGINT, handleSigInt);
    while(!g_quitFlag)
    {
        if(OCProcess() != OC_STACK_OK)
        {
            return 0;
        }
        sleep(1);
    }

    return 0;


}
