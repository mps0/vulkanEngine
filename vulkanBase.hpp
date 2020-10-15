#ifndef VULKANBASE_HPP
#define VULKANBASE_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <set>
#include <string>

#include "window.hpp"
#include "camera.hpp"
#include "vertex.hpp"
#include "model.hpp"


class VulkanBase {
    public:
        Camera* cam = new Camera(&MVP.view, glm::vec3(0.f, -.7071f, -.7071f), glm::vec3(0.f, .7071f, -.7071f), glm::vec3(0.f, 1.f, 0.f));

        VulkanBase(Window* pWindow, Scene* pScene, bool enableValidationLayers);
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
        void createFramebuffers();

        void createVertexBuffer();
        void createIndexBuffer();

        void createGraphicsPipeline();

        void createSyncObjects();

        void draw();


        void updateMVP();        


        void cleanUp();

    private:
        bool enableValidationLayers;

        Scene* pScene;

        struct uniformBufferObject{
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        };

        uniformBufferObject MVP = {};

        std::vector<const char*> requiredLayers;
        std::vector<const char*> requiredInstanceExtensions;
        std::vector<const char*> requiredDeviceExtensions = {"VK_KHR_swapchain"};
        uint32_t graphicsQueueIndex;
        uint32_t presentQueueIndex;
        std::set<uint32_t> uniqueQueues;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
 
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
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        VkImage depthImage;
        VkFormat depthFormat;
        VkDeviceMemory depthMemory;
        VkImageView depthImageView;

        VkBuffer ubo;
        VkDeviceMemory uboMemory;
        void* pUboData;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkPipelineLayout pipelineLayout;
        std::vector<VkDescriptorSet> descriptorSets;

        VkRenderPass renderPass;

        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;

        std::vector<VkFramebuffer> framebuffers;

        VkBuffer vertBuffer;
        VkDeviceMemory vertBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;

        VkPipeline pipeline;

        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        


};




#endif
