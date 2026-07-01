#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class FrameBuffer;
	class GraphicsShader;
	class Material;
	class RenderContext;
	class SSBO;
	class VertexObject;

	class GANYMEDE_API Renderer
	{
	public:
		~Renderer() = default;

		virtual void DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, GraphicsShader& shader, bool doDepthTest) = 0;
		virtual void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const GraphicsShader& shader, bool doDepthTest) = 0;
		virtual void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Material& material, bool doDepthTest) = 0;
		virtual void ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth) = 0;

	protected:
		Renderer() = delete;
		Renderer(RenderContext& renderContext) {};
	};
}