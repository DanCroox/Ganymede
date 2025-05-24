#pragma once

#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;
	class Shader;
	class VertexObject;

	class GANYMEDE_API Renderer
	{
	public:
		Renderer();
		~Renderer() = default;

		void DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, Shader& shader, bool doDepthTest);
		void ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth);

	private:
		glm::u32vec2 m_ViewportDimension;
		bool m_DoDepthTesting;
	};
}