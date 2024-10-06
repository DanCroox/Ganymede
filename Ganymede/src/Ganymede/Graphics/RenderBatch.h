#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "RendererTypes.h"

namespace Ganymede
{
	class SSBO;

	struct GANYMEDE_API PerInstanceData
	{
		float m_LightIndex;
		float m_LayerID; // For shadow mapping
		float m_AnimationDataOffset = 0; // Bone transform data offset
	};

	struct GANYMEDE_API IData
	{
		glm::mat4 mv;
		glm::mat4 instance;
		PerInstanceData pid;
	};

	struct GANYMEDE_API DrawElementsIndirectCommand {
		unsigned int count;
		unsigned int  instanceCount;
		unsigned int  firstIndex;
		unsigned int  baseVertex;
		unsigned int  baseInstance;
	};

	enum class GANYMEDE_API AddToBatchResult
	{
		TooBigForBatch,
		NotFitIntoBatch,
		MaterialChanged,
		Success
	};

	class GANYMEDE_API RenderBatch
	{
	public:
		RenderBatch();
		~RenderBatch();

		AddToBatchResult TryAddToBatch(const MeshWorldObject::Mesh& mesh, const std::vector<MeshInstancess>& instances, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, RenderPass renderPass, const std::vector<PointLight>* pointlightsToTest = nullptr);
		void Flush(RenderPass renderPass);

	private:
		inline void* GetVRAMMemoryPointer(int glBufferType, unsigned int byteOffset, unsigned int numBytes);

		std::vector<IData> gpuData;

		SSBO* m_AnimationDataUBO;

		const Material* m_BoundMaterial = nullptr;
		unsigned int m_DrawCommandFirstIndex = 0;
		unsigned int m_DrawCommandBaseIndex = 0;
		unsigned int m_VertexBufferDataOffset = 0;
		unsigned int m_IndexBufferDataOffset = 0;
		unsigned int m_DrawIndirectBufferDataOffset = 0;
		unsigned int m_TransformsBufferDataOffset = 0;
		unsigned int m_DrawCommandBaseInstance = 0;
		unsigned int m_NumDrawCommands = 0;
		unsigned int m_AnimationDataOffset = 0;

		unsigned int m_VertexArrayID;
		unsigned int m_ElementBufferID;
		unsigned int m_DrawIndirectBufferID;
		unsigned int m_VertexBufferID;
		unsigned int m_TransformsBufferID;

		const unsigned int m_MaxVerticesPerBatch = 100000;
		const unsigned int m_MaxIndicesPerBatch = 3 * 100000; // 3 = Triangle has 3 vertices. 10000000 = Max triangle count
		const unsigned int m_MaxTransformsPerBatch = (m_MaxIndicesPerBatch / 3) * 2; // Smalles geometry is a triangle. We provide space for the max number of geometry objects we could draw (we have 2 matrices per mesh instance object)
		const unsigned int m_MaxDrawCommandsPerBatch = 1000000;

		const unsigned int m_BatchVertexBufferSize = m_MaxVerticesPerBatch * sizeof(MeshWorldObject::Mesh::Vertex);
		const unsigned int m_BatchElementsBufferSize = m_MaxIndicesPerBatch * sizeof(unsigned int);
		const unsigned int m_BatchTransformsBufferSize = m_MaxTransformsPerBatch * sizeof(glm::mat4);
		const unsigned int m_BatchDrawCommandsBufferSize = m_MaxDrawCommandsPerBatch * sizeof(DrawElementsIndirectCommand);
		const unsigned int m_BatchAnimationDataBufferSize = 45000 * sizeof(glm::mat4);

		void* m_BatchAnimationDataBufferInter;

		void* m_BatchVertexBufferPtr = nullptr;
		void* m_BatchTransformsBufferPtr = nullptr;
		void* m_BatchElementsBufferPtr = nullptr;
		void* m_BatchDrawCommandsBufferPtr = nullptr;

		bool m_UpdateVRAMBuffers = true;
	};
}