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

			FrameBuffer() = delete;
			FrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer);
			~FrameBuffer();

			unsigned int GetRenderID() const { return m_RenderID; }

			void SetFrameBufferAttachment(AttachmentType attachmentType, RenderTarget& frameBufferTexture);

			void SetColorBufferClearColor(const glm::vec4& color) { m_ColorBufferClearColor = color; }
			const glm::vec4& GetColorBufferClearColor() const { return m_ColorBufferClearColor; }

			void SetDepthBufferClearColor(float color) { m_DepthBufferClearColor = color; }
			float GetDepthBufferClearColor() const { return m_DepthBufferClearColor; }

			void SetRenderDimension(glm::u32vec2 dimension) { m_RenderDimension = dimension; }
			glm::u32vec2 GetRenderDimension() const { return m_RenderDimension; }

			const std::unordered_map<AttachmentType, RenderTarget*>& GetFrameBufferAttachments() const { return m_FrameBufferAttachments; }

			inline bool IsValid() const { return m_IsHardwareFrameBuffer || m_RenderID != 0; }

			enum class BlitFilterType
			{
				Linear,
				Nearest
			};

			struct BlitFrameBufferConfig
			{
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

			static void Blit(const BlitFrameBufferConfig& blitConfig);

		private:
			void UpdateActiveDrawBufferAttachments();

			bool m_IsHardwareFrameBuffer;
			unsigned int m_RenderID;

			glm::u32vec2 m_RenderDimension;

			std::unordered_map<AttachmentType, RenderTarget*> m_FrameBufferAttachments;
			glm::vec4 m_ColorBufferClearColor;
			float m_DepthBufferClearColor;
	};
}