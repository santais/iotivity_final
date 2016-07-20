#ifndef RPIRCSCONTROLLER_H
#define RPIRCSCONTROLLER_H

#include "PCA9685LEDResource.h"
#include "RPI7SegmentResource.h"
#include "RPIRCSResourceObject.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"

// Drivers
#include "segment7_controller.h"
#include "PCA9685RPi.h"

#include <vector>
#include <iostream>
#include <functional>

// Used to use a C++ callback to a C callback
template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
   template <typename... Args>
   static Ret callback(Args... args) {
      func(args...);
   }
   static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

namespace
{
    // 10 cups on each side + 1 for the water cleaner
    static const int NUM_OF_SIDE_RESOURCES = 11;

    // Number of segment pairs on each side. 2 x 2 for 10 digits
    static const int NUM_OF_SEGMENT_PAIRS = 2;

    // I2C Addresses for the PCA9685 modules
    static const int I2CADDRESS_BLUE_1 = 64;
    static const int I2CADDRESS_BLUE_2 = 65;
    static const int I2CADDRESS_BLUE_3 = 66;

    static const int I2CADDRESS_RED_1 = 67;
    static const int I2CADDRESS_RED_2 = 68;
    static const int I2CADDRESS_RED_3 = 69;

    // I2C Address PWM Frequency HZ (24 - 1524 range)
    static const int I2C_FREQUENCY_HZ = 500;



}

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
    // Segment 7 Controller
    Segment7* m_segmentController;

    // Vectors which contains the Beer Pong resources
    std::array<PCA9685LEDResource::Ptr, NUM_OF_SIDE_RESOURCES> m_redSideResources;
    std::array<PCA9685LEDResource::Ptr, NUM_OF_SIDE_RESOURCES> m_blueSideResources;

    // Vector which contains the 7segment display resources
    std::array<RPI7SegmentResource::Ptr, NUM_OF_SEGMENT_PAIRS> m_7SegmentResources;

private:
    RPIBeerPongController();

    /**
     * @brief Initializes the NUM_OF_SIDE_RESOURCES * 2 cup resources
     */
    void initializeCupResources();

    /**
     * @brief Initializes the NUM_OF_SEGMENT_PAIRS resources
     */
    void initializeSegmentResources();

    /**
     * @brief Callback initiated when a change to one of the input
     *        sensors are registered and the segment value is changed
     *
     * @param segment   The segment which has changed its value
     */
    void segmentCallback(Segment7* segment, uint16_t* inputData);

    //static void staticCallback(Segment7* segment);


public:
    RPIBeerPongController(RPIBeerPongController const&)   = delete;   // No overloading allowed
    void operator=(RPIBeerPongController const&)     = delete;   // No assigning allowed


};

#endif // RPIRCSCONTROLLER_H
