#include "vulkanBase.hpp"

#include <iostream>
#include <cstring>
#include <set>
#include <stdexcept>

VulkanBase::VulkanBase(Window* pWindow, bool enableValidationLayers) : pWindow(pWindow), enableValidationLayers(enableValidationLayers) {};

void VulkanBase::createInstance() {

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = nullptr;
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = 0;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    std::vector<VkLayerProperties> availableLayers = getAvailableLayers(false);
    requiredLayers = getRequiredLayers(false);

    for(const char* rLayer : requiredLayers) {
        bool foundLayer = false;
        for(VkLayerProperties aLayer : availableLayers) {
            if(strcmp(rLayer, aLayer.layerName) == 0) {
                std::cout << "Found required layer: " << rLayer << '\n';
            }
            foundLayer = true;
            break;
        }
        if (!foundLayer) {
        throw std::runtime_error("Could not find required layer.");
        }
    }

    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensions(false);
    requiredExtensions = getRequiredExtensions(false);

    for(const char* rExtension : requiredExtensions) {
        bool foundExtension = false;
        for(VkExtensionProperties aExtension : availableExtensions) {
            if(strcmp(rExtension, aExtension.extensionName) == 0) {
                std::cout << "Found required extension: " << rExtension << '\n';
        foundExtension = true;
                break;
            }
        }
        if (!foundExtension) {
        throw std::runtime_error("Could not find required extension.");
        }
    }

    VkInstanceCreateInfo instanceInfo = {};    
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    instanceInfo.ppEnabledLayerNames = requiredLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if(vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Could not create instance.");
    }
}

std::vector<VkLayerProperties> VulkanBase::getAvailableLayers(bool print) {

    unsigned int layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

    if(print) {
        std::cout << "available layers:\n";
        for(VkLayerProperties layer : layerProperties) {
            std::cout << '\t' << layer.layerName << '\n';
        }
    }
    return layerProperties;
}

std::vector<const char*> VulkanBase::getRequiredLayers(bool print) {

    std::vector<const char*> requiredLayers;
    if(enableValidationLayers) {
        requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    if(print) {
        std::cout << "required layers:\n";
        for(const char* layer : requiredLayers) {
            std::cout << '\t' << layer << '\n';
        }
    }
    return requiredLayers;
}

std::vector<const char*> VulkanBase::getRequiredExtensions(bool print) {

    return pWindow->getRequiredVulkanExtensions(print);
}

std::vector<VkExtensionProperties> VulkanBase::getAvailableExtensions(bool print) {

    unsigned int extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

    if(print) {
        std::cout << "available extensions:\n";
        for(VkExtensionProperties extension : extensionProperties) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }
    return extensionProperties;
}


void VulkanBase::createLogicalDevice() {

    findSuitablePhysicalDevice(true);

    std::set<uint32_t> uniqueQueues = {graphicsQueueIndex, presentQueueIndex};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.f;
    for(uint32_t queue : uniqueQueues) {
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.queueFamilyIndex = queue;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledLayerCount = requiredLayers.size();
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.pEnabledFeatures = nullptr;


    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
       throw std::runtime_error("failed to create logical device.");
    } 
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
}

void VulkanBase::findSuitablePhysicalDevice(bool print) {

    std::vector<const char*> requiredQueues;
    requiredQueues.push_back("VK_QUEUE_GRAPHICS_BIT");

    unsigned int deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties(deviceCount);

    for(size_t i = 0; i < deviceCount; i++) {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties[i]);
    }

    bool foundGraphicsQueue = false;
    bool foundPresentQueue = false;
    unsigned int physicalDeviceIdx;
    for(size_t i = 0; i < deviceCount; i++) {
        unsigned int queueCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueCount, queueProperties.data());
        for(size_t k = 0; k < queueCount; k++) {
            if(queueProperties[k].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                physicalDevice = physicalDevices[i];
                physicalDeviceIdx = i;
                graphicsQueueIndex = static_cast<uint32_t>(k);
                foundGraphicsQueue = true;
            } 
            VkBool32 presentSupport;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], static_cast<uint32_t>(k), surface, &presentSupport);
            if(presentSupport) {
                presentQueueIndex = static_cast<uint32_t>(k);
                foundPresentQueue = true;
            }
        }
    }
    if (!foundGraphicsQueue && !foundPresentQueue) {
    throw std::runtime_error("Could not find suitable physical device.");
    }
    if(print) {
        std::cout << "Found suitable device: " << physicalDeviceProperties[physicalDeviceIdx].deviceName << '\n';
    }
}

void VulkanBase::createSurface() {

    if(SDL_Vulkan_CreateSurface(pWindow->getWindow(), instance, &surface) != SDL_TRUE) {
        throw std::runtime_error("Could not create surface.");
    }
}

void VulkanBase::cleanUp() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

}








