#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"
#include <optional>
#include <array>

namespace Ganymede
{
	class RenderTarget;

	enum class FrameBufferAttachmentTypee
	{
		Color,
		Depth
	};

	struct FrameBufferAttachment
	{
		FrameBufferAttachmentTypee m_AttachmentType;
		uint32_t m_AttachmentLocation;
		const RenderTarget* m_RenderTarget = nullptr;
	};

	enum class FrameBufferBlitFilterType
	{
		Linear,
		Nearest
	};

	class FrameBufferAttachmentStorage
	{
	public:
		FrameBufferAttachmentStorage() = default;
		FrameBufferAttachmentStorage(std::initializer_list<FrameBufferAttachment> attachments);

		auto begin() { return m_BindingLocations.begin(); }
		auto end() { return m_BindingLocations.begin() + m_NumEntries; }
		auto begin() const { return m_BindingLocations.begin(); }
		auto end()   const { return m_BindingLocations.begin() + m_NumEntries; }

		uint32_t size() const { return m_NumEntries; }
		uint32_t empty() const { return m_NumEntries == 0; }

		const FrameBufferAttachment* TryGetByBindingLocation(uint32_t locationBindingID) const;

	private:
		static constexpr uint32_t MAX_BINDING_LOCATIONS = 16;
		uint32_t m_NumEntries = 0;
		std::array<uint32_t, MAX_BINDING_LOCATIONS> m_BindingLocations;
		std::array<FrameBufferAttachment, MAX_BINDING_LOCATIONS> m_Attachments;
	};

	class GANYMEDE_API FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		void SetColorBufferClearColor(const glm::vec4& color) { m_ColorBufferClearColor = color; }
		void SetDepthBufferClearColor(float color) { m_DepthBufferClearColor = color; }

		const glm::vec4& GetColorBufferClearColor() const { return m_ColorBufferClearColor; }
		float GetDepthBufferClearColor() const { return m_DepthBufferClearColor; }

		glm::u32vec2 GetRenderDimension() const { return m_RenderDimension; }

		const FrameBufferAttachmentStorage& GetAttachments() const { return m_Attachments; };

		virtual void Blit(
			FrameBuffer& m_SourceFrameBuffer,
			FrameBufferAttachmentTypee m_SourceAttachementType,
			uint32_t m_SourceAttachementLocation,
			FrameBufferAttachmentTypee m_DestAttachementType,
			uint32_t m_DestAttachementLocation,
			const glm::u32vec4& m_SourcePixelBounds,
			const glm::u32vec4& m_DestPixelBounds,
			FrameBufferBlitFilterType m_FilterType) = 0;

		virtual bool IsValid() const = 0;

	protected:
		FrameBuffer() = delete;
		FrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension) :
			m_Attachments(attachments),
			m_RenderDimension(renderDimension),
			m_ColorBufferClearColor(0),
			m_DepthBufferClearColor(1)
		{}

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&& other) noexcept = default;
		FrameBuffer& operator=(FrameBuffer&& other) noexcept = default;

		glm::u32vec2 m_RenderDimension;
		glm::vec4 m_ColorBufferClearColor;
		float m_DepthBufferClearColor;
		FrameBufferAttachmentStorage m_Attachments;
	};
}