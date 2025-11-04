#pragma once

#include "Ganymede/Core/Core.h"
#include "glm/glm.hpp"
#include <memory>

namespace Ganymede
{
	class FrameBuffer;
	class GPUDebugHandler;
	class GPUTexture;
	class RenderContext;
	class Renderer;
	class RenderTarget;
	class Shader;
	class ShaderBinary;
	class SSBO;
	class Texture;
	class VertexObject;

	namespace RenderTargetTypes
	{
		enum class ComponentType;
		enum class ChannelPrecision;
		enum class ChannelDataType;
	}

	namespace GraphicsFactory
	{
#ifndef GM_RETAIL
		GANYMEDE_API std::unique_ptr<GPUDebugHandler> CreateGPUDebugHandler();
#endif //GM_RETAIL

		GANYMEDE_API std::unique_ptr<FrameBuffer> CreateFrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer);
		GANYMEDE_API std::unique_ptr<SSBO> CreateSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		GANYMEDE_API std::unique_ptr<VertexObject> CreateVertexObject(const unsigned int* indicesData, unsigned int numIndices);
		GANYMEDE_API std::unique_ptr<GPUTexture> CreateGPUTexture(const Texture& texture);
		GANYMEDE_API std::unique_ptr<Shader> CreateShader(const ShaderBinary& shaderBinary);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateSingleSampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateMultiSampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateCubeMapArrayRenderTarget(unsigned int textureCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<Renderer> CreateRenderer(RenderContext& renderContext);
	}
}