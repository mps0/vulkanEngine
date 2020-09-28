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
        void cleanUp();

    private:
        Window* pWindow;
        VkInstance instance;
        bool enableValidationLayers;
};


#endif
