#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;
	class VertexObject;
	class Pipeline;

	struct PCData
	{
		glm::uint m_BufferIndex;
		glm::uint m_DataIndex;
		glm::uvec2 _PadA;
	};

	class GANYMEDE_API CommandList
	{
	public:
		virtual ~CommandList() = default;

		virtual void Reset() = 0;
		virtual void Begin() = 0;
		virtual void End() = 0;
		
		virtual void BindFrameBuffer(const FrameBuffer& framebuffer) = 0;
		virtual void BindPipeline(const Pipeline& pipeline) = 0;

		virtual void DrawGeometry(const VertexObject& vertexObject, PCData pcData) = 0;
		
		virtual void DrawFullscreenQuad(PCData pcData) = 0;

	protected:
		CommandList() = default;
	};
}