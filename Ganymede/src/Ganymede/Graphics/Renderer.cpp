#include "Renderer.h"

#include "gl/glew.h"

#include "FrameBuffer.h"
#include "OGLBindingHelper.h"
#include "Shader.h"
#include "VertexObject.h"

namespace Ganymede
{
	Renderer::Renderer() :
		m_ViewportDimension({0, 0}), m_DoDepthTesting(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE)
	{}

	void Renderer::DrawVertexObject(VertexObject& vertexObject, unsigned int numInstances, FrameBuffer& frameBuffer, Shader& shader, bool doDepthTest)
	{
		GM_CORE_ASSERT(shader.GetRendererID() != 0 , "Shader invalid.");
		GM_CORE_ASSERT(m_ViewportDimension.x == 0 || m_ViewportDimension.y == 0, "Invalid viewport size (0x0 pixels).");

		OGLBindingHelper::BindFrameBuffer(frameBuffer);
		OGLBindingHelper::BindShader(shader.GetRendererID());
		OGLBindingHelper::BindVertexArrayObject(vertexObject.GetRenderID());

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

		glDrawElementsInstanced(GL_TRIANGLES, vertexObject.GetVertexObjectIndexBuffer().GetNumIndices(), GL_UNSIGNED_INT, 0, numInstances);
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

		OGLBindingHelper::BindFrameBuffer(frameBuffer);
		glClear(flags);
	}
}