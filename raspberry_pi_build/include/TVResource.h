#ifndef TVResource_H_
#define TVResource_H_

#include "RPIRCSResourceObject.h"
//#include "wiringPi.h"

namespace
{
    const static int TV_OFF = 0;
    const static int TV_ON  = 1;
}

class TVResource
{
public:
    TVResource();

    TVResource(const std::string &uri);

    ~TVResource();

    TVResource(const TVResource&);
    TVResource(TVResource &&);
    TVResource& operator=(const TVResource&);
    TVResource& operator=(TVResource&&);

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


#endif /* TVResource_H_ */
