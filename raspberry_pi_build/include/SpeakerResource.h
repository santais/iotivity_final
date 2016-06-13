#ifndef SPEAKERRESOURCE_H_
#define SPEAKERRESOURCE_H_

#include "RPIRCSResourceObject.h"
#include <mutex>
#include <thread>
#include <chrono>
#include <ostream>

namespace
{
    const int ALARM_ACTIVE = 1;
    const int ALARM_STOP = 2;

    const int AUDIO_THREAD_DELAY_MS = 6000;

    enum class AttributeType
    {
        STATE,
        VOLUME,
        MUTE,
        SOUND,
        UNKNOWN
    };
}

class SpeakerResource
{
public:
    SpeakerResource();

    SpeakerResource(const std::string &uri);

    ~SpeakerResource();

    SpeakerResource(const SpeakerResource&);
    SpeakerResource(SpeakerResource &&);
    SpeakerResource& operator=(const SpeakerResource&);
    SpeakerResource& operator=(SpeakerResource&&);

    /**
     * @brief getResourceObject
     *
     * @return
     */
    RPIRCSResourceObject::Ptr getResourceObject();

    /**
     * @brief createResource
     */
    int createResource();

    /**
     * @brief setUri
     * @param uri
     */
    void setUri(std::string& uri);

    /**
     * @brief getUri
     * @return
     */
    std::string getUri();

private:
    /**
     * Resource object
     */
    RPIRCSResourceObject::Ptr m_resource;

    /**
     * @brief m_uri
     */
    std::string m_uri;

    /**
     * @brief Indiciator used to start audio alarm
     */
    bool m_audioRunning;

    /**
     * @brief m_audioRunningMutex
     */
    std::mutex m_audioRunningMutex;


private:
    /**
     * @brief setRequestHandler
     *
     * @param request
     * @param attr
     */
    void setRequestHandler(const RCSRequest &request, RCSResourceAttributes &attr);

    /**
     * @brief setAttributes
     */
    void setAttributes();

    /**
     * @brief readInputThread
     */
    void playAudioThread();

    /**
     * @brief getAttributeType
     * @param type
     * @return
     */
    AttributeType getAttributeType(const std::string &type);
};


#endif /* SPEAKERRESOURCE_H_ */
