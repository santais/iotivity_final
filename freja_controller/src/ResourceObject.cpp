#include "ResourceObject.hpp"

#include <iostream>
//#include "Controller.h"
#include "BuildingController.h"

using namespace OIC;
using namespace OC;
using namespace OIC::Service;

ResourceObject::ResourceObject(RCSRemoteResourceObject::Ptr remoteResource)
{
    if(remoteResource)
    {
        this->m_resourceObject = remoteResource;


        if(remoteResource->getUri().compare("/arduino/temperatureSensor") == 0)
        {
            std::cout << "Android temperature sensor. No caching activated" << std::endl;
        }
        else
        {
            try
            {
                // Set the callbacks
                m_resourceObject->startCaching(std::bind(&ResourceObject::cacheUpdateCallback, this, std::placeholders::_1));
                //m_resourceObject->startMonitoring(std::bind(&ResourceObject::stateChangedCallback, this, std::placeholders::_1));

            }
            catch (RCSBadRequestException e)
            {
                std::cout << e.what() << std::endl;
            }
        }

        // Find device type
        setResourceDeviceType(std::move(remoteResource->getTypes()), m_resourceDeviceType);

        m_resourceObjectCacheCallback = BuildingController::getInstance()->getControllerResourceCacheObjCallback();
        m_resourceObjectStateCallback = BuildingController::getInstance()->getControllerResourceStateObjCallback();
    }
}

/**
 * @brief ~ResourceObject     Destructor. Currently not used.
 */
ResourceObject::~ResourceObject()
{
    std::cout << __func__ << "Destructor called" << std::endl;
    //m_resourceObject.reset();
    m_resourceObjectCacheCallback = nullptr;
    m_resourceObjectStateCallback = nullptr;
}

/**
 * @brief getAttributes
 * @return The latest cached resources.
 */
RCSResourceAttributes ResourceObject::getAttributes()
{
    std::lock_guard<mutex> lock(mutex);
    if(!m_attrs.empty())
    {
        return m_attrs;
    }
}


/**
 * @brief getResourceDeviceType
 * @return The Resource Device Type
 */
ResourceDeviceType ResourceObject::getResourceDeviceType()
{
    return m_resourceDeviceType;
}

/**
 * @brief convertResourceDeviceTypeToString
 * @return a string representing the resource device type
 */
std::string ResourceObject::convertResourceDeviceTypeToString(const ResourceDeviceType &deviceType)
{
    switch(deviceType)
    {
    case ResourceDeviceType::OIC_BUTTON:
        return std::string("OIC_BUTTON");

    case ResourceDeviceType::OIC_FAN:
        return std::string("OIC_FAN");

    case ResourceDeviceType::OIC_LIGHT:
        return std::string("OIC_LIGHT");

    case ResourceDeviceType::OIC_SENSOR:
        return std::string("OIC_SENSOR");

    case ResourceDeviceType::OIC_SPEAKER:
        return std::string("OIC_SPEAKER");

    case ResourceDeviceType::OIC_TV:
        return std::string("OIC_TV");

    case ResourceDeviceType::UNKNOWN_DEVICE:
        return std::string("UNKNOWN DEVICE");
    }
}

/**
 * @brief startCaching Starts caching the device
 */
void ResourceObject::startCaching()
{
    /*if(!m_resourceObject->isCaching() && m_resourceObject->isObservable())
    {*/
    if(m_resourceObject)
    {
        try
        {
            m_resourceObject->startCaching(std::bind(&ResourceObject::cacheUpdateCallback, this, std::placeholders::_1));
        }
        catch (RCSBadRequestException e)
        {
            std::cout << e.what() << std::endl;
        }
    }

}

/**
 * @brief stopCaching Stop caching the deivce
 */
void ResourceObject::stopCaching()
{
    /*if(m_resourceObject->isCaching())
    {*/
        m_resourceObject->stopCaching();
    /*}
    else
    {
        std::cerr << "Resource was not caching!" << std::endl;
    }*/
}


/**
 * @brief setAttributes Set the resource attributes.
 * @param attrs
 */
void ResourceObject::setAttributes(RCSResourceAttributes attrs)
{
    if(m_resourceObject)
    {
        try
        {
            m_resourceObject->setRemoteAttributes(attrs, std::bind(&ResourceObject::remoteAttributesSetCallback, this, std::placeholders::_1));
        }
        catch(RCSInvalidParameterException e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

/**
 * @brief getRemoteResourceObject
 * @return The remote resource
 */
RCSRemoteResourceObject::Ptr ResourceObject::getRemoteResourceObject()
{
    return m_resourceObject;
}



/**
 * @brief cacheUpdateCallback Cache callback called when the remote attributes
 *                            have changed.
 *
 * @param attrs               The new set of attributes
 */
void ResourceObject::cacheUpdateCallback(const RCSResourceAttributes attrs)
{
    std::lock_guard<mutex> lock(mutex);
    //std::cout << __func__ << std::endl;

    m_attrs = attrs;

    // Send the respond to the Controller
    if(m_resourceObjectCacheCallback)
    {
      /*  std::cout << "\n=================================" << std::endl;
        std::cout << "Cached changed for device: " << ResourceObject::convertResourceDeviceTypeToString(m_resourceDeviceType) << std::endl;*/
        //this->printAttributes(attrs);
        if(m_resourceObjectCacheCallback) {
            m_resourceObjectCacheCallback(std::move(attrs), ResourceObjectState::CACHE_CHANGED, m_resourceDeviceType);
        }
    }
    else
    {
        std::cerr << "ResourceObjectCallback has not been initiaized" << std::endl;
    }
}

/**
 * @brief stateChangedCallback Called when the state of the resource object changes
 *
 * @param state     New state of the resource
 */
void ResourceObject::stateChangedCallback(const ResourceState state)
{
    std::lock_guard<mutex> lock(mutex);
   // std::cout << __func__ << std::endl;

    // Send the response to the Controller
    if(m_resourceObjectStateCallback)
    {
        //std::cout << "\n=================================" << std::endl;
        //std::cout << "State changed for device: " << ResourceObject::convertResourceDeviceTypeToString(m_resourceDeviceType) << std::endl;
        //this->printResourceState(state);
        if(m_resourceObject) {
            std::cout << "Before resourceobject call" << std::endl;
            m_resourceObjectStateCallback(state, m_resourceObject->getUri(), m_resourceObject->getAddress());
        }
        else
        {
            std::cerr << "m_resorceObject is null!" << std::endl;
        }
    }
    else
    {
    	std::cerr << "ResourceObjectCallback has not been initialized" << std::endl;
    }
}

/**
 * @brief remoteAttributesGetcallback Attributes of the called resource
 *
 * @param attrs               The resource current attributes state
 */
void ResourceObject::remoteAttributesGetCallback(const RCSResourceAttributes attrs)
{
    std::lock_guard<mutex> lock(mutex);
    std::cout << __func__ << std::endl;

    m_attrs = attrs;
}

/**
 * @brief remoteAttributesSetCallback Set the attributes of the resource
 *
 * @param attrs               The attributes to set at the endpoint device.
 */
void ResourceObject::remoteAttributesSetCallback(const RCSResourceAttributes attrs)
{
    std::cout << __func__ << std::endl;
    printAttributes(attrs);
}



/**
 * @brief printAttributes     Prints the attributes.
 */
void ResourceObject::printAttributes(RCSResourceAttributes attrs)
{
    std::cout << "=================================" << std::endl;
    if(attrs.empty())
    {
        std::cout << "\t No attributes present" << std::endl;
    }
    else
    {
        std::cout << "\t Attributes: " << std::endl;

        for (const auto& attribute : attrs)
        {
            std::cout << "\t\t Key: " << attribute.key() << std::endl;
            std::cout << "\t\t Value: " << attribute.value().toString() << std::endl;
        }
    }
     std::cout << "=================================\n" << std::endl;
}

/**
 * @biref Prints the current resource state
 *
 * @param The new resource state
 */
void ResourceObject::printResourceState(ResourceState state)
{	
	std::cout << "\t New state is: ";
	switch(state)
	{
		case ResourceState::NONE:
			std::cout << "NONE" << std::endl;
		break;
		case ResourceState::REQUESTED:
			std::cout << "REQUESTED" << std::endl;
		break;
		case ResourceState::ALIVE: 
			std::cout << "ALIVE" << std::endl;
		break;
		case ResourceState::LOST_SIGNAL:
			std::cout << "LOST_SIGNAL" << std::endl;
		break;
		case ResourceState::DESTROYED:
			std::cout << "DESTROYED" << std::endl;
		break;	
		default:
			std::cout << "UNKNOWN TYPE" << std::endl;
		break;
	}
	std::cout << "=================================\n" << std::endl;
}

/**
 * @brief setResourceDeviceType Find the resource device type and sets it.
 * @param types
 */
void ResourceObject::setResourceDeviceType(const std::vector<std::string> &types, ResourceDeviceType &deviceType)
{
    // Search for the device type
    for(const std::string &type : types)
    {
        if(type.compare(OIC_DEVICE_BUTTON) == 0) { deviceType = ResourceDeviceType::OIC_BUTTON; break; }
        else if(type.compare(OIC_DEVICE_FAN) == 0) { deviceType = ResourceDeviceType::OIC_FAN; break; }
        else if(type.compare(OIC_DEVICE_LIGHT) == 0) { deviceType = ResourceDeviceType::OIC_LIGHT; break; }
        else if(type.compare(OIC_DEVICE_SENSOR) == 0) { deviceType = ResourceDeviceType::OIC_SENSOR; break; }
        else if(type.compare(OIC_DEVICE_TV) == 0) { deviceType = ResourceDeviceType::OIC_TV; break; }
        else { deviceType = ResourceDeviceType::UNKNOWN_DEVICE; break; }
    }
}

