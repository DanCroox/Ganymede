#include "GraphicsFactory.h"

#include "OpenGL/OGLFrameBuffer.h"
#include "OpenGL/OGLSSBO.h"
#include "OpenGL/OGLVertexObject.h"

namespace Ganymede
{
	namespace GraphicsFactory
	{
		GANYMEDE_API std::unique_ptr<FrameBuffer> CreateFrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer)
		{
			return std::make_unique<OGLFrameBuffer>(renderDimension, isHardWareFrameBuffer);
		}
		
		GANYMEDE_API std::unique_ptr<SSBO> CreateSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize)
		{
			return std::make_unique<OGLSSBO>(bindingPointID, bufferSize, autoResize);
		}

		GANYMEDE_API std::unique_ptr<VertexObject> CreateVertexObject(const unsigned int* indicesData, unsigned int numIndices)
		{
			return std::make_unique<OGLVertexObject>(indicesData, numIndices);
		}
	}
}