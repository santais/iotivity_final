#ifndef ButtonResource_H_
#define ButtonResource_H_

#include "RPIRCSResourceObject.h"
#include <thread>
#ifdef ARM
    #include "wiringPi.h"
#endif

#include <chrono>

class ButtonResource
{
public:
    ButtonResource();

    ButtonResource(int portPin, const std::string &uri);

    ~ButtonResource();

    ButtonResource(const ButtonResource&);
    ButtonResource(ButtonResource &&);
    ButtonResource& operator=(const ButtonResource&);
    ButtonResource& operator=(ButtonResource&&);

    /**
     * @brief setInputPortPin
     *
     * @param portPin
     */
    void setInputPortPin(int portPin);

    /**
     * @brief getInputPortPin
     *
     * @return
     */
    int getInputPortPin();

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

    void setUri(std::string& uri);

    std::string getUri();

private:
    /**
     * Resource object
     */
    RPIRCSResourceObject::Ptr m_resource;

    /**
     * Input port for the LED
     */
    int m_inputPortPin;

    /**
     * @brief m_uri
     */
    std::string m_uri;

    /**
     * @brief m_buttonInputThread
     */
    //std::thread m_buttonInputThread;

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
    void readInputThread();
};


#endif /* ButtonResource_H_ */
