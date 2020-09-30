#ifndef VULKANBASE_HPP
#define VULKANBASE_HPP

#include <vulkan/vulkan.h>

#include <vector>

#include "window.hpp"



class VulkanBase {
    public:
        VulkanBase(Window* pWindow, bool enableValidationLayers);
        void createInstance();
        std::vector<VkLayerProperties>getAvailableLayers(bool print);
        std::vector<const char*>getRequiredLayers(bool print);
        std::vector<VkExtensionProperties>getAvailableExtensions(bool print);
        std::vector<const char*>getRequiredExtensions(bool print);

        void createSurface();
        void findSuitablePhysicalDevice(bool print);
        void createLogicalDevice();


        void cleanUp();

    private:
        bool enableValidationLayers;
        std::vector<const char*> requiredLayers;
        std::vector<const char*> requiredExtensions;
        uint32_t graphicsQueueIndex;
        uint32_t presentQueueIndex;

        Window* pWindow;
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
};


#endif
