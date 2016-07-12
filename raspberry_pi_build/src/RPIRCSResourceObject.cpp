#include "RPIRCSResourceObject.h"

/**
 * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
 * @param resourceTypes         The types associated with the resource
 * @param resourceInterfaces    The interfaces associated with the resource
 */
RPIRCSResourceObject::RPIRCSResourceObject(const std::string& uri, const std::string& resourceType, const std::string& resourceInterface)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
   // std::cout << "Called rvalue constructor" << std::endl;
    m_uri = uri;
    m_resourceTypes.push_back(resourceType);
    m_resourceInterfaces.push_back(resourceInterface);
}

/**
 * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
 * @param resourceTypes         The types associated with the resource
 * @param resourceInterfaces    The interfaces associated with the resource
 */
RPIRCSResourceObject::RPIRCSResourceObject(const std::string &uri, std::vector<std::string>&& resourceTypes, std::vector<std::string>&& resourceInterfaces)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
   // std::cout << "Called rvalue constructor" << std::endl;
    m_uri = uri;
    m_resourceTypes = std::move(resourceTypes);
    m_resourceInterfaces = std::move(resourceInterfaces);
}


/**
 * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
 * @param resourceTypes         The types associated with the resource
 * @param resourceInterfaces    The interfaces associated with the resource
 */
RPIRCSResourceObject::RPIRCSResourceObject(const std::string& uri, const std::vector<std::string>& resourceTypes, const std::vector<std::string>& resourceInterfaces)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
   // std::cout << "Called lvalue constructor" << std::endl;
    m_uri = uri;
    m_resourceTypes = resourceTypes;
    m_resourceInterfaces = resourceInterfaces;
}
/**
 * @brief ~RPIRCSResourceObject Clear all resources bindings.
 */
RPIRCSResourceObject::~RPIRCSResourceObject()
{
    // TODO: Destroy smart pointer object
}

/**
 * @brief createResource Creates the resource
 */
void RPIRCSResourceObject::createResource(bool discoverable, bool observable, bool secured)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
    try {
        std::cout << "uri: " << m_uri << std::endl;
        RCSResourceObject::Builder builder = RCSResourceObject::Builder(m_uri, m_resourceTypes[0], m_resourceInterfaces[0])
                .setDiscoverable(discoverable)
                .setObservable(observable)
                .setSecureFlag(secured);

        std::for_each(m_resourceTypes.begin() + 1, m_resourceTypes.end(),
                      [&builder](const std::string& typeName) {
            builder = builder.addType(typeName);
            //std::cout << "Adding type: " << typeName << std::endl;
        });

        std::for_each(m_resourceInterfaces.begin() + 1, m_resourceInterfaces.end(),
                      [&builder](const std::string& interfaceName) {
            builder = builder.addInterface(interfaceName);
            //std::cout << "Adding interface: " << interfaceName << std::endl;
        });

        m_resource = builder.build();

        m_resource->setSetRequestHandler(std::bind(&RPIRCSResourceObject::setResponse, this, std::placeholders::_1, std::placeholders::_2));
        m_resource->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    }
    catch (RCSPlatformException e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (RCSException e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (RCSInvalidParameterException e)
    {
        std::cout << "Invalid param: " << e.what() << std::endl;
    }
}

/**
 * @brief addType
 * @param type
 * @return
 */
std::vector<std::string> RPIRCSResourceObject::getTypes()
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
    try
    {
        return m_resource->getTypes();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << __func__ << std::endl;
    }
}

/**
 * @brief addInterface
 * @param interface
 * @return
 */
std::vector<std::string> RPIRCSResourceObject::getInterfaces()
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
    try
    {
        return m_resource->getInterfaces();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << __func__ << std::endl;
    }
}

/**
 * @brief addAttributes
 * @param name
 */
void RPIRCSResourceObject::addAttribute(const std::string& name, RCSResourceAttributes::Value& value)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
    try
    {
        m_resource->setAttribute(name, value);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << __func__ << std::endl;
    }
}

/**
 * @brief getAttributes
 * @return
 */
RCSResourceAttributes RPIRCSResourceObject::getAttributes()
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif
    try
    {
        return m_resource->getAttributes();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << __func__ << std::endl;
    }
}

/**
 * @brief setAttribute
 *
 * @param name
 * @param value
 */
void RPIRCSResourceObject::setAttribute(std::string& name, RCSResourceAttributes::Value value)
{
    m_resource->setAttribute(name, value);
}

/**
 * @brief setAttributes
 *
 * @param attributes
 */
void RPIRCSResourceObject::setAttributes(const RCSResourceAttributes &attributes)
{
    for(const auto &attr : attributes) {
        m_resource->setAttribute(attr.key(), attr.value());
    }
}

void RPIRCSResourceObject::setAttributes(RCSResourceAttributes &&attributes)
{
    for(const auto &attr : std::move(attributes)) {
        m_resource->setAttribute(attr.key(), attr.value());
    }
}


/**
 * @brief getResourceObject
 * @return
 */
RCSResourceObject::Ptr RPIRCSResourceObject::getResourceObject()
{
    return m_resource;
}

/**
 * @brief setReqHandler
 * @param handler
 */
void RPIRCSResourceObject::setReqHandler(setRPIRequestHandlerCallback handler)
{
#ifdef DEBUG
    std::cout << __func__ << std::endl;
#endif

    std::cout << "Setting the application callback" << std::endl;
    m_applicationCallback = handler;

}

RCSSetResponse RPIRCSResourceObject::setResponse(const RCSRequest& request, RCSResourceAttributes& attributes)
{
    std::cout << __func__ << " in RPIRCSResourceObject" << std::endl;

    try
    {
        m_applicationCallback(request, attributes);
    }
    catch (const std::bad_function_call& e)
    {
        std::cout << e.what() << "in setResponse" << std::endl;
    }
    catch (const std::bad_alloc& e)
    {
        std::cout << e.what() << "in setResponse" << std::endl;
    }

    return RCSSetResponse::defaultAction();
}
