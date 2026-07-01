#include "GraphicsFactory.h"

#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/VertexDataTypes.h"

#include "OpenGL/OGLComputeShader.h"
#include "OpenGL/OGLDataBuffer.h"
#include "OpenGL/OGLFrameBuffer.h"
#include "OpenGL/OGLGPUDebugHandler.h"
#include "OpenGL/OGLGPUTexture.h"
#include "OpenGL/OGLGraphicsShader.h"
#include "OpenGL/OGLRenderer.h"
#include "OpenGL/OGLRenderTarget.h"
#include "OpenGL/OGLShaderLoader.h"
#include "OpenGL/OGLSSBO.h"
#include "OpenGL/OGLVertexObject.h"

#include "Vulkan/VKCommandList.h"
#include "Vulkan/VKContext.h"
#include "Vulkan/VKDataBuffer.h"
#include "Vulkan/VKFrameBuffer.h"
#include "Vulkan/VKGPUDebugHandler.h"
#include "Vulkan/VKGPUTexture.h"
#include "Vulkan/VKPipeline.h"
#include "Vulkan/VKRenderTarget.h"
#include "Vulkan/VKShaderLoader.h"
#include "Vulkan/VKSSBO.h"
#include "Vulkan/VKVertexObject.h"

namespace Ganymede
{
	namespace GraphicsFactory
	{
#ifndef GM_RETAIL
		GANYMEDE_API std::unique_ptr<GPUDebugHandler> CreateGPUDebugHandler()
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLGPUDebugHandler>();
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKGPUDebugHandler>();
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}
#endif //GM_RETAIL

		GANYMEDE_API std::unique_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLFrameBuffer>(attachments, renderDimension);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKFrameBuffer>(attachments, renderDimension);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}
		
		GANYMEDE_API std::unique_ptr<SSBO> CreateSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLSSBO>(bindingPointID, bufferSize, autoResize);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKSSBO>(bindingPointID, bufferSize, autoResize);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::unique_ptr<VertexObject> CreateVertexObject(const unsigned int* indicesData, unsigned int numIndices)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLVertexObject>(indicesData, numIndices);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKVertexObject>(indicesData, numIndices);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::unique_ptr<GPUTexture> CreateGPUTexture(const Texture& texture)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLGPUTexture>(texture);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKGPUTexture>(texture);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::unique_ptr<GraphicsShader> CreateGraphicsShader(const ShaderBinary& shaderBinary)
		{
			return std::make_unique<OGLGraphicsShader>(shaderBinary);
		}

		GANYMEDE_API std::unique_ptr<ComputeShader> CreateComputeShader(const ShaderBinary& shaderBinary)
		{
			return std::make_unique<OGLComputeShader>(shaderBinary);
		}

		GANYMEDE_API std::unique_ptr<RenderTarget> CreateSingleSampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLSinglesampleRenderTarget>(componentType, dataType, precision, size);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKSinglesampleRenderTarget>(componentType, dataType, precision, size);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::unique_ptr<RenderTarget> CreateMultiSampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
		{
			return std::make_unique<OGLMultisampleRenderTarget>(sampleCount, componentType, dataType, precision, size);
		}

		GANYMEDE_API std::unique_ptr<RenderTarget> CreateCubeMapArrayRenderTarget(unsigned int textureCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size)
		{
			return std::make_unique<OGLCubeMapArrayRenderTarget>(textureCount, componentType, dataType, precision, size);
		}

		GANYMEDE_API std::unique_ptr<Renderer> CreateRenderer(RenderContext& renderContext)
		{
			return std::make_unique<OGLRenderer>(renderContext);
		}

		GANYMEDE_API std::unique_ptr<Pipeline> CreatePipeline(
			const ShaderBinary& shaderBinary,
			uint32_t vertexInputDataStride,
			const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKPipeline>(shaderBinary, vertexInputDataStride, vertexDataPrimitiveTypeInfos, static_cast<const VKFrameBuffer&>(frameBuffer), ssboBindingPoints);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::unique_ptr<Pipeline> CreatePipeline(
			const ShaderBinary& shaderBinary,
			const FrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKPipeline>(shaderBinary, static_cast<const VKFrameBuffer&>(frameBuffer), ssboBindingPoints);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API std::optional<ShaderBinary> LoadShader(const std::string& filePath)
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return OGLShaderLoader::Load(filePath);
			case GraphicsBackend::Vulkan:
				return VKShaderLoader::Load(filePath);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		GANYMEDE_API CommandList* GetCommandList()
		{
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::Vulkan:
				return VKContext::GetInstance().m_CommandList.get();
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}

		#define DECLARE_CREATEDATABUFFER_FUNC(TYPE)												\
		DECLARE_CREATEDATABUFFER_FUNC_HEADER(TYPE)												\
		{																						\
			switch (GM_ActiveBackend)															\
			{																					\
			case GraphicsBackend::OpenGL:														\
				return std::make_unique<OGLDataBuffer<TYPE>>(data, numElements, bufferType);	\
			case GraphicsBackend::Vulkan:														\
				return std::make_unique<VKDataBuffer<TYPE>>(data, numElements, bufferType);		\
			default:																			\
				GM_CORE_ASSERT(false, "Not supported");											\
				return {};																		\
			}																					\
		}
		#define X(type, basetype, ...) DECLARE_CREATEDATABUFFER_FUNC(type);
		#include "Ganymede/Graphics/VertexDataTypes.def"
		#undef X
	}
}