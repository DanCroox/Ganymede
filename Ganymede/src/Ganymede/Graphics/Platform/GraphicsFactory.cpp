#include "GraphicsFactory.h"

#include "Ganymede/Graphics/ShaderBinary.h"

#include "OpenGL/OGLFrameBuffer.h"
#include "OpenGL/OGLGPUDebugHandler.h"
#include "OpenGL/OGLGPUTexture.h"
#include "OpenGL/OGLShader.h"
#include "OpenGL/OGLSSBO.h"
#include "OpenGL/OGLVertexObject.h"

namespace Ganymede
{
	namespace GraphicsFactory
	{
#ifndef GM_RETAIL
		GANYMEDE_API std::unique_ptr<GPUDebugHandler> CreateGPUDebugHandler()
		{
			return std::make_unique<OGLGPUDebugHandler>();
		}
#endif //GM_RETAIL

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

		GANYMEDE_API std::unique_ptr<GPUTexture> CreateGPUTexture(const Texture& texture)
		{
			return std::make_unique<OGLGPUTexture>(texture);
		}

		GANYMEDE_API std::unique_ptr<Shader> CreateShader(const ShaderBinary& shaderBinary)
		{
			return std::make_unique<OGLShader>(shaderBinary);
		}
	}
}