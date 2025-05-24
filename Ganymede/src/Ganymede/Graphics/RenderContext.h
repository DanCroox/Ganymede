#pragma once

#include "Ganymede/Core/Core.h"

#include "DataBuffer.h"
#include "FrameBuffer.h"
#include "GPUResourceSystem.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "SSBO.h"
#include "VertexDataTypes.h"
#include "VertexObject.h"
#include <cstdarg>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace Ganymede
{
	class World;
	class FPSCamera;

	struct GANYMEDE_API RenderCommand
	{
		VertexObject* m_VO = nullptr;
		Material* m_Material = nullptr;
		std::uint32_t m_SSBOInstanceID = 0;
	};

	using RenderCommandQueue = std::vector<RenderCommand>;

	class GANYMEDE_API RenderContext
	{
	public:
		template <typename T>
		struct RenderData
		{
			VertexObject m_VO;
			Material* m_Material;
			std::vector<T> m_Instances;
		};

		RenderContext(const RenderContext&) = delete;
		RenderContext& operator=(const RenderContext&) = delete;
		RenderContext() = delete;

		RenderContext(World& world, const FPSCamera& camera);
		virtual ~RenderContext() = default;

		World& GetWorld();
		const FPSCamera& GetCamera() const;

		Renderer& GetRenderer() { return m_Renderer; };

		FrameBuffer* CreateFrameBuffer(const std::string& name, glm::u32vec2 renderDimension, bool isHardwareBuffer);
		SinglesampleRenderTarget* CreateSingleSampleRenderTarget(const std::string& name, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		MultisampleRenderTarget* CreateMultiSampleRenderTarget(const std::string& name, unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		CubeMapArrayRenderTarget* CreateCubeMapArrayRenderTarget(const std::string& name, unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		VertexObject* CreateVertexObject(const std::string& name, const unsigned int* indicesData, unsigned int numIndices);
		SSBO* CreateSSBO(const std::string& name, unsigned int bindingID, unsigned int numBytes, bool autoResize);
		Shader* LoadShader(const std::string& name, const std::string& path);
		
		template <typename T>
		DataBuffer<T>* CreateDataBuffer(const std::string& name, T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType)
		{
			auto it = m_DataBuffers.find(name);
			if (it != m_DataBuffers.end())
			{
				GM_CORE_ASSERT(false, "Tried to create DataBuffer which already existed. Using from cache.");
				const std::pair<ClassID, void*>& pair = it->second;
				if (T::GetStaticClassID() != pair.first)
				{
					GM_CORE_ASSERT(false, "Template type does not match stored object type.");
					return nullptr;
				}

				return (DataBuffer<T>*) pair.second;
			}

			DataBuffer<T>* instanceDataBuffer = new DataBuffer<T>(data, numElements, bufferType);
			m_DataBuffers[name] = std::make_pair(T::GetStaticClassID(), (void*)instanceDataBuffer);
			return instanceDataBuffer;
		}

		FrameBuffer* GetFrameBuffer(const std::string& name);
		SinglesampleRenderTarget* GetSingleSampleRenderTarget(const std::string& name);
		MultisampleRenderTarget* GetMultiSampleRenderTarget(const std::string& name);
		CubeMapArrayRenderTarget* GetCubeMapArrayRenderTarget(const std::string& name);
		VertexObject* GetVertexObject(const std::string& name);
		SSBO* GetSSBO(const std::string& name);
		Shader* GetShader(const std::string& name);

		template <typename T>
		DataBuffer<T>* GetDataBuffer(const std::string& name)
		{
			auto it = m_DataBuffers.find(name);
			if (it == m_DataBuffers.end())
			{
				GM_CORE_ASSERT(false, "DataBuffer does not exist.");
				return nullptr;
			}
			
			const std::pair<ClassID, void*>& pair = it->second;

			if (T::GetStaticClassID() != pair.first)
			{
				GM_CORE_ASSERT(false, "Template type does not match stored object type.");
				return nullptr;
			}

			return (DataBuffer<T>*) it->second.second;
		}

		void DeleteFrameBuffer(const std::string& name);
		void DeleteSingleSampleRenderTarget(const std::string& name);
		void DeleteMultiSampleRenderTarget(const std::string& name);
		void DeleteCubeMapArrayRenderTarget(const std::string& name);
		void DeleteVertexObject(const std::string& name);
		void UnloadShader(const std::string& name);
		void DeleteDataBuffer(const std::string& name);

		std::vector<std::optional<VertexObject>> m_VertexObjectStorage;
		RenderCommandQueue m_CubemapShadowMappingCommandQueue;
		std::vector<std::int32_t> m_InstanceIDToGBufferInstanceDataIndexLookup;
		std::vector<std::int32_t> m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup;
		std::int32_t m_NextFreeCubemapSSBOInstanceDataIndex = 0;

		GPUResourceSystem m_GpuResources;
	private:
		World& m_World;
		const FPSCamera& m_Camera;

		Renderer m_Renderer;

		std::unordered_map<std::string, FrameBuffer> m_FrameBuffers;
		std::unordered_map<std::string, SinglesampleRenderTarget> m_SingleSampleRenderTargets;
		std::unordered_map<std::string, MultisampleRenderTarget> m_MultiSampleRenderTargets;
		std::unordered_map<std::string, CubeMapArrayRenderTarget> m_CubeMapArrayRenderTargets;
		std::unordered_map<std::string, VertexObject> m_VertexObjects;
		std::unordered_map<std::string, SSBO> m_SSBOs;
		std::unordered_map<std::string, Shader> m_Shaders;
		std::unordered_map<std::string, std::pair<ClassID, void*>> m_DataBuffers;
	};
}