#ifndef RPIRCSCONTROLLER_H
#define RPIRCSCONTROLLER_H

#include "RPIRCSResourceObject.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"

// Drivers
#include "segment7_controller.h"
#include "sn74hc165.h"
#include "sn74hc595.h"
#include "PCA9685RPi.h"

#include <vector>
#include <iostream>
#include <functional>

class RPIBeerPongController
{
public:
    typedef std::unique_ptr<RPIBeerPongController> Ptr;
    typedef std::unique_ptr<const RPIBeerPongController> ConstPtr;
public:
    /**
     * @brief getInstance Singleton instance of the class
     * @return
     */
    static RPIBeerPongController* getInstance();

    ~RPIBeerPongController();

private:

    // SN74HC165 8 Parallel in Serial Out Driver Module
    SN74HC165* m_SN74HC165;

    // SN74HC595 8 Paralle out Serial In Driver Module
    SN74HC595* m_SN74HC595;

private:

    RPIBeerPongController();

public:

    RPIBeerPongController(RPIBeerPongController const&)   = delete;   // No overloading allowed
    void operator=(RPIBeerPongController const&)     = delete;   // No assigning allowed


};

#endif // RPIRCSCONTROLLER_H
