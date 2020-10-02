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
        std::vector<VkExtensionProperties>getAvailableInstanceExtensions(bool print);
        std::vector<const char*>getRequiredInstanceExtensions(bool print);

        void createSurface();
        void findSuitablePhysicalDevice(bool print);
        bool checkPhysicalDeviceExtensions(bool print);
        void createLogicalDevice();

        void createSwapchain();
        VkSurfaceFormatKHR getSurfaceFormat();
        uint32_t getMinImageCount(VkSurfaceCapabilitiesKHR surfaceCapabilities);
        VkExtent2D getSwapExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
        VkPresentModeKHR getPresentMode();

        void createImageViews();

        void cleanUp();

    private:
        bool enableValidationLayers;
        std::vector<const char*> requiredLayers;
        std::vector<const char*> requiredInstanceExtensions;
        std::vector<const char*> requiredDeviceExtensions = {"VK_KHR_swapchain"};
        uint32_t graphicsQueueIndex;
        uint32_t presentQueueIndex;

        Window* pWindow;
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSwapchainKHR swapchain;
        VkSurfaceFormatKHR surfaceFormat;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
};


#endif
