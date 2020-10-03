#include "vulkanBase.hpp"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include <cstring>
#include <vulkan/vulkan_core.h>

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

    uniqueQueues = {graphicsQueueIndex, presentQueueIndex};

    if(print) {
        std::cout << "Found suitable device: " << physicalDeviceProperties[physicalDeviceIdx].deviceName << std::endl;
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

void VulkanBase::createCommandBuffers() {

    commandPools.resize(uniqueQueues.size());
    commandBuffers.resize(uniqueQueues.size()); //note just have 1 command buffer per queue family!
    size_t i = 0;
    for(uint32_t queue : uniqueQueues) {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = queue;

        if(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPools[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool."); 
        }

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {}; 
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPools[i];
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        if(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Could not allocate command buffers.");
        } 
        i++;
    }
}

void VulkanBase::createDepthBuffer() {

    VkImageCreateInfo depthImageCreateInfo = {};
    depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageCreateInfo.format = VK_FORMAT_D16_UNORM;
    depthImageCreateInfo.extent.width = SCREEN_WIDTH;
    depthImageCreateInfo.extent.height = SCREEN_HEIGHT;
    depthImageCreateInfo.extent.depth = 1;
    depthImageCreateInfo.mipLevels = 1;
    depthImageCreateInfo.arrayLayers = 1;
    depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageCreateInfo.queueFamilyIndexCount = 0;
    depthImageCreateInfo.pQueueFamilyIndices = nullptr;
    depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateImage(device, &depthImageCreateInfo, nullptr, &depthImage) != VK_SUCCESS) {
        throw std::runtime_error("Could not create depth image.");
    }
    VkMemoryRequirements depthMemRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &depthMemRequirements);

    VkPhysicalDeviceMemoryProperties physicalDeviceMemProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemProps);

    uint32_t memoryTypeIndex;
    getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryTypeIndex);

    VkMemoryAllocateInfo depthMemAlloc = {};
    depthMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    depthMemAlloc.allocationSize = depthMemRequirements.size;
    depthMemAlloc.memoryTypeIndex = memoryTypeIndex;

    if(vkAllocateMemory(device, &depthMemAlloc, nullptr, &depthMemory) != VK_SUCCESS) {
        throw std::runtime_error("Could not allocate memory for depth image.");
    }
    if(vkBindImageMemory(device, depthImage, depthMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Could not bind depth memory to depth image.");
    }

    VkImageViewCreateInfo depthImageViewCreateInfo = {};
    depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewCreateInfo.image = depthImage;
    depthImageViewCreateInfo.format = VK_FORMAT_D16_UNORM;
    depthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewCreateInfo.subresourceRange.levelCount = 1;
    depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewCreateInfo.subresourceRange.layerCount = 1;
    depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    if(vkCreateImageView(device, &depthImageViewCreateInfo, nullptr, &depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("Could not create depth image view.");
    }
}

void VulkanBase::getMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t memoryFlagBitMask, uint32_t &memoryTypeIndex) {

    VkPhysicalDeviceMemoryProperties physicalDeviceMemProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemProps);

    for(size_t i = 0; i < physicalDeviceMemProps.memoryTypeCount; i++) {
       if((physicalDeviceMemProps.memoryTypes[i].propertyFlags & memoryFlagBitMask) == memoryFlagBitMask) {
            memoryTypeIndex = i;
            return;
       } 
    }

    throw std::runtime_error("Could not find suitable memory type.");
}

void VulkanBase::createUniformBuffer() {

    model = glm::mat4(1.f);
    projection = glm::perspective(45.f, 1.f, 0.1f, 100.f);
    MVP = projection * view * model;

    VkBufferCreateInfo uboCreateInfo = {};
    uboCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uboCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uboCreateInfo.size = sizeof(MVP);
    uboCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &uboCreateInfo, nullptr, &ubo) != VK_SUCCESS) {
        throw std::runtime_error("Could not create UBO.");
    }

    VkMemoryRequirements uboMemReqs;
    vkGetBufferMemoryRequirements(device, ubo, &uboMemReqs);

    uint32_t memoryTypeIndex;
    getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryTypeIndex);

    VkMemoryAllocateInfo uboAllocInfo = {};
    uboAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    uboAllocInfo.allocationSize = uboMemReqs.size;
    uboAllocInfo.memoryTypeIndex = memoryTypeIndex;

    if(vkAllocateMemory(device, &uboAllocInfo, nullptr, &uboMemory) != VK_SUCCESS) {
        throw std::runtime_error("Could not allocate UBO memory.");
    }

    void* pData;
    if(vkMapMemory(device, uboMemory, 0, uboMemReqs.size, 0, &pData)) {
        throw std::runtime_error("Could not map ubo memory.");
    }

    std::memcpy(pData, &MVP, sizeof(MVP));

    if(vkBindBufferMemory(device, ubo, uboMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Could not bind ubo to memory.");
    }
}

void VulkanBase::createDescriptorSet() {

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create descriptor set layout.");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    if(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipelinay layout.");
    }

    
   VkDescriptorPoolSize poolSizes[1];
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = 1;

   VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
   descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolCreateInfo.maxSets = 1;
   descriptorPoolCreateInfo.poolSizeCount = 1;
   descriptorPoolCreateInfo.pPoolSizes = poolSizes;

   if(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
       throw std::runtime_error("Could not create descriptor pool.");
   }

   VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
   descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocInfo.descriptorPool = descriptorPool;
   descriptorSetAllocInfo.descriptorSetCount = 1;
   descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

   descriptorSets.resize(descriptorSetAllocInfo.descriptorSetCount);
   if(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, descriptorSets.data()) != VK_SUCCESS) {
       throw std::runtime_error("Could not allocate descriptor sets.");
   }

   VkDescriptorBufferInfo uboInfo = {};
   uboInfo.buffer = ubo;
   uboInfo.offset = 0;
   uboInfo.range = sizeof(MVP);

   VkWriteDescriptorSet writeDescriptorSets[1];
   writeDescriptorSets[0] = {};
   writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSets[0].dstSet = descriptorSets[0];
   writeDescriptorSets[0].dstBinding = 0;
   writeDescriptorSets[0].dstArrayElement = 0;
   writeDescriptorSets[0].descriptorCount = 1;
   writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   writeDescriptorSets[0].pBufferInfo = &uboInfo;

   vkUpdateDescriptorSets(device, 1, writeDescriptorSets, 0, nullptr);
}


void VulkanBase::cleanUp() {
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkFreeMemory(device, uboMemory, nullptr);
    vkDestroyBuffer(device, ubo, nullptr);
    delete cam;
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthMemory, nullptr);
    for(VkCommandPool commandPool : commandPools) {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }
    for (VkImageView imageView : swapchainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

}








