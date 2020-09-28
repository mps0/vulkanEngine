#include "vulkanBase.hpp"

#include <iostream>
#include <cstring>

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
    std::vector<const char*> requiredLayers = getRequiredLayers(false);

    for(const char* rLayer : requiredLayers) {
        for(VkLayerProperties aLayer : availableLayers) {
            if(strcmp(rLayer, aLayer.layerName) == 0) {
                std::cout << "Found required layer: " << rLayer << '\n';
                break;
            }
        }
        std::runtime_error("Could not find required layer.");
    }

    std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensions(false);
    std::vector<const char*> requiredExtensions = getRequiredExtensions(false);

    for(const char* rExtension : requiredExtensions) {
        for(VkExtensionProperties aExtension : availableExtensions) {
            if(strcmp(rExtension, aExtension.extensionName) == 0) {
                std::cout << "Found required extension: " << rExtension << '\n';
                break;
            }
        }
        std::runtime_error("Could not find required extension.");
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
        std::runtime_error("Could not create instance.");
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

std::vector<const char*> VulkanBase::getRequiredExtensions(bool print) {

    return pWindow->getRequiredVulkanExtensions(print);
}

void VulkanBase::cleanUp() {
    vkDestroyInstance(instance, nullptr);

}








