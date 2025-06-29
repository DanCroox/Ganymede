#pragma once

#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;
	class Material;
	class RenderContext;
	class Shader;
	class SSBO;
	class VertexObject;

	class GANYMEDE_API Renderer
	{
	public:
		Renderer() = delete;
		Renderer(RenderContext& renderContext);
		
		~Renderer() = default;


		void DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, Shader& shader, bool doDepthTest);
		void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Material& material, bool doDepthTest);
		void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Shader& shader, bool doDepthTest);
		void ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth);

	private:
		void PrepareDraw(const VertexObject& vertexObject, FrameBuffer& frameBuffer, bool doDepthTest);
		
		RenderContext& m_RenderContext;
		glm::u32vec2 m_ViewportDimension;
		bool m_DoDepthTesting;
	};
}