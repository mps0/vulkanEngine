#include "vulkanBase.hpp"

#include <iostream>
#include <cstring>
#include <set>
#include <stdexcept>
#include <algorithm>

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

    std::vector<VkExtensionProperties> availableInstanceExtensions = getAvailableInstanceExtensions(false);
    requiredInstanceExtensions = getRequiredInstanceExtensions(false);

    for(const char* rExtension : requiredInstanceExtensions) {
        bool foundExtension = false;
        for(VkExtensionProperties availableInstanceExtension : availableInstanceExtensions) {
            if(strcmp(rExtension, availableInstanceExtension.extensionName) == 0) {
                std::cout << "Found required instance extension: " << rExtension << '\n';
                foundExtension = true;
                break;
            }
        }
        if (!foundExtension) {
            throw std::runtime_error("Could not find required instance extension.");
        }
    }

    VkInstanceCreateInfo instanceInfo = {};    
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    instanceInfo.ppEnabledLayerNames = requiredLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

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

std::vector<const char*> VulkanBase::getRequiredInstanceExtensions(bool print) {

    return pWindow->getRequiredVulkanExtensions(print);
}

std::vector<VkExtensionProperties> VulkanBase::getAvailableInstanceExtensions(bool print) {

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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
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
    if (!foundGraphicsQueue || !foundPresentQueue || !checkPhysicalDeviceExtensions(true)) {
        throw std::runtime_error("Could not find suitable physical device.");
    }
    if(print) {
        std::cout << "Found suitable device: " << physicalDeviceProperties[physicalDeviceIdx].deviceName << '\n';
    }
}

bool VulkanBase::checkPhysicalDeviceExtensions(bool print) {

    uint32_t propertyCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> extensionproperties(propertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extensionproperties.data());

    for(const char* requiredDeviceExtension : requiredDeviceExtensions) {
        bool foundExtension = false;
        for(VkExtensionProperties properties : extensionproperties) {
            if(strcmp(properties.extensionName, requiredDeviceExtension) == 0) {
                if(print){
                    std::cout << "Found required device extension: " << properties.extensionName << std::endl;
                }
                foundExtension = true;
                break;
            }
        }
        if(!foundExtension) {
            return false;
        }
    }
    return true;
}

void VulkanBase::createSurface() {

    if(SDL_Vulkan_CreateSurface(pWindow->getWindow(), instance, &surface) != SDL_TRUE) {
        throw std::runtime_error("Could not create surface.");
    }
}

void VulkanBase::createSwapchain() {
    
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    surfaceFormat = getSurfaceFormat();
    uint32_t minImageCount = getMinImageCount(surfaceCapabilities);
    VkExtent2D swapExtent = getSwapExtent(surfaceCapabilities);
    VkPresentModeKHR presentMode = getPresentMode();

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = minImageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = swapExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(graphicsQueue != presentQueue) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[2] = {graphicsQueueIndex, presentQueueIndex};
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Could not create swapchain.");
     }

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &swapChainImageCount, nullptr);
    swapchainImages.resize(swapChainImageCount);    
    vkGetSwapchainImagesKHR(device, swapchain, &swapChainImageCount, swapchainImages.data());
}

VkSurfaceFormatKHR VulkanBase::getSurfaceFormat() {

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

    for(VkSurfaceFormatKHR surfaceFormat : surfaceFormats) {
        if(surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surfaceFormat;
        }
    }
    return surfaceFormats[0];
}

uint32_t VulkanBase::getMinImageCount(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

    if (imageCount > surfaceCapabilities.maxImageCount && 0 > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    return imageCount;
}

VkExtent2D VulkanBase::getSwapExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) {

    VkExtent2D minExtent = surfaceCapabilities.minImageExtent;
    VkExtent2D maxExtent = surfaceCapabilities.maxImageExtent;

    VkExtent2D extent = {SCREEN_WIDTH, SCREEN_HEIGHT};
    extent.width = std::max(minExtent.width, std::min(maxExtent.width, extent.width));
    extent.height = std::max(minExtent.height, std::min(maxExtent.height, extent.height));

    return extent;
}

VkPresentModeKHR VulkanBase::getPresentMode() {

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
    for(VkPresentModeKHR presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
     }
    return VK_PRESENT_MODE_FIFO_KHR;
}
        

void VulkanBase::createImageViews() {

    swapchainImageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {     
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = surfaceFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Could not create image view.");
        }
    }
}


void VulkanBase::cleanUp() {
    for (VkImageView imageView : swapchainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

}








