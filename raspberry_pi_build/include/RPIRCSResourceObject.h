#ifndef _RPIRCSRESOURCEOBJECT_H_
#define _RPIRCSRESOURCEOBJECT_H_

#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"
#include "ConfigurationResource.h"

#include <vector>
#include <iostream>
#include <functional>

#define UNUSED __attribute__((__unused__))

using namespace OIC;
using namespace OIC::Service;

typedef std::function<void(const RCSRequest&, RCSResourceAttributes&)> setRPIRequestHandlerCallback;

class RPIRCSResourceObject
{
public:
    typedef std::shared_ptr<RPIRCSResourceObject> Ptr;
    typedef std::shared_ptr<const RPIRCSResourceObject> ConstPtr;

public:

    // Use default c++ libraries for copy constructor
    RPIRCSResourceObject(const RPIRCSResourceObject&)            = default;
    RPIRCSResourceObject(RPIRCSResourceObject&&)                 = default;

    // Do not allow overload opeartions
    RPIRCSResourceObject& operator=(const RPIRCSResourceObject&) = delete;
    RPIRCSResourceObject& operator=(RPIRCSResourceObject&&)      = delete;

    /**
     * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
     * @param resourceTypes         The types associated with the resource
     * @param resourceInterfaces    The interfaces associated with the resource
     */
    RPIRCSResourceObject(const std::string& uri, const std::string& resourceType, const std::string& resourceInterface);

    /**
     * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
     * @param resourceTypes         The types associated with the resource
     * @param resourceInterfaces    The interfaces associated with the resource
     */
    RPIRCSResourceObject(const std::string& uri, std::vector<std::string>&& resourceTypes, std::vector<std::string>&& resourceInterfaces);

    /**
     * @brief RPIRCSResourceObject Set the internal parameters for this specific resource
     * @param resourceTypes         The types associated with the resource
     * @param resourceInterfaces    The interfaces associated with the resource
     */
    RPIRCSResourceObject(const std::string &uri, const std::vector<std::string>& resourceTypes, const std::vector<std::string>& resourceInterfaces);

    /**
     * @brief ~RPIRCSResourceObject Clear all resources bindings.
     */
    ~RPIRCSResourceObject();

    /**
     * @brief createResource Creates the resource
     */
    void createResource(bool discovery, bool observable, bool secured);

    /**
     * @brief addType
     * @param type
     * @return
     */
    std::vector<std::string> getTypes();

    /**
     * @brief addInterface
     * @param interface
     * @return
     */
    std::vector<std::string> getInterfaces();

    /**
     * @brief addAttributes
     * @param name
     */
    void addAttribute(const std::string& name, RCSResourceAttributes::Value& value);

    /**
     * @brief getAttributes
     * @return
     */
    RCSResourceAttributes getAttributes();

    /**
     * @brief setAttribute
     *
     * @param name
     * @param value
     */
    void setAttribute(const std::string& name, RCSResourceAttributes::Value value);

    /**
     * @brief setAttributes
     *
     * @param attributes
     */
    void setAttributes(const RCSResourceAttributes &attributes);
    void setAttributes(RCSResourceAttributes &&attributes);

    /**
     * @brief getResourceObject
     * @return
     */
    RCSResourceObject::Ptr getResourceObject();

    /**
     * @brief setReqHandler
     * @param handler
     */
    void setReqHandler(setRPIRequestHandlerCallback handler);

private:

    RPIRCSResourceObject() {}


    RCSSetResponse setResponse(const RCSRequest& request, RCSResourceAttributes& attributes);

private:
    /**
     * @brief m_resource
     * Iotivity resource object container
     */
    RCSResourceObject::Ptr m_resource;

    /**
     * @brief m_resourceTypes Resource types of the resource
     */
    std::vector<std::string> m_resourceTypes;

    /**
     * @brief m_resourceInterfaces Resource interfaces of the resource
     */
    std::vector<std::string> m_resourceInterfaces;

    /**
     * @brief m_uri Uri of the registered resource
     */
    std::string m_uri;

    /**
     * @brief m_callback
     */
    setRPIRequestHandlerCallback m_applicationCallback;


protected:

};

#endif /* _RPIRCSRESOURCEOBJECT_H_ */
