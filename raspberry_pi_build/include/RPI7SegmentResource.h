#ifndef _RPI7SEGMENTRESOURCE_H_
#define _RPI7SEGMENTRESOURCE_H_

#include "RPIRCSResourceObject.h"
#ifdef ARM
    #include "wiringPi.h"
#endif

#include "sn74hc165.h"
#include "sn74hc595.h"

namespace
{
    static const std::string ATTRIBUTE_NAME = "score";
}

class RPI7SegmentResource
{
public:
        typedef std::shared_ptr<RPI7SegmentResource> Ptr;
        typedef std::shared_ptr<const RPI7SegmentResource> ConstPtr;

public:
    RPI7SegmentResource();

    RPI7SegmentResource(const std::string &uri, const int &id);

    ~RPI7SegmentResource();

    RPI7SegmentResource(const RPI7SegmentResource&);
    RPI7SegmentResource(RPI7SegmentResource &&);
    RPI7SegmentResource& operator=(const RPI7SegmentResource&);
    RPI7SegmentResource& operator=(RPI7SegmentResource&&);

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
     * @brief Set the segment value
     *
     * @param value     New value to overwrite
     */
    void setSegmentValue(int value);

    void setSegmentID(int id);

    int getSegmentID();

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
     * @brief 7segment driver id
     */
    int m_id;

private:
    /**
     * @brief setAttributes
     */
    void setAttributes();

};


#endif /* _RPI7SEGMENTRESOURCE_H_ */
