#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"
#include <memory>

namespace Ganymede
{
	class FrameBuffer;
	class SSBO;
	class VertexObject;

	namespace GraphicsFactory
	{
		GANYMEDE_API std::unique_ptr<FrameBuffer> CreateFrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer);
		GANYMEDE_API std::unique_ptr<SSBO> CreateSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		GANYMEDE_API std::unique_ptr<VertexObject> CreateVertexObject(const unsigned int* indicesData, unsigned int numIndices);
	}
}