#pragma once

#include "Ganymede/Core/Core.h"

#include "FrameBuffer.h"
#include "Renderer2.h"
#include "DataBuffer.h"
#include "RenderTarget.h"
#include "VertexDataTypes.h"
#include "Shader.h"
#include "SSBO.h"
#include <cstdarg>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "VertexObject.h"


namespace Ganymede
{
	class World;
	class FPSCamera;

	class GANYMEDE_API RenderContext
	{
	public:
		RenderContext(const RenderContext&) = delete;
		RenderContext& operator=(const RenderContext&) = delete;
		RenderContext() = delete;

		RenderContext(const World& world, const FPSCamera& camera);
		virtual ~RenderContext() = default;

		const World& GetWorld() const;
		const FPSCamera& GetCamera() const;

		Renderer2& GetRenderer() { return m_Renderer; };

		FrameBuffer* CreateFrameBuffer(const std::string& name, glm::u32vec2 renderDimension, bool isHardwareBuffer);
		SinglesampleRenderTarget* CreateSingleSampleRenderTarget(const std::string& name, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		MultisampleRenderTarget* CreateMultiSampleRenderTarget(const std::string& name, unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		CubeMapArrayRenderTarget* CreateCubeMapArrayRenderTarget(const std::string& name, unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		VertexObject* CreateVertexObject(const std::string& name, const unsigned int* indicesData, unsigned int numIndices);
		SSBO* CreateSSBO(const std::string& name, unsigned int bindingID, unsigned int numBytes);
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

	private:
		const World& m_World;
		const FPSCamera& m_Camera;

		Renderer2 m_Renderer;

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