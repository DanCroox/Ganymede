#include "OGLRenderer.h"

#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Shader.h"
#include "OGLGraphicsShader.h"
#include "OGLSSBO.h"
#include "OGLFrameBuffer.h"
#include "OGLVertexObject.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "OGLContext.h"
#include <GL/glew.h>

namespace Ganymede
{
	OGLRenderer::OGLRenderer(RenderContext& renderContext) :
		Renderer(renderContext),
		m_ViewportDimension({ 0, 0 }),
		m_DoDepthTesting(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE),
		m_RenderContext(renderContext)
	{}

	void OGLRenderer::DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, GraphicsShader& shader, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);
		OGLContext::BindShader(static_cast<const OGLGraphicsShader&>(shader).GetRendererID());

		glDrawElementsInstanced(GL_TRIANGLES, static_cast<OGLVertexObject&>(vertexObject).GetVertexObjectIndexBuffer().GetNumIndices(), GL_UNSIGNED_INT, 0, numInstances);
	}

	void OGLRenderer::DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const GraphicsShader& shader, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);

		OGLContext::BindShader(static_cast<const OGLGraphicsShader&>(shader).GetRendererID());
		OGLContext::BindIndirectDrawBuffer(static_cast<OGLSSBO&>(indirectCommandsBuffer));

		const unsigned int byteOffset = commandOffset * 20;
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)byteOffset);
	}

	void OGLRenderer::DrawIndirect(const VertexObject& vertexObject, SSBO& indirectCommandsBuffer, unsigned int commandOffset, FrameBuffer& frameBuffer, const Material& material, bool doDepthTest)
	{
		PrepareDraw(vertexObject, frameBuffer, doDepthTest);

		m_RenderContext.BindMaterial(material);
		OGLContext::BindIndirectDrawBuffer(static_cast<OGLSSBO&>(indirectCommandsBuffer));

		const unsigned int byteOffset = commandOffset * 20;
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)byteOffset);
	}

	void OGLRenderer::ClearFrameBuffer(FrameBuffer& frameBuffer, bool clearColor, bool clearDepth)
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

		OGLContext::BindFrameBuffer(static_cast<const OGLFrameBuffer&>(frameBuffer));
		glClear(flags);
	}

	void OGLRenderer::PrepareDraw(const VertexObject& vertexObject, FrameBuffer& frameBuffer, bool doDepthTest)
	{
		GM_CORE_ASSERT(m_ViewportDimension.x == 0 || m_ViewportDimension.y == 0, "Invalid viewport size (0x0 pixels).");

		OGLContext::BindFrameBuffer(static_cast<const OGLFrameBuffer&>(frameBuffer));
		OGLContext::BindVertexArrayObject(static_cast<const OGLVertexObject&>(vertexObject));

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