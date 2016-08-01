#ifndef RPIRCSCONTROLLER_H_
#define RPIRCSCONTROLLER_H_

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

/**
 * @brief Controller FSM states.
 */
enum class ControllerState {
    IDLE                   = 0,
    AUTOMATIC_GAME_ON      = 1,
    MANUAL_LED_CONTROL     = 2,
    PLAY_LED_SEQUENCE      = 3
};

/**
 * @brief Different available LED sequences
 */
enum class SequenceState {
    IDLE         = 0,
    BOWLING      = 1,
    PULSE_UP     = 2,
    COLOR_SHOW   = 3
};

/**
 * @brief Private variables for RPIBeerPongController
 */
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

    // Const vector values
    static const std::vector<int> CUP_OFF_RGB_VALUES = {0, 4095, 0};
    static const std::vector<int> CUP_ON_RGB_VALUES  = {4095, 0, 0};

    // URI
    static const std::string CONTROLLER_URI = "/rpi/controller";

    // Resource Type
    static const std::string CONTROLLER_TYPE = "rpi.d.controller";

    // Attributes
    static const std::string STATE_NAME = "controllerState";
    static const std::string SEQUENCE_NAME = "sequence";
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

    /**
     * @brief Get the current control state
     *
     * @return
     */
    ControllerState getControllerState();

private:

    // RCSRPIController resource to control controller state
    RPIRCSResourceObject::Ptr m_resource;

    // Segment 7 Controller
    Segment7* m_segmentController;

    // Vectors which contains the Beer Pong resources
    std::array<PCA9685LEDResource::Ptr, NUM_OF_SIDE_RESOURCES> m_redSideResources;
    std::array<PCA9685LEDResource::Ptr, NUM_OF_SIDE_RESOURCES> m_blueSideResources;

    // Vector which contains the 7segment display resources
    std::array<RPI7SegmentResource::Ptr, NUM_OF_SEGMENT_PAIRS> m_7SegmentResources;

    /**
     * @brief Current control state of the controller
     */
    ControllerState m_controllerState;

    /**
     * @brief Current Sequence state
     */
    SequenceState m_sequenceState;

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
     * @brief Initialize the RPIResource object
     */
    void initializeRPIResource();

    /**
     * @brief Callback initiated when a change to one of the input
     *        sensors are registered and the segment value is changed
     *
     * @param segment   The segment which has changed its value
     */
    void segmentCallback(Segment7* segment, uint16_t* inputData);

    /**
     * @brief setGameLEDLight
     */
    void setGameLEDLight(PCA9685LEDResource::Ptr, bool state);

    /**
     * @brief Handler to receive incoming POST requests
     *
     * @param request
     * @param attr
     */
    void setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr);

    /**
     * @brief Set the attribute sof the m_resource object
     */
    void setAttributes();

    /**
     * @brief Set the control state
     *
     * @param state
     * @param sequence
     */
    void setControlState(ControllerState state, SequenceState sequence);

public:
    RPIBeerPongController(RPIBeerPongController const&)   = delete;   // No overloading allowed
    void operator=(RPIBeerPongController const&)     = delete;   // No assigning allowed

};

#endif // RPIRCSCONTROLLER_H
