#pragma once

#include "Ganymede/Core/Core.h"

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
		~Renderer() = default;

		virtual void DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, Shader& shader, bool doDepthTest) = 0;
		virtual void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Shader& shader, bool doDepthTest) = 0;
		virtual void DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Material& material, bool doDepthTest) = 0;
		virtual void ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth) = 0;

	protected:
		Renderer() = delete;
		Renderer(RenderContext& renderContext) {};
	};
}