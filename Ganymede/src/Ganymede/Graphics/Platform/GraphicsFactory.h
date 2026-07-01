#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/DataBuffer.h"
#include "glm/glm.hpp"
#include <memory>
#include <optional>

namespace Ganymede
{
	class CommandList;
	class ComputeShader;
	class FrameBuffer;
	class FrameBufferAttachmentStorage;
	class GPUDebugHandler;
	class GPUTexture;
	class GraphicsShader;
	class Material;
	class Pipeline;
	class RenderContext;
	class Renderer;
	class RenderPipeline;
	class RenderTarget;
	class ShaderBinary;
	class SSBO;
	class Texture;
	class VertexObject;

	struct FrameBufferAttachment;

	namespace RenderTargetTypes
	{
		enum class ComponentType;
		enum class ChannelPrecision;
		enum class ChannelDataType;
	}

	namespace GraphicsFactory
	{
	#define DECLARE_CREATEDATABUFFER_FUNC_HEADER(TYPE)					\
    template<>															\
    GANYMEDE_API std::unique_ptr<DataBuffer<TYPE>> CreateDataBuffer(	\
        typename TYPE::VertexDataType* data,							\
        unsigned int numElements,										\
        DataBufferType bufferType)


#ifndef GM_RETAIL
		GANYMEDE_API std::unique_ptr<GPUDebugHandler> CreateGPUDebugHandler();
#endif //GM_RETAIL

		GANYMEDE_API std::unique_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension);
		GANYMEDE_API std::unique_ptr<SSBO> CreateSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		GANYMEDE_API std::unique_ptr<VertexObject> CreateVertexObject(const unsigned int* indicesData, unsigned int numIndices);
		GANYMEDE_API std::unique_ptr<GPUTexture> CreateGPUTexture(const Texture& texture);
		GANYMEDE_API std::unique_ptr<GraphicsShader> CreateGraphicsShader(const ShaderBinary& shaderBinary);
		GANYMEDE_API std::unique_ptr<ComputeShader> CreateComputeShader(const ShaderBinary& shaderBinary);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateSingleSampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateMultiSampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<RenderTarget> CreateCubeMapArrayRenderTarget(unsigned int textureCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		GANYMEDE_API std::unique_ptr<Renderer> CreateRenderer(RenderContext& renderContext);
		GANYMEDE_API std::unique_ptr<Pipeline> CreatePipeline(const ShaderBinary& shaderBinary,
			uint32_t vertexInputDataStride,
			const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints);
		GANYMEDE_API std::unique_ptr<Pipeline> CreatePipeline(const ShaderBinary& shaderBinary,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints);

		GANYMEDE_API std::optional<ShaderBinary> LoadShader(const std::string& filePath);
		GANYMEDE_API CommandList* GetCommandList();

		template<typename T>
		std::unique_ptr<DataBuffer<T>> CreateDataBuffer(typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType)
		{
			static_assert(sizeof(T) == 0, "Unsupported DataBuffer type!");
			return nullptr;
		}

		#define X(type, basetype, ...) DECLARE_CREATEDATABUFFER_FUNC_HEADER(type);
		#include "Ganymede/Graphics/VertexDataTypes.def"
		#undef X
	}
}