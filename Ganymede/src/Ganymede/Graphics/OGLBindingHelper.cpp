#include "OGLBindingHelper.h"

#include "FrameBuffer.h"
#include "gl/glew.h"

namespace Ganymede
{
	unsigned int OGLBindingHelper::m_BoundFrameBuffer = 0;
	unsigned int OGLBindingHelper::m_BoundShader = 0;
	unsigned int OGLBindingHelper::m_BoundVertexArrayObject = 0;
	glm::u32vec2 OGLBindingHelper::m_CurrentViewportDimension = {0, 0};
	
	void OGLBindingHelper::BindFrameBuffer(const FrameBuffer& frameBuffer)
	{
		const unsigned int renderID = frameBuffer.GetRenderID();
		if (renderID == m_BoundFrameBuffer)
		{
			return;
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, renderID);

		const glm::u32vec2 framebufferViewportDimension= frameBuffer.GetRenderDimension();
		if (m_CurrentViewportDimension != framebufferViewportDimension)
		{
			glViewport(0,0, framebufferViewportDimension.x, framebufferViewportDimension.y);
			m_CurrentViewportDimension = framebufferViewportDimension;
		}
		m_BoundFrameBuffer = renderID;
	}

	void OGLBindingHelper::UnbindFrameBuffer()
	{
		if (m_BoundFrameBuffer == 0)
		{
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_BoundFrameBuffer = 0;
	}

	void OGLBindingHelper::BindShader(unsigned int renderID)
	{
		if (renderID == m_BoundShader)
		{
			return;
		}

		glUseProgram(renderID);
		m_BoundShader = renderID;
	}

	void OGLBindingHelper::BindVertexArrayObject(unsigned int renderID)
	{
		if (renderID == m_BoundVertexArrayObject)
		{
			return;
		}

		glBindVertexArray(renderID);
		m_BoundVertexArrayObject = renderID;
	}
}