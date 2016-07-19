#include "RPIBeerPongController.h"


RPIBeerPongController::RPIBeerPongController()
{
    // Create the resource

}

RPIBeerPongController* RPIBeerPongController::getInstance()
{
    static RPIBeerPongController* instance(new RPIBeerPongController);
    return instance;
}

RPIBeerPongController::~RPIBeerPongController()
{
    ;
}
