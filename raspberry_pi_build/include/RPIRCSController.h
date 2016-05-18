#ifndef RPIRCSCONTROLLER_H
#define RPIRCSCONTROLLER_H

#include "RPIRCSResourceObject.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSException.h"
#include "RCSRequest.h"

#include <vector>
#include <iostream>
#include <functional>

class RPIRCSController
{
public:
    typedef std::unique_ptr<RPIRCSController> Ptr;

    typedef std::map<std::string, std::string> QueryParamsMap;
    typedef const std::map<std::string, std::string> ConstQueryParamsMap;

    typedef std::map<std::string, std::string>::iterator QueryParamsMapIterator;

    typedef std::unordered_map<std::string, RPIRCSResourceObject::Ptr> ControllerMap;

public:
    /**
     * @brief getInstance Singleton instance of the class
     * @return
     */
    static RPIRCSController* getInstance();

    ~RPIRCSController();

    ControllerMap getResourceList();

private:

    /**
     * @brief m_controller Object resource
     */
    RCSResourceObject::Ptr m_controller;

    /**
     * @brief m_resourceList Contains all registered resources
     */
    ControllerMap m_resourceList;

    /**
     * @brief m_uri
     */
    std::string m_uri;

    /**
     * @brief m_resourceType
     */
    std::string m_resourceType;

    /**
     * @brief m_resourceInterface
     */
    std::string m_resourceInterface;

private:

    RPIRCSController();

    /**
     * @brief setResponse response called whenever a call to the rpi controller is applied.
     * @param request
     * @param attributes
     * @return
     */
    RCSSetResponse setResponse(const RCSRequest& request, RCSResourceAttributes& attributes);

public:
    RPIRCSController(RPIRCSController const&)   = delete;   // No overloading allowed
    void operator=(RPIRCSController const&)     = delete;   // No assigning allowed


};

#endif // RPIRCSCONTROLLER_H
