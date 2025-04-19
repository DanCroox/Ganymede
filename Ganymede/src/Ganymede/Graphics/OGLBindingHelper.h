#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class FrameBuffer;

	class GANYMEDE_API OGLBindingHelper
	{
	public:
		enum class FrameBufferTarget
		{
			Read,
			Draw
		};

		static void BindFrameBuffer(const FrameBuffer& frameBuffer);
		static void UnbindFrameBuffer();
		static void BindShader(unsigned int renderID);
		static void BindVertexArrayObject(unsigned int renderID);

	private:
		static unsigned int m_BoundFrameBuffer;
		static unsigned int m_BoundShader;
		static unsigned int m_BoundVertexArrayObject;
		static glm::u32vec2 m_CurrentViewportDimension;
	};
}