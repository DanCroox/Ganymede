#pragma once

#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/System/Types.h"
#include <volk.h>
#include "VKBackBuffer.h"

namespace Ganymede
{
	class GANYMEDE_API VKRenderTarget : public RenderTarget
	{
	public:
		VKRenderTarget(
			RenderTargetTypes::ComponentType componentType,
			RenderTargetTypes::ChannelDataType dataType,
			RenderTargetTypes::ChannelPrecision precision,
			glm::uvec2 size);
		
		~VKRenderTarget() override;

		VKRenderTarget(VKRenderTarget&&) noexcept;
		VKRenderTarget& operator=(VKRenderTarget&&) noexcept;

		virtual VkSampleCountFlagBits GetSampleCountFlagBits() const = 0;

		bool IsValid() const override { return g_VKGetSCIndex(m_VkImage) != VK_NULL_HANDLE; }

		VkFormat m_VkFormat;
		VKBackBuffer<VkImage> m_VkImage;
		VKBackBuffer<VkImageView> m_VkImageView;
		VKBackBuffer<VkSampler> m_VkSampler;
		VkImageAspectFlags m_VkImageAspectFlags = 0;
		VKBackBuffer<VkImageLayout> m_CurrentLayout;

		const VKBackBuffer<uint32_t>& GetBindlessDSIndex() const { return m_BindlessDescriptorSetIndex; }
		bool IsBindlessDSIndexValid() const { return g_VKGetSCIndex(m_BindlessDescriptorSetIndex) != Numbers::MAX_UINT32; }

		void ResetBindlessDSIndex();
		VKBackBuffer<uint32_t> m_BindlessDescriptorSetIndex;
	};

	class VKRenderTargetEmpty : public VKRenderTarget
	{
	public:
		enum class Layout
		{
			AttachmentWrite,
			ShaderRead,
			TransferSrc,
			TransferRead
		};

		VKRenderTargetEmpty(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		~VKRenderTargetEmpty() override;

		VKRenderTargetEmpty(VKRenderTargetEmpty&&) noexcept;
		VKRenderTargetEmpty& operator=(VKRenderTargetEmpty&&) noexcept;

		void Clear(
			unsigned int mipLayer,
			unsigned int destX,
			unsigned int destY,
			unsigned int destDepth,
			unsigned int extendX,
			unsigned int extendY,
			unsigned int extendDepth,
			const float* normalizedClearColor) override;

		void SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value) override;

		virtual VkSampleCountFlagBits GetSampleCountFlagBits() const override { return VK_SAMPLE_COUNT_1_BIT; }

	protected:
		VKBackBuffer<VkDeviceMemory> m_VkImageMemory;
		VkAccessFlags m_VkCurrentAccessMask = VK_IMAGE_LAYOUT_UNDEFINED;
		VkPipelineStageFlags  m_VkCurrentStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	};

	class VKSinglesampleRenderTarget : public VKRenderTargetEmpty
	{
	public:
		VKSinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
	};

	class VKMultisampleRenderTarget : public VKRenderTargetEmpty
	{
	public:
		VKMultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
	};

	class VKCubeMapArrayRenderTarget : public VKRenderTargetEmpty
	{
	public:
		VKCubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
	};

	class VKRenderTargetImageWrapper : public VKRenderTarget
	{
	public:
		VKRenderTargetImageWrapper(
			VKBackBuffer<VkImage> image,
			VKBackBuffer<VkImageView> imageView,
			VkFormat m_VkFormat,
			VkImageAspectFlags m_VkImageAspectFlags,
			RenderTargetTypes::ComponentType componentType,
			RenderTargetTypes::ChannelDataType dataType,
			RenderTargetTypes::ChannelPrecision precision,
			glm::uvec2 size);

		void Clear(
			unsigned int mipLayer,
			unsigned int destX,
			unsigned int destY,
			unsigned int destDepth,
			unsigned int extendX,
			unsigned int extendY,
			unsigned int extendDepth,
			const float* normalizedClearColor) override {}

		void SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value) override {}

		virtual VkSampleCountFlagBits GetSampleCountFlagBits() const override { return VK_SAMPLE_COUNT_1_BIT; }
	};
}