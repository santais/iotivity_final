#include "SpeakerResource.h"
#include "resource_types.h"

SpeakerResource::SpeakerResource() : m_uri("/a/speaker"), m_audioRunning(false)
{
#ifdef ARM
    #ifdef __linux__
    system("lsmod");
    system("amixer cset numid=1");
    #else
        std::cerr << "System is not linux!" << std::endl;
    #endif
#endif
}

/**
 * @brief SpeakerResource::SpeakerResource
 *
 * @param portPin
 * @param uri       Uri of the new light resource
 */
SpeakerResource::SpeakerResource(const std::string &uri) : m_audioRunning(false)
{
    m_uri = uri;
#ifdef ARM
    #ifdef __linux__
    system("lsmod");
    system("amixer cset numid=1");
    #else
        std::cerr << "System is not linux!" << std::endl;
    #endif
#endif
}

SpeakerResource::~SpeakerResource()
{
    m_resource.reset();
}

SpeakerResource::SpeakerResource(const SpeakerResource& resource) :
    m_resource(resource.m_resource),
    m_uri(resource.m_uri)
{}

SpeakerResource::SpeakerResource(SpeakerResource&& resource) :
    m_resource(std::move(resource.m_resource)),
    m_uri(std::move(resource.m_uri))
{}

SpeakerResource& SpeakerResource::operator=(const SpeakerResource& resource)
{
    m_resource = resource.m_resource;
    m_uri = resource.m_uri;
}

SpeakerResource& SpeakerResource::operator=(SpeakerResource&& resource)
{
    m_resource = std::move(resource.m_resource);
    m_uri = std::move(resource.m_uri);
}


RPIRCSResourceObject::Ptr SpeakerResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int SpeakerResource::createResource()
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resource_types = {OIC_DEVICE_SPEAKER, OIC_TYPE_AUDIO, OIC_TYPE_BINARY_SWITCH};

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resource_types), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT,
                                                                 "oic.if.a", "oic.if.rw"})));

    m_resource->createResource(true, true, false);

    m_resource->setReqHandler(std::bind(&SpeakerResource::setRequestHandler, this, std::placeholders::_1,
                                        std::placeholders::_2));


    // Set the attributes
    this->setAttributes();

    return 1;
}

void SpeakerResource::setUri(std::string& uri)
{
    m_uri = uri;
}

std::string SpeakerResource::getUri()
{
    return m_uri;
}

/**
 * @brief setRequestHandler
 *
 * @param request
 * @param attr
 */
void SpeakerResource::setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attrs)
{
    // Lookup the state attribute
    std::cout << "============================================ \n";

    for(const auto& attr : attrs)
    {
        std::string attrString = attr.value().toString();
       // std::cout << attrString << std::endl;
       // std::cout << attr.key() << std::endl;

        AttributeType type = getAttributeType(attr.key());
        switch(type)
        {
        case AttributeType::STATE:
            std::cout << "Not yet supported" << std::endl;
            break;
        case AttributeType::MUTE:

            if(attr.value().get<bool>())
            {
#ifdef __linux__
                std::cout << "Mute speakers" << std::endl;
                // Mute speakers
#ifdef ARM
            //system("amixer cset numid=1 -- 0%");
            system("amixer -D pulse sset Master mute");
#else
            system("amixer sset Master mute");
#endif
            }
            else
            {
#ifdef ARM
            //system("amixer cset numid=1 -- 0%");
            system("amixer -D pulse sset Master unmute");
#else
            system("amixer sset Master unmute");
            system("amixer sset PCM unmute");
#endif
#else
            std::cerr << "Unsupported Operating System" << std::endl;
#endif
            }

            break;
        case AttributeType::SOUND:
        {
            int type = attr.value().get<int>();
            switch(type)
            {
            case ALARM_ACTIVE:
            {
                // Start thread
                if(!m_audioRunning)
                {
                    std::thread thread(std::bind(&SpeakerResource::playAudioThread, this));
                    thread.detach();
                }
                m_audioRunning = true;
                break;
            }
            case ALARM_STOP:
                // Kill thread
                m_audioRunning = false;

                int count = 0;
                while(!m_audioRunningMutex.try_lock() && count < 20)
                {
                    count++;
                   // std::cerr << "Failed to lock mutex" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if(count >= 10)
                        break;
                }

                // Play stop sound
                std::cout << "Play stop sound" << std::endl;
#ifdef __linux__
#ifdef ARM
                    system("aplay /home/rpi/alarm_stop.wav");
#endif
#else
                    std::cerr << "Unsupported Operating System" << std::endl;
#endif
                break;
            }

            break;
        }
        case AttributeType::VOLUME:
        {
            // Get the volume and range it between 0 - 100
            int volume = (static_cast<double>(attr.value().get<int>()) / 255) * 100;
            std::cout << "Volume is: " << volume << std::endl;
#ifdef __linux__
/*#ifdef ARM
            std::stringstream ss;
            ss << "amixer cset numid=1 -- ";
            ss << static_cast<int>(volume);
            ss << "%";
            system(ss.str().c_str());
#else*/
            std::stringstream ss;
            ss << "amixer -D pulse sset Master ";
            ss << static_cast<int>(volume);
            ss << "%";
            system(ss.str().c_str());
//#endif
#else
            std::cerr << "Unsupported Operating System" << std::endl;
#endif

            // DEBUG
            std::cout << ss.str().c_str() << std::endl;
            break;
        }
        case AttributeType::UNKNOWN:
            std::cerr << "Unknown value!" << std::endl;
            break;
        default:
            break;
        }

        // Overwrite existing value
        std::string key = attr.key();
        m_resource->setAttribute(key, attr.value());
    }
    std::cout << "============================================ \n";
}


/**
 * @brief setAttributes
 */
void SpeakerResource::setAttributes()
{
    RCSResourceAttributes attrs;
    attrs["state"] = false;
    attrs["volume"] = (int) 0;
    attrs["mute"] = (bool) false;
    attrs["sound"] = (int) 0;

    m_resource->setAttributes(attrs);
}


/**
 * @brief readInputThread
 */
void SpeakerResource::playAudioThread()
{
    std::lock_guard<std::mutex> lock(m_audioRunningMutex);
    while(m_audioRunning)
    {
        // Play audio
        #ifdef __linux__
            system("aplay /home/rpi/alarm_sound.wav");
        #else
            std::cerr << "Unsupported Operating system" << std::endl;
        #endif

        if(!m_audioRunning)
            break;

        // Sleep thread for 6 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(AUDIO_THREAD_DELAY_MS));
    }
    std::cout << "Stopping playAudioThread" << std::endl;
}

/**
 * @brief getAttributeType
 * @param type
 * @return
 */
AttributeType SpeakerResource::getAttributeType(const std::string &type)
{
    if(type.compare("state") == 0)
        return AttributeType::STATE;
    else if(type.compare("volume") == 0)
        return AttributeType::VOLUME;
    else if(type.compare("mute") == 0)
        return AttributeType::MUTE;
    else if(type.compare("sound") == 0)
        return AttributeType::SOUND;
    else
        return AttributeType::UNKNOWN;
}
