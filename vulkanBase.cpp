#include "vulkanBase.hpp"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"
#include <cstring>
#include <fstream>


#include <vulkan/vulkan_core.h>


VulkanBase::VulkanBase(Window* pWindow, Scene* pScene, bool enableValidationLayers) : pWindow(pWindow), pScene(pScene), enableValidationLayers(enableValidationLayers) {

vertices = *(pScene->getVerts());
indices = *(pScene->getInds());

};

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

    commandBuffers.resize(framebuffers.size());

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;

        if(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool."); 
        }

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {}; 
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = commandBuffers.size();

        if(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Could not allocate command buffers.");
        } 

        for(size_t i = 0; i < commandBuffers.size(); i++) {

            VkCommandBufferBeginInfo commandBufferBeginInfo = {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if(vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Could not begin command buffer.");

            }

        VkClearValue clearValues[2];
        clearValues[0].color = {0.f, 0.f, 0.f, 0.f};
        clearValues[1].depthStencil.depth = 1.f;
        VkRenderPassBeginInfo  renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers[i];
        renderPassBeginInfo.renderArea.extent.width = SCREEN_WIDTH;
        renderPassBeginInfo.renderArea.extent.height = SCREEN_HEIGHT;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;


    vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets.data(), 0, nullptr);
    

    
    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float) SCREEN_WIDTH;
    viewport.height = (float) SCREEN_HEIGHT;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor;
    scissor.extent.width = SCREEN_WIDTH;
    scissor.extent.height = SCREEN_HEIGHT;
    scissor.offset.x = 0.f;
    scissor.offset.y = 0.f;
    


    vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

    
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertBuffer, offsets);


    vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32); 
    

    uint32_t start = 0;
    uint32_t end;
    for(size_t j = 0; j < pScene->getModelCount(); j++) { 
        end = pScene->modelMarkers[j];
        uint32_t num = end - start;
        vkCmdDrawIndexed(commandBuffers[i], num, 1, start, 0, 0);
        start = end;
    }

    vkCmdEndRenderPass(commandBuffers[i]);

    if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("Could not record command buffer.");
    }



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
    depthImageCreateInfo.queueFamilyIndexCount = graphicsQueueIndex;
    depthImageCreateInfo.pQueueFamilyIndices = nullptr;
    depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    depthFormat = depthImageCreateInfo.format;

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

    MVP.model = glm::mat4(1.f);
    //MVP.model[3] = glm::vec4(0.f, 0.f, -10.f, 1.f);
    cam->updateView();
    MVP.projection = glm::perspective(45.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.f);
    //account for glm y down
    MVP.projection[1][1] *= -1;

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

    if(vkBindBufferMemory(device, ubo, uboMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Could not bind ubo to memory.");
    }

    //void* pData;
    if(vkMapMemory(device, uboMemory, 0, uboMemReqs.size, 0, &pUboData)) {
        throw std::runtime_error("Could not map ubo memory.");
    }

    updateMVP();
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

void VulkanBase::createRenderPass() {
    VkAttachmentDescription attachmentDescription[2];

    attachmentDescription[0] = {};
    attachmentDescription[0].format = surfaceFormat.format;
    attachmentDescription[0].samples = VK_SAMPLE_COUNT_1_BIT; 
    attachmentDescription[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachmentDescription[1] = {};
    attachmentDescription[1].format = depthFormat;
    attachmentDescription[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthRef;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create render pass.");
    }
    
}

void VulkanBase::createFramebuffers() {

    framebuffers.resize(swapchainImageViews.size());

    VkImageView attachments[2];
    attachments[1] = depthImageView;

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = attachments; 
    framebufferCreateInfo.width = SCREEN_WIDTH;
    framebufferCreateInfo.height = SCREEN_HEIGHT;
    framebufferCreateInfo.layers = 1;
    
    for(size_t i = 0; i < framebuffers.size(); i++) {
        attachments[0] =  swapchainImageViews[i];

        if(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("Could not create framebuffer.");
    }
}

void VulkanBase::createVertexBuffer() {


    VkBufferCreateInfo vertBufferCreateInfo = {};
    vertBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertBufferCreateInfo.size = sizeof(vertices[0]) * vertices.size();
    vertBufferCreateInfo.queueFamilyIndexCount = graphicsQueueIndex;
    vertBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &vertBufferCreateInfo, nullptr, &vertBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Could not create vertex buffer.");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, vertBuffer, &memReqs);

    uint32_t memoryTypeIndex;
    getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryTypeIndex);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if(vkAllocateMemory(device, &allocInfo, nullptr, &vertBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Could not allocate vertex buffer memory.");
    }

    if(vkBindBufferMemory(device, vertBuffer, vertBufferMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Could not bind vertex buffer to memory.");
    }

    void* pData;
    if(vkMapMemory(device, vertBufferMemory, 0, memReqs.size, 0, &pData)) {
        throw std::runtime_error("Could not map vertex buffer memory.");
    }

    std::memcpy(pData, vertices.data(), sizeof(vertices[0]) * vertices.size());
    vkUnmapMemory(device, vertBufferMemory);
}

void VulkanBase::createIndexBuffer() {

    VkBufferCreateInfo indexBufferCreateInfo = {};
    indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.size = sizeof(indices[0]) * indices.size();
    indexBufferCreateInfo.queueFamilyIndexCount = graphicsQueueIndex;
    indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &indexBufferCreateInfo, nullptr, &indexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Could not create index buffer.");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, indexBuffer, &memReqs);

    uint32_t memoryTypeIndex;
    getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryTypeIndex);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if(vkAllocateMemory(device, &allocInfo, nullptr, &indexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Could not allocate index buffer memory.");
    }

    if(vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Could not bind index buffer to memory.");
    }

    void* pData;
    if(vkMapMemory(device, indexBufferMemory, 0, memReqs.size, 0, &pData)) {
        throw std::runtime_error("Could not map index buffer memory.");
    }

    std::memcpy(pData, indices.data(), sizeof(indices[0]) * indices.size());
    vkUnmapMemory(device, indexBufferMemory);

}

void VulkanBase::createGraphicsPipeline() {

    #include "shaders/vert.spv"
    VkShaderModuleCreateInfo vertShaderModuleCreateInfo = {};
    vertShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleCreateInfo.codeSize = sizeof(vertShader);
    vertShaderModuleCreateInfo.pCode = vertShader;

    if(vkCreateShaderModule(device, &vertShaderModuleCreateInfo, nullptr, &vertShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Could not create vertex shader module.");
    }

    #include "shaders/frag.spv"
    VkShaderModuleCreateInfo fragShaderModuleCreateInfo = {};
    fragShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleCreateInfo.codeSize = sizeof(fragShader);
    fragShaderModuleCreateInfo.pCode = fragShader;

    if(vkCreateShaderModule(device, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Could not create fragment shader module.");
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo(2);
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    shaderStagesCreateInfo[0] = vertShaderStageCreateInfo;
    shaderStagesCreateInfo[1] = fragShaderStageCreateInfo;

    VkDynamicState dynamicStateEnables[2]; //viewport + scissor
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;


    VkVertexInputAttributeDescription attribDescription[3];
    attribDescription[0] = {};
    attribDescription[0].location = 0;
    attribDescription[0].binding = 0;
    attribDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribDescription[0].offset = 0;

    attribDescription[1] = {};
    attribDescription[1].location = 1;
    attribDescription[1].binding = 0;
    attribDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribDescription[1].offset = 12;

    attribDescription[2] = {};
    attribDescription[2].location = 2;
    attribDescription[2].binding = 0;
    attribDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribDescription[2].offset = 24;

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(vertices[0]);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    VkPipelineVertexInputStateCreateInfo vertInputStateCreateInfo = {};
    vertInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInputStateCreateInfo.vertexAttributeDescriptionCount = 3;
    vertInputStateCreateInfo.pVertexAttributeDescriptions = attribDescription;
    vertInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
    rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    //rastCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rastCreateInfo.depthClampEnable = VK_FALSE;
    rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rastCreateInfo.depthBiasEnable = VK_FALSE;
    rastCreateInfo.depthBiasConstantFactor = 0;
    rastCreateInfo.lineWidth = 1.f;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = 0xf;
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
    colorBlendCreateInfo.blendConstants[0] = 1.f;
    colorBlendCreateInfo.blendConstants[1] = 1.f;
    colorBlendCreateInfo.blendConstants[2] = 1.f;
    colorBlendCreateInfo.blendConstants[3] = 1.f;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pViewports = nullptr;
    viewportStateCreateInfo.pScissors = nullptr;

    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
    depthStencilStateCreateInfo.sType =  VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    //depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateCreateInfo.back.compareMask = 0;
    depthStencilStateCreateInfo.back.reference = 0;
    depthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.writeMask = 0;
    depthStencilStateCreateInfo.front = depthStencilStateCreateInfo.back;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
    multisampleStateCreateInfo.minSampleShading = 0.f;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = 0;
    pipelineCreateInfo.pVertexInputState = &vertInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rastCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pStages = shaderStagesCreateInfo.data();
    pipelineCreateInfo.stageCount = shaderStagesCreateInfo.size();
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;

    if(vkCreateGraphicsPipelines(device, nullptr, 1,&pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline.");
    }
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

        void VulkanBase::draw() {

            uint32_t imageIndex;
            if(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
                    throw std::runtime_error("Could not aquire next swapchain image.");
                    }

           VkPipelineStageFlags pipelineStageFlags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

           VkSubmitInfo submitInfo = {};
           submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
           submitInfo.waitSemaphoreCount = 1;
           submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
           submitInfo.pWaitDstStageMask = &pipelineStageFlags;
           submitInfo.commandBufferCount = 1;
           submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
           submitInfo.signalSemaphoreCount = 1;
           submitInfo.pSignalSemaphores = &renderFinishedSemaphore;


           if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
               throw std::runtime_error("Could not submit draw command buffer");
           } 


           VkPresentInfoKHR presentInfo = {};
           presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
           presentInfo.swapchainCount = 1;
           presentInfo.pSwapchains = &swapchain;
           presentInfo.pImageIndices = &imageIndex;
           presentInfo.waitSemaphoreCount = 1;
           presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
           presentInfo.pResults = nullptr;

           if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS) {
               throw std::runtime_error("Could not present.");
           }

           vkQueueWaitIdle(presentQueue);
        }


        void VulkanBase::createSyncObjects() {
    
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if((vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS) 
            ||
//
            (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)) 
            {
                throw std::runtime_error("Could not create semaphores.");
            }

        }

//TODO: use push constants instead of ubo buffer
void VulkanBase::updateMVP() {

    //MVP = projection * view * model;
    std::memcpy(pUboData, &MVP, sizeof(MVP));
}

void VulkanBase::cleanUp() {
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, vertBufferMemory, nullptr);
    vkDestroyBuffer(device, vertBuffer, nullptr);
    for(VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkFreeMemory(device, uboMemory, nullptr);
    vkDestroyBuffer(device, ubo, nullptr);
    delete cam;
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthMemory, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (VkImageView imageView : swapchainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}








