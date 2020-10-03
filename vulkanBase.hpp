#ifndef VULKANBASE_HPP
#define VULKANBASE_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <set>

#include "window.hpp"
#include "camera.hpp"


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
        void createCommandBuffers(); 
        void createDepthBuffer();
        void getMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t memoryFlagBitMask, uint32_t &memoryTypeIndex);
        void createUniformBuffer();
        void createDescriptorSet();
        void createRenderPass();


        void cleanUp();

    private:
        bool enableValidationLayers;
        std::vector<const char*> requiredLayers;
        std::vector<const char*> requiredInstanceExtensions;
        std::vector<const char*> requiredDeviceExtensions = {"VK_KHR_swapchain"};
        uint32_t graphicsQueueIndex;
        uint32_t presentQueueIndex;
        std::set<uint32_t> uniqueQueues;

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
        std::vector<VkCommandPool> commandPools;
        std::vector<VkCommandBuffer> commandBuffers;

        VkImage depthImage;
        VkFormat depthFormat;
        VkDeviceMemory depthMemory;
        VkImageView depthImageView;

        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 MVP;
        Camera* cam = new Camera(&view);
        VkBuffer ubo;
        VkDeviceMemory uboMemory;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkPipelineLayout pipelineLayout;
        std::vector<VkDescriptorSet> descriptorSets;

        VkRenderPass renderPass;


};




#endif
