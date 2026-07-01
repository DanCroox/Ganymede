#pragma once

#include "Ganymede/Data/Handle.h"
#include "Ganymede/System/FreeList.h"
#include "vulkan_device.h"
#include <volk.h>
#include "GLFW/glfw3.h"
#include <optional>
#include <array>

namespace Ganymede
{
	// Forward declarations
	template <typename T>
	class VKDataBuffer;

	class Material;
	class VKCommandList;
	class VKDescriptorSetPoolManager;
	class VKFrameBuffer;
	class VKGPUTexture;
	class VKRenderTarget;
	class VKRenderTargetImageWrapper;
	class VKSSBO;
	class VKVertexObject;

	class VKContext
	{
	public:
		struct SSBOBindingInfo
		{
			uint32_t m_BindingPoint;
			const VKSSBO& m_SSBO;
			uint32_t m_ByteOffset = 0;
			uint32_t m_Range = 0;
		};

		struct TextureBindingInfo
		{
			unsigned int m_BindingPoint;
			VkSampler m_VkSampler;
			VkImageView m_VkImageView;
			uint32_t m_Offset = 0;
		};

		static VKContext& GetInstance();
		
		void Initialize(const char* appName, GLFWwindow& glfwWindow);
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		uint32_t GetApiVersion() const { return VK_API_VERSION_1_4; }

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VKCommandList& GetCommandList() const { return *m_CommandList; }

		std::pair<VkDescriptorSetLayout, VkDescriptorSet> CreateDescriptorSetDefinition(std::vector<VkDescriptorSetLayoutBinding> dsLayoutBindings);
		void BindToDescriptorSet(VkDescriptorSet descriptorSet, const std::vector<SSBOBindingInfo>& ssboBindings);
		void BindToDescriptorSet(VkDescriptorSet descriptorSet, const std::vector<TextureBindingInfo>& textureBindings);

		void RegisterBindlessVKGPUTexture(VKGPUTexture& vkGPUTexture);
		void RegisterBindlessVKRenderTarget(VKRenderTarget& vkRenderTarget);
		void RegisterBindlessVKSSBO(VKSSBO& ssbo);
		void UnregisterBindlessVKGPUTexture(VKGPUTexture& vkGPUTexture);
		void UnregisterBindlessVKRenderTarget(VKRenderTarget& vkRenderTarget);
		void UnregisterBindlessVKSSBO(VKSSBO& ssbo);
	
	private:
		VKContext() = default;

		template <typename T>
		friend class VKDataBuffer;

		bool m_IsInitialized = false;

	public:
		VKDescriptorSetPoolManager* m_DescriptorSetPoolManager;

		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface;
		vkutil::VulkanDevice device;

		VkSurfaceFormatKHR surfaceFormat;
		VkExtent2D extent;
		uint32_t m_SCImgCount = 0;
		uint32_t m_NumFIF = 0;
		std::vector<VkImage> swapImages;
		VkSwapchainKHR swapchain;

		std::unique_ptr<VKCommandList> m_CommandList;

		uint32_t m_FiFIndex = 0;
		uint32_t m_SCIndex = 0;

		std::unique_ptr<VKRenderTargetImageWrapper> m_SCImageRenderTargets;

		VkDescriptorSetLayout m_BindlessDescriptorSetLayout;
		VkDescriptorSet m_BindlessDescriptorSet;
		FreeList m_BindlessTextureFreeList;
		FreeList m_BindlessSSBOFreeList;
	};
}