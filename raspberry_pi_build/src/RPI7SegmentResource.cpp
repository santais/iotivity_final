#include "RPI7SegmentResource.h"
#include "resource_types.h"

RPI7SegmentResource::RPI7SegmentResource() :
    m_uri("/rpi7segment/0"),
    m_id(0)
{}

/**
 * @brief RPI7SegmentResource::RPI7SegmentResource
 *
 * @param portPin
 * @param uri       Uri of the new SegmentResource resource
 */
RPI7SegmentResource::RPI7SegmentResource(const std::string &uri, const int &id) :
    m_uri(uri),
    m_id(id)
{}

RPI7SegmentResource::~RPI7SegmentResource()
{
    m_resource.reset();
}

RPI7SegmentResource::RPI7SegmentResource(const RPI7SegmentResource& SegmentResource) :
    m_resource(SegmentResource.m_resource),
    m_uri(SegmentResource.m_uri),
    m_id(SegmentResource.m_id)
{}

RPI7SegmentResource::RPI7SegmentResource(RPI7SegmentResource&& SegmentResource) :
    m_resource(std::move(SegmentResource.m_resource)),
    m_uri(std::move(SegmentResource.m_uri)),
    m_id(std::move(SegmentResource.m_id))
{}

RPI7SegmentResource& RPI7SegmentResource::operator=(const RPI7SegmentResource& SegmentResource)
{
    m_resource = SegmentResource.m_resource;
    m_uri = SegmentResource.m_uri;
    m_id = SegmentResource.m_id;
}

RPI7SegmentResource& RPI7SegmentResource::operator=(RPI7SegmentResource&& SegmentResource)
{
    m_resource = std::move(SegmentResource.m_resource);
    m_uri = std::move(SegmentResource.m_uri);
    m_id = std::move(SegmentResource.m_id);
}


RPIRCSResourceObject::Ptr RPI7SegmentResource::getResourceObject()
{
    return m_resource;
}


/**
 * @brief createResource
 */
int RPI7SegmentResource::createResource()
{
    if(m_uri.empty())
    {
        std::cout << "Uri is empty!" << std::endl;
        return -1;
    }
    // Using default parameters
    std::vector<std::string> resourceTypes = {OIC_DEVICE_LIGHT};

    m_resource = std::make_shared<RPIRCSResourceObject>(RPIRCSResourceObject(m_uri,
                            std::move(resourceTypes), std::move(std::vector<std::string> {OC_RSRVD_INTERFACE_DEFAULT,
                                                                "oic.if.rw", "oic.if.a"})));

    m_resource->createResource(true, true, false);

    // Set the attributes
    this->setAttributes();

    return 1;
}

/**
 * @brief RPI7SegmentResource::setUri
 * @param uri
 */
void RPI7SegmentResource::setUri(std::string& uri)
{
    m_uri = uri;
}

/**
 * @brief RPI7SegmentResource::getUri
 * @return
 */
std::string RPI7SegmentResource::getUri()
{
    return m_uri;
}

/**
 * @brief Set the segment value
 *
 * @param value     New value to overwrite
 */
void RPI7SegmentResource::setSegmentValue(int value)
{
    RCSResourceAttributes::Value rcsValue((int) value);
    m_resource->setAttribute(ATTRIBUTE_NAME, rcsValue);
    m_resource->getResourceObject()->notify();
}

void RPI7SegmentResource::setSegmentID(int id)
{
    m_id = id;
}

int RPI7SegmentResource::getSegmentID()
{
    return m_id;
}



/**
 * @brief setAttributes
 */
void RPI7SegmentResource::setAttributes()
{
    RCSResourceAttributes::Value value((int) 0);
    m_resource->addAttribute(ATTRIBUTE_NAME, value);
}

