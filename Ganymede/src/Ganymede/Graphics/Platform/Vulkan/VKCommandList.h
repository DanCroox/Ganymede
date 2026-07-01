#pragma once
#include "Ganymede/Graphics/CommandList.h"
#include "VKBackBuffer.h"
#include <volk.h>

namespace Ganymede
{
	class VKFrameBuffer;
	class VKPipeline;

	class GANYMEDE_API VKCommandList : public CommandList
	{
	public:
		VKCommandList(VKBackBuffer<VkCommandBuffer> vkCommandBuffer);
		~VKCommandList() override;

		void Reset() override;
		void Begin() override;
		void End() override;

		void BindFrameBuffer(const FrameBuffer& framebuffer) override;
		void BindPipeline(const Pipeline& pipeline) override;

		void DrawGeometry(const VertexObject& vertexObject, PCData pcData) override;

		void DrawFullscreenQuad(PCData pcData) override;

		const VKBackBuffer<VkCommandBuffer>& GetVkCommandBuffer() const { return m_VkCommandBuffer; }

	private:
		VKBackBuffer<VkCommandBuffer> m_VkCommandBuffer;
		VkCommandBufferBeginInfo m_BeginInfo{};
		const VKPipeline* m_BoundPipeline = nullptr;
		const VKFrameBuffer* m_BoundFramebuffer = nullptr;
	};
}