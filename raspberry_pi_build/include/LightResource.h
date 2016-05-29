#ifndef LIGHTRESOURCE_H_
#define LIGHTRESOURCE_H_

#include "RPIRCSResourceObject.h"
#ifdef ARM
    #include "wiringPi.h"
#endif

class LightResource
{
public:
    LightResource();

    LightResource(int portPin, const std::string &uri);

    ~LightResource();

    LightResource(const LightResource&);
    LightResource(LightResource &&);
    LightResource& operator=(const LightResource&);
    LightResource& operator=(LightResource&&);

    /**
     * @brief setOutputPortPin
     *
     * @param portPin
     */
    void setOutputPortPin(int portPin);

    /**
     * @brief getOutputPortPin
     *
     * @return
     */
    int getOutputPortPin();

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

    /**
     * @brief setHostingResource
     */
    void setHostingResource();

private:
    /**
     * Resource object
     */
    RPIRCSResourceObject::Ptr m_resource;

    /**
     * Output port for the LED
     */
    int m_outputPortPin;

    /**
     * @brief m_uri
     */
    std::string m_uri;

    /**
     * @brief m_hosting
     */
    bool m_hosting;

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
};


#endif /* LIGHTRESOURCE_H_ */
