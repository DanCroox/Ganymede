#pragma once
#include "Ganymede/Core/Core.h"

#include "DataBuffer.h"
#include "VertexDataTypes.h"

#include <memory>
#include <vector>

namespace Ganymede
{
	class GANYMEDE_API FreeeList
	{
	public:
		size_t Append()
		{
			if (!m_NextFreeIndices.empty())
			{
				const size_t index = m_NextFreeIndices.back();
				m_NextFreeIndices.pop_back();
				return index;
			}
			
			return m_HighestIndex++;
		}

		void Free(size_t index)
		{
			GM_CORE_ASSERT(index <= m_HighestIndex, "FreeList does not contain given index.");
			m_NextFreeIndices.push_back(index);
		}

	private:
		std::vector<size_t> m_NextFreeIndices;
		size_t m_HighestIndex = 0;
	};

	class MeshWorldObject::Mesh;
	class SSBO;
	class VertexObject;
	class World;

	class GANYMEDE_API GPUResourceSystem
	{
	public:
		GPUResourceSystem(World& world);

		void UpdateGPUResources();

		std::shared_ptr<VertexObject> GetVO(MeshWorldObject::Mesh& mesh);
		DataBuffer<UInt32VertexData>& GetInstanceDataIndexBuffer() { return m_InstanceDataIndexBuffer; }

	private:
		void RemoveMeshDataFromGPU();
		void LoadPendingMeshDataToGPU();
		void UpdateInstanceDataOnGPU();
		void UpdateAnimationDataOnGPU();

		World& m_World;
		FreeeList m_GBufferInstanceDataFreeList;
		std::unique_ptr<SSBO> m_GBufferInstanceDataSSBO;
		std::unique_ptr<SSBO> m_AnimationDataSSBO;
		DataBuffer<UInt32VertexData> m_InstanceDataIndexBuffer;
		std::vector<std::weak_ptr<VertexObject>> m_VertexObjects;
	};
}