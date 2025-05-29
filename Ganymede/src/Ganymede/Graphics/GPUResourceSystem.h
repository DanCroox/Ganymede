#pragma once
#include "Ganymede/Core/Core.h"

#include "DataBuffer.h"
#include "Ganymede/System/FreeList.h"
#include "VertexDataTypes.h"

#include <memory>
#include <vector>

namespace Ganymede
{
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
		FreeList m_GBufferInstanceDataFreeList;
		std::unique_ptr<SSBO> m_GBufferInstanceDataSSBO;
		std::unique_ptr<SSBO> m_AnimationDataSSBO;
		DataBuffer<UInt32VertexData> m_InstanceDataIndexBuffer;
		std::vector<std::weak_ptr<VertexObject>> m_VertexObjects;
	};
}