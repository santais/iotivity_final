#include "RPIBeerPongController.h"

std::vector<int> DEFAULT_RGB_VALUE = {4095, 0, 0};


RPIBeerPongController::RPIBeerPongController() : m_controllerState(ControllerState::AUTOMATIC_GAME_ON),
    m_sequenceState(SequenceState::BOWLING)
{
    // Initialize the vectors
    std::for_each(m_redSideResources.begin(), m_redSideResources.end(), [](PCA9685LEDResource::Ptr &resource)
        {resource = nullptr;});

    std::for_each(m_blueSideResources.begin(), m_blueSideResources.end(), [](PCA9685LEDResource::Ptr &resource)
        {resource = nullptr;});

    // Initialize the PCA9685 module
    PCA9685Setup(I2CADDRESS_BLUE_1, I2C_FREQUENCY_HZ);

    // Initialize the cup resources
    this->initializeCupResources();

    // Initialize the 7 segment resources
    this->initializeSegmentResources();

    // Initialize the RPIRcsresource
    this->initializeRPIResource();

    // Setup the Segment7 Controller callback
    Callback<void(Segment7*, uint16_t*)>::func = std::bind(&RPIBeerPongController::segmentCallback, this, std::placeholders::_1, std::placeholders::_2);
    SegmentValueCallback func = static_cast<SegmentValueCallback>(Callback<void(Segment7*, uint16_t*)>::callback);

    if(segment7Setup(NUM_OF_SEGMENT_PAIRS, func) < 0)
    {
        std::cerr << "Failed to setup the 7segment driver" << std::endl;
        return;
    }


    // Start and store the linked list
    m_segmentController = getSegment7List();

    // Start the segment
    startSegment7();
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

/**
 * @brief Get the current control state
 *
 * @return
 */
ControllerState RPIBeerPongController::getControllerState()
{
    return m_controllerState;
}


/**
 * @brief Initializes the NUM_OF_SIDE_RESOURCES * 2 cup resources
 */
void RPIBeerPongController::initializeCupResources()
{
    // 10 cups are to be initialized = 10 RPi resources.
    /*  Cups setup - ID */
    /* 9  8  7  6 */
    /*  5  4   3  */
    /*   2   1    */
    /*     0      */


    // A maximum of 5 cups are possible per PCA9865 Module.
    // Requires a total of 2 modules per side
    int waterCupPins = 0;
    std::stringstream waterStringStream;

    // BLUE SIDE
    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcablue/led/" << i;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_BLUE_1, I2C_FREQUENCY_HZ);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        m_blueSideResources[i] = resource;
        resource->setRGBValues(DEFAULT_RGB_VALUE);
    }

    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcablue/led/" << i + 5;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_BLUE_2, I2C_FREQUENCY_HZ);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        m_blueSideResources[i + 5] = resource;
        resource->setRGBValues(DEFAULT_RGB_VALUE);
    }

    // Water Resource
    waterStringStream << "/pcablue/led/" << NUM_OF_SIDE_RESOURCES - 1;
    PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(waterStringStream.str(), I2CADDRESS_BLUE_3, I2C_FREQUENCY_HZ);
    resource->createResource(waterCupPins++, waterCupPins++, waterCupPins++);
    m_blueSideResources[NUM_OF_SIDE_RESOURCES - 1] = resource;

    // RED SIDE
    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcared/led/" << i;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_RED_1, I2C_FREQUENCY_HZ);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        m_redSideResources[i] = resource;
	resource->setRGBValues(DEFAULT_RGB_VALUE);
    }

    for(size_t i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << "/pcared/led/" << i + 5;
        PCA9685LEDResource::Ptr resource = std::make_shared<PCA9685LEDResource>(ss.str(), I2CADDRESS_RED_2, I2C_FREQUENCY_HZ);
        resource->createResource(i*3, (i*3) + 1, (i*3) + 2);
        std::cout << "Created resource with uri : " << ss.str() << " and pins: " <<
                     i*3 << " " << (i*3) + 1 << " " << (i*3) + 2 << std::endl;

        m_redSideResources[i + 5] = resource;
	resource->setRGBValues(DEFAULT_RGB_VALUE);
    }


    // Water Resource
    waterStringStream.str(std::string());
    waterStringStream.clear();
    waterStringStream << "/pcared/led/" << NUM_OF_SIDE_RESOURCES - 1;
    resource = std::make_shared<PCA9685LEDResource>(waterStringStream.str(), I2CADDRESS_BLUE_3, I2C_FREQUENCY_HZ);
    resource->createResource(waterCupPins++, waterCupPins++, waterCupPins++);
    m_redSideResources[NUM_OF_SIDE_RESOURCES - 1] = resource;

}

/**
 * @brief Initializes the NUM_OF_SEGMENT_PAIRS resources
 */
void RPIBeerPongController::initializeSegmentResources()
{
    for(int i = 0; i < NUM_OF_SEGMENT_PAIRS; i++)
    {
        std::stringstream ss;
        ss << "/segment/" << i;
        RPI7SegmentResource::Ptr resource = std::make_shared<RPI7SegmentResource>(ss.str(), i);
        resource->createResource();
        m_7SegmentResources[i] = resource;
    }
}

/**
 * @brief Initialize the RPIResource object
 */
void RPIBeerPongController::initializeRPIResource()
{
    // Using default parameters
    const std::string resourceType = CONTROLLER_TYPE;

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(CONTROLLER_URI,
                            std::vector<std::string>{resourceType, "oic.d.light"}, std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT,
                                                                "oic.if.rw", "oic.if.a"})));

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&RPIBeerPongController::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));

    // Set the attributes
    this->setAttributes();
}

/**
 * @brief Callback initiated when a change to one of the input
 *        sensors are registered and the segment value is changed
 *
 * @param segment   The segment which has changed its value
 */
void RPIBeerPongController::segmentCallback(Segment7* segment, uint16_t* inputData)
{
    int id = static_cast<int>(segment->id);
    int value = static_cast<int>(segment->value);

    if(segment == NULL)
    {
        std::cerr << "Segment is null! returning" << std::endl;
        return;
    }

    std::cout << "Before id check" << std::endl;
    if(segment->id > NUM_OF_SEGMENT_PAIRS)
    {
        std::cout << "Unknown id" << std::endl;
        return;
    }

    std::cout << "Id: " << id << " " << "Value: " << value << std::endl;

    RPI7SegmentResource::Ptr resource = m_7SegmentResources[id];

    if(resource == nullptr)
    {
        std::cerr << "The found resource is a nullptr" << std::endl;
        return;
    }

    resource->setSegmentValue(value);

    // Check if the state has changed for the particular LED
    for(int i = 0; i < NUM_OF_SIDE_RESOURCES - 1; i++)
    {
        bool state = static_cast<bool>(*inputData & (1 << i));

        bool attribute = false;
        RCSResourceObject::Ptr RCSResource = nullptr;
        PCA9685LEDResource::Ptr PCA9685Resource = nullptr;

        if(segment->id == 0)
        {
            RCSResource = m_blueSideResources[i]->getResourceObject()->getResourceObject();
            PCA9685Resource = m_blueSideResources[i];
        }
        else if(segment->id == 1)
        {
            RCSResource = m_redSideResources[i]->getResourceObject()->getResourceObject();
            PCA9685Resource = m_redSideResources[i];
        }

        if(RCSResource != nullptr && PCA9685Resource != nullptr)
        {
            attribute = RCSResource->getAttribute<bool>("state");
            RCSResourceAttributes::Value RCSValue(attribute);

            if(state != attribute)
            {
                std::cout << "Setting cup: " << PCA9685Resource->getUri() << " state to: " << state << std::endl;

                RCSResource->setAttribute("state", state);
                RCSResource->notify();

                // Set the RGB Values
                this->setGameLEDLight(PCA9685Resource, state);
            }
        }
    }
}


/**
 * @brief setGameLEDLight
 */
void RPIBeerPongController::setGameLEDLight(PCA9685LEDResource::Ptr resource, bool state)
{
    if(state)
    {
        resource->setRGBValues(CUP_ON_RGB_VALUES);
    }
    else
    {
        resource->setRGBValues(CUP_OFF_RGB_VALUES);
    }
}


/**
 * @brief Handler to receive incoming POST requests
 *
 * @param request
 * @param attr
 */
void RPIBeerPongController::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    std::cout << "Inside " << __func__ << std::endl;
    ControllerState controllerState = ControllerState::IDLE;
    SequenceState sequenceState     = SequenceState::IDLE;

    for(const auto &attr : attrs)
    {
        if(attr.key().compare(STATE_NAME) == 0)
        {
            // Set the new control state
            std::cout << "Setting STATE" << std::endl;
            controllerState = static_cast<ControllerState>(attr.value().get<int>());

        }
        else if(attr.key().compare(SEQUENCE_NAME) == 0)
        {
            std::cout << "Setting SEQUENCE" << std::endl;
            sequenceState = static_cast<SequenceState>(attr.value().get<int>());
        }
        else
        {
            std::cerr << "Invalid key" << std::endl;
        }
    }


    /*if(controllerState != ControllerState::IDLE && sequenceState != SequenceState::IDLE)
    {*/
        std::cout << "Setting state" << std::endl;
        this->setControlState(controllerState, sequenceState);
    //}

    std::cout << "Leaving: " << __func__ << std::endl;
}

/**
 * @brief Set the attribute sof the m_resource object
 */
void RPIBeerPongController::setAttributes()
{
    const int controllerState = static_cast<int>(m_controllerState);
    RCSResourceAttributes::Value RCSValue(controllerState);
    m_resource->addAttribute(STATE_NAME, RCSValue);

    // Add the sequence state
    const int sequnceState = static_cast<int>(m_sequenceState);
    RCSValue = RCSResourceAttributes::Value(sequnceState);
    m_resource->addAttribute(SEQUENCE_NAME, RCSValue);

}


/**
 * @brief Set the control state
 *
 * @param state
 * @param sequence
 */
void RPIBeerPongController::setControlState(ControllerState state, SequenceState sequence)
{
    std::cout << __func__ << std::endl;
    switch(state)
    {
    case ControllerState::AUTOMATIC_GAME_ON:
        // Start the segment 7 driver
        std::cout << "Setting state to AUTOMATIC" << std::endl;
        startSegment7();
        break;

    case ControllerState::MANUAL_LED_CONTROL:
        // Stop the segment 7 driver
        std::cout << "Setting state to MANUAL" << std::endl;
        stopSegment7();
        break;

    case ControllerState::PLAY_LED_SEQUENCE:
    {
        // Stores the previous state which will be active after the sequence is finished
        ControllerState prevControllerState = m_controllerState;
        m_controllerState = state;
        // Run the sequence
        // TODO: INSERT SEQUENCES

        // Overwrite the state with the current run.
        state = prevControllerState;

        break;
    }
    }

    m_controllerState = state;
    m_sequenceState = sequence;
}
