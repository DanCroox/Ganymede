#pragma once

// Vulkan headers
#include <volk.h>

// std
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <stdexcept>

namespace vkutil {

    // Hilfs-structs
    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;

        bool isComplete() const {
            return graphicsFamilyHasValue && presentFamilyHasValue;
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    // Utility: Create/Destroy debug messenger (extern, weil Instance auﬂen erstellt werden kann)
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    // Simple device wrapper (engine-free)
    class VulkanDevice {
    public:
        VulkanDevice() = default;
        ~VulkanDevice() = default;

        void Initialize(VkInstance instance, VkSurfaceKHR surface,
            const std::vector<const char*>& deviceExtensions = {},
            bool enableValidationLayers = false,
            const std::vector<const char*>& validationLayers = {});

        void Shutdown();

        // Non-copyable
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;

        VkDevice device() const { return device_; }
        VkPhysicalDevice physicalDeviceHandle() const { return physicalDevice_; }
        VkQueue graphicsQueue() const { return graphicsQueue_; }
        VkQueue presentQueue() const { return presentQueue_; }
        VkCommandPool commandPoolHandle() const { return commandPool; }

        // Helpers (buffer/image/memory)
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    private:
        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        QueueFamilyIndices findPhysicalQueueFamilies(); // uses stored physicalDevice_

        // members
        VkInstance instance_;
        VkSurfaceKHR surface_;

        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
        VkDevice device_ = VK_NULL_HANDLE;

        VkQueue graphicsQueue_ = VK_NULL_HANDLE;
        VkQueue presentQueue_ = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        std::vector<const char*> deviceExtensions_;
        bool enableValidationLayers_;
        std::vector<const char*> validationLayers_;
    };

} // namespace vkutil