#include "Renderer.h"

#include "gl/glew.h"

#include "FrameBuffer.h"
#include "Material.h"
#include "OGLContext.h"
#include "RenderContext.h"
#include "Shader.h"
#include "SSBO.h"
#include "VertexObject.h"

namespace Ganymede
{
	Renderer::Renderer(RenderContext& renderContext) :
		m_ViewportDimension({0, 0}),
		m_DoDepthTesting(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE),
		m_RenderContext(renderContext)
	{}

	void Renderer::DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, Shader& shader, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);
		OGLContext::BindShader(shader.GetRendererID());

		glDrawElementsInstanced(GL_TRIANGLES, vertexObject.GetVertexObjectIndexBuffer().GetNumIndices(), GL_UNSIGNED_INT, 0, numInstances);
	}

	void Renderer::DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Material& material, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);

		m_RenderContext.BindMaterial(material);
		OGLContext::BindIndirectDrawBuffer(indirectCommandsBuffer);

		const unsigned int byteOffset = commandOffset * 20;
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*) byteOffset);
	}

	void Renderer::DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Shader& shader, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);

		OGLContext::BindShader(shader.GetRendererID());
		OGLContext::BindIndirectDrawBuffer(indirectCommandsBuffer);

		const unsigned int byteOffset = commandOffset * 20;
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)byteOffset);
	}

	void Renderer::ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth)
	{
		unsigned int flags = 0;
		if (clearColor) 
		{
			const glm::vec4 color = frameBuffer.GetColorBufferClearColor();
			glClearColor(color.x, color.y, color.z, color.a);
			flags |= GL_COLOR_BUFFER_BIT;
		}
		
		if (clearDepth)
		{
			glClearDepth(frameBuffer.GetDepthBufferClearColor());
			flags |= GL_DEPTH_BUFFER_BIT;
		}

		if (flags == 0)
		{
			return;
		}

		OGLContext::BindFrameBuffer(frameBuffer);
		glClear(flags);
	}

	void Renderer::PrepareDraw(const VertexObject& vertexObject, FrameBuffer& frameBuffer, bool doDepthTest)
	{
		GM_CORE_ASSERT(m_ViewportDimension.x == 0 || m_ViewportDimension.y == 0, "Invalid viewport size (0x0 pixels).");

		OGLContext::BindFrameBuffer(frameBuffer);
		OGLContext::BindVertexArrayObject(vertexObject.GetRenderID());

		if (doDepthTest != m_DoDepthTesting)
		{
			if (doDepthTest)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			m_DoDepthTesting = doDepthTest;
		}
	}
}