#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class RenderTarget;

	class GANYMEDE_API FrameBuffer
	{
	public:
		enum class AttachmentType
		{
			Color0,
			Color1,
			Color2,
			Color3,
			Color4,
			Color5,
			Color6,
			Color7,
			Color8,
			Color9,
			Color10,
			Color11,
			Color12,
			Color13,
			Color14,
			Color15,
			Depth
		};

		enum class BlitFilterType
		{
			Linear,
			Nearest
		};

		class BlitFrameBufferConfig
		{
		public:
			struct BlitAttachementInfo
			{
				FrameBuffer& m_SourceFrameBuffer;
				FrameBuffer& m_DestFrameBuffer;
				FrameBuffer::AttachmentType m_SourceAttachement;
				FrameBuffer::AttachmentType m_DestAttachement;
				glm::u32vec4 m_SourcePixelBounds;
				glm::u32vec4 m_DestPixelBounds;
				BlitFilterType m_FilterType;
			};

			std::vector<BlitAttachementInfo> m_AttachementsToBlit;
		};

		virtual ~FrameBuffer() = default;

		virtual void SetFrameBufferAttachment(AttachmentType attachmentType, RenderTarget& frameBufferTexture) = 0;
		virtual void SetColorBufferClearColor(const glm::vec4& color) = 0;
		virtual void SetDepthBufferClearColor(float color) = 0;

		virtual const std::unordered_map<AttachmentType, RenderTarget*>& GetFrameBufferAttachments() const = 0;
		virtual glm::u32vec2 GetRenderDimension() const = 0;
		virtual const glm::vec4& GetColorBufferClearColor() const = 0;
		virtual float GetDepthBufferClearColor() const = 0;

		virtual bool IsValid() const = 0;

	protected:
		FrameBuffer() = delete;
		FrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer) {};

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&& other) noexcept = default;
		FrameBuffer& operator=(FrameBuffer&& other) noexcept = default;
	};
}