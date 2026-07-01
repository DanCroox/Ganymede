#pragma once

#include "Ganymede/Core/Core.h"

#include "ComputeShader.h"
#include "DataBuffer.h"
#include "FrameBuffer.h"
#include "Ganymede/System/FreeList.h"
#include "GPUTexture.h"
#include "GraphicsShader.h"
#include "Platform/GraphicsFactory.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "RenderView.h"
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
	class MeshWorldObject::Mesh;
	class World;

	struct VisibleEntity
	{
		uint32_t m_EntityID; // entt ID
		glm::uint m_GPUBufferDataIndex;
	};

	struct RenderMeshInstanceCommand
	{
		glm::uint m_MeshID;
		glm::uint m_ViewID;
		glm::uint m_IndirectCommandIndex;
		glm::uint m_RenderViewGroup;
	};

	struct RenderMeshInstanceCommandOffsetsByView
	{
		glm::uint m_StartIndex;
		glm::uint m_LastIndex;
	};

	struct alignas(16) EntityData
	{
		glm::mat4 m_Transform;
		std::array<glm::vec4, 8> m_AABB;
		glm::uint m_MeshID;
		glm::uint m_NumMeshIndices;
		glm::uint m_AnimationDataOffset;
		glm::uint m_EntityID;
	};

	struct RenderCommand
	{
		VertexObject* m_VO = nullptr;
		Material* m_Material = nullptr;
		std::uint32_t m_SSBOInstanceID = 0;
	};

	using RenderCommandQueue = std::vector<RenderCommand>;

	class GANYMEDE_API RenderContext
	{
	public:
		struct CachedVertexObject
		{
			float m_LastAccessTime;
			std::unique_ptr<VertexObject> m_VertexObject;
		};

		struct CachedGPUTextureObject
		{
			float m_LastAccessTime;
			std::unique_ptr<GPUTexture> m_GPUTexture;
		};

		RenderContext(const RenderContext&) = delete;
		RenderContext& operator=(const RenderContext&) = delete;
		RenderContext() = delete;

		RenderContext(World& world);
		virtual ~RenderContext() = default;

		World& GetWorld();

		Renderer& GetRenderer();

		FrameBuffer* CreateFrameBuffer(const std::string& name, const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension);
		RenderTarget* CreateSingleSampleRenderTarget(const std::string& name, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		RenderTarget* CreateMultiSampleRenderTarget(const std::string& name, unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		RenderTarget* CreateCubeMapArrayRenderTarget(const std::string& name, unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size);
		VertexObject* CreateVertexObject(const std::string& name, const unsigned int* indicesData, unsigned int numIndices);
		SSBO* CreateSSBO(const std::string& name, unsigned int bindingID, unsigned int numBytes, bool autoResize);
		GraphicsShader* LoadGraphicsShader(const std::string& name, const std::string& shaderFile);
		ComputeShader* LoadComputeShader(const std::string& name, const std::string& shaderFile);
		
		template <typename T>
		DataBuffer<T>* CreateDataBuffer(const std::string& name, typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType)
		{
			auto it = m_DataBuffers.find(name);
			if (it != m_DataBuffers.end())
			{
				GM_CORE_ASSERT(false, "Tried to create DataBuffer which already existed. Using from cache.");
				const std::pair<ClassID, std::unique_ptr<DataBufferBase>>& pair = it->second;
				if (T::GetStaticClassID() != pair.first)
				{
					GM_CORE_ASSERT(false, "Template type does not match stored object type.");
					return nullptr;
				}
				
				return static_cast<DataBuffer<T>*>(pair.second.get());
			}

			auto [emplacedBufferIt, inserted] = m_DataBuffers.emplace(name, std::make_pair(T::GetStaticClassID(), GraphicsFactory::CreateDataBuffer<T>(data, numElements, bufferType)));
			GM_CORE_ASSERT(inserted, "Failed to emplace DataBuffer.");

			return static_cast<DataBuffer<T>*>(emplacedBufferIt->second.second.get());
		}

		void BindMaterial(const Material& material);

		FrameBuffer* GetFrameBuffer(const std::string& name);
		RenderTarget* GetSingleSampleRenderTarget(const std::string& name);
		RenderTarget* GetMultiSampleRenderTarget(const std::string& name);
		RenderTarget* GetCubeMapArrayRenderTarget(const std::string& name);
		VertexObject* GetVertexObject(const std::string& name);
		SSBO* GetSSBO(const std::string& name);
		GraphicsShader* GetGraphicsShader(const std::string& name);
		ComputeShader* GetComputeShader(const std::string& name);

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

			return static_cast<DataBuffer<T>*>(it->second.second);
		}

		void DeleteFrameBuffer(const std::string& name);
		void DeleteSingleSampleRenderTarget(const std::string& name);
		void DeleteMultiSampleRenderTarget(const std::string& name);
		void DeleteCubeMapArrayRenderTarget(const std::string& name);
		void DeleteVertexObject(const std::string& name);
		void UnloadGraphicsShader(const std::string& name);
		void UnloadComputeShader(const std::string& name);
		void DeleteDataBuffer(const std::string& name);

		RenderCommandQueue m_CubemapShadowMappingCommandQueue;
		std::vector<std::int32_t> m_InstanceIDToGBufferInstanceDataIndexLookup;
		std::vector<std::int32_t> m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup;
		std::int32_t m_NextFreeCubemapSSBOInstanceDataIndex = 0;

		//TODO proper viewid and groupid handling!
		RenderView& CreateRenderView(glm::u32vec2 renderResolution, float fov, float nearClip, float farClip, unsigned int viewGroupID);
		void DestroyRenderView(RenderView& renderView);
		RenderView& GetRenderView(unsigned int viewID) { return m_RenderViews[viewID].value(); }
		unsigned int GetNumRenderViews() const { return numRenderView; }
		unsigned int numRenderView = 0;

		const VertexObject& GetVO(MeshWorldObject::Mesh& mesh);
		const GPUTexture& GetGPUTexture(const Handle<Texture> handle);

		std::vector<RenderMeshInstanceCommand> m_RenderInfo;
		std::vector<RenderMeshInstanceCommandOffsetsByView> m_RenderInfoOffsets;
		std::vector<MeshWorldObject::Mesh*> m_MeshIDMapping;

		std::vector<VisibleEntity>& GetVisibleEntities() { return m_VisibleEntities; }

	private:
		World& m_World;

		std::unique_ptr<Renderer> m_Renderer;

		std::unordered_map<std::string, std::unique_ptr<FrameBuffer>> m_FrameBuffers;
		std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_SingleSampleRenderTargets;
		std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_MultiSampleRenderTargets;
		std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_CubeMapArrayRenderTargets;
		std::unordered_map<std::string, std::unique_ptr<VertexObject>> m_VertexObjects;
		std::unordered_map<std::string, std::unique_ptr<SSBO>> m_SSBOs;
		std::unordered_map<std::string, std::unique_ptr<GraphicsShader>> m_GraphicsShaders;
		std::unordered_map<std::string, std::unique_ptr<ComputeShader>> m_ComputeShaders;
		std::unordered_map<std::string, std::pair<ClassID, std::unique_ptr<DataBufferBase>>> m_DataBuffers;
		
		std::vector<CachedVertexObject> m_VertexObjectCache;
		std::vector<CachedGPUTextureObject> m_TextureObjectCache;

		std::vector<VisibleEntity> m_VisibleEntities;

		std::vector<std::optional<RenderView>> m_RenderViews;
		FreeList m_RenderViewFreeList;
	};
}