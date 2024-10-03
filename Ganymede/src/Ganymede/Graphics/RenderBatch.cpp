#include "RenderBatch.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Renderer.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include "SSBO.h"
#include "Ganymede/System/Types.h"
#include "Ganymede/Runtime/GMTime.h"
#include "GL/glew.h"

namespace RenderBatch_Private
{
	static unsigned int locStrideVertex = sizeof(MeshWorldObject::Mesh::Vertex);
}

RenderBatch::RenderBatch()
{
	using namespace RenderBatch_Private;

	//m_AnimationDataUBO = new SSBO(2, m_BatchAnimationDataBufferSize);
	m_BatchAnimationDataBufferInter = malloc(m_BatchAnimationDataBufferSize);

	GLCall(glGenVertexArrays(1, &m_VertexArrayID));
	GLCall(glBindVertexArray(m_VertexArrayID));

	GLCall(glGenBuffers(1, &m_VertexBufferID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, m_BatchVertexBufferSize, nullptr, GL_STREAM_DRAW));

	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Position)));

	GLCall(glEnableVertexAttribArray(1));
	GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_UV)));

	GLCall(glEnableVertexAttribArray(2));
	GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Normal)));

	GLCall(glEnableVertexAttribArray(3));
	GLCall(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Tangent)));

	GLCall(glEnableVertexAttribArray(4));
	GLCall(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_Bitangent)));

	GLCall(glEnableVertexAttribArray(5));
	GLCall(glVertexAttribIPointer(5, 4, GL_INT, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_BoneIndex)));

	GLCall(glEnableVertexAttribArray(6));
	GLCall(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, locStrideVertex, (const void*)offsetof(MeshWorldObject::Mesh::Vertex, m_BoneWeight)));

	GLCall(glGenBuffers(1, &m_TransformsBufferID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_TransformsBufferID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, m_BatchTransformsBufferSize, nullptr, GL_STREAM_DRAW));

	GLCall(glEnableVertexAttribArray(7));
	GLCall(glEnableVertexAttribArray(8));
	GLCall(glEnableVertexAttribArray(9));
	GLCall(glEnableVertexAttribArray(10));
	GLCall(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)0));
	GLCall(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)16));
	GLCall(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)32));
	GLCall(glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)48));
	GLCall(glVertexAttribDivisor(7, 1));
	GLCall(glVertexAttribDivisor(8, 1));
	GLCall(glVertexAttribDivisor(9, 1));
	GLCall(glVertexAttribDivisor(10, 1));

	GLCall(glEnableVertexAttribArray(11));
	GLCall(glEnableVertexAttribArray(12));
	GLCall(glEnableVertexAttribArray(13));
	GLCall(glEnableVertexAttribArray(14));
	GLCall(glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)64));
	GLCall(glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)80));
	GLCall(glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)96));
	GLCall(glVertexAttribPointer(14, 4, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)112));
	GLCall(glVertexAttribDivisor(11, 1));
	GLCall(glVertexAttribDivisor(12, 1));
	GLCall(glVertexAttribDivisor(13, 1));
	GLCall(glVertexAttribDivisor(14, 1));

	GLCall(glEnableVertexAttribArray(15));
	GLCall(glVertexAttribPointer(15, 3, GL_FLOAT, GL_FALSE, (sizeof(glm::mat4) * 2) + sizeof(PerInstanceData), (const void*)128));
	GLCall(glVertexAttribDivisor(15, 1));

	GLCall(glGenBuffers(1, &m_ElementBufferID));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBufferID));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_BatchElementsBufferSize, nullptr, GL_STREAM_DRAW));

	GLCall(glGenBuffers(1, &m_DrawIndirectBufferID));
	GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBufferID));
	GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, m_BatchDrawCommandsBufferSize, nullptr, GL_STREAM_DRAW));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	GLCall(glBindVertexArray(0));
}

RenderBatch::~RenderBatch()
{
	delete m_AnimationDataUBO;

	GLCall(glBindVertexArray(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLCall(glDeleteBuffers(1, &m_VertexBufferID));
	GLCall(glDeleteBuffers(1, &m_ElementBufferID));
	GLCall(glDeleteBuffers(1, &m_DrawIndirectBufferID));
	GLCall(glDeleteBuffers(1, &m_TransformsBufferID));

	GLCall(glDeleteVertexArrays(1, &m_VertexArrayID));
}

void* RenderBatch::GetVRAMMemoryPointer(int glBufferType, unsigned int byteOffset, unsigned int numBytes)
{
	// Invalidating is heavy on memory. Maybe try double buffering instead invalidating. This way of invalidating is the opengl pendant to D3DLOCK_DISCARD
	//GLCall(glBufferData(glBufferType, numBytes, nullptr, GL_STREAM_DRAW));
	//void* ptr = glMapBufferRange(glBufferType, byteOffset, numBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	// NO INVALIDATING/DOUBLE BUFFER FOR NOW!
	void* ptr = glMapBufferRange(glBufferType, byteOffset, numBytes, GL_MAP_WRITE_BIT);
	ASSERT(ptr != nullptr);
	return ptr;
}

AddToBatchResult RenderBatch::TryAddToBatch(const MeshWorldObject::Mesh& mesh, const std::vector<MeshInstancess>& instances, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, RenderPass renderPass, const std::vector<PointLight>* pointlightsToTest)
{
	using namespace RenderBatch_Private;

	const unsigned int meshIndexCount = mesh.m_VertexIndicies.size();
	const unsigned int meshVertexCount = mesh.m_Vertices.size();

	unsigned int kk = GMTime::s_FrameNumber;
#
	if (meshIndexCount > m_MaxIndicesPerBatch ||
		meshVertexCount > m_MaxVerticesPerBatch)
	{
		// Geometry does not fit into a single batch -> needs a separate drawcallinstances
		return AddToBatchResult::TooBigForBatch;
	}

	if (renderPass == RenderPass::Geometry &&
		m_BoundMaterial != nullptr &&
		*m_BoundMaterial != mesh.m_Material)
	{
		// Material changed. Cannot batch
		return AddToBatchResult::MaterialChanged;
	}

	const unsigned int indexCountLeftInCurrentBatch = m_BatchElementsBufferSize - m_IndexBufferDataOffset;
	const unsigned int vertexCountLeftInCurrentBatch = m_BatchVertexBufferSize - m_VertexBufferDataOffset;

	if (meshIndexCount * sizeof(unsigned int) > indexCountLeftInCurrentBatch ||
		meshVertexCount * sizeof(MeshWorldObject::Mesh::Vertex) > vertexCountLeftInCurrentBatch)
	{
		// Geometry fits into batch, but not in current batch
		return AddToBatchResult::NotFitIntoBatch;
	}

	//REWORK Pass camera into here
	//const FPSCamera& camera = *Globals::fpsCamera;
	const FPSCamera* camera;

	if (renderPass == RenderPass::Geometry &&
		m_BoundMaterial == nullptr)
	{
		m_BoundMaterial = &mesh.m_Material;
		m_BoundMaterial->Bind();

		const glm::mat4 camTransform = camera->GetTransform();

		m_BoundMaterial->GetShader()->SetUniformMat4f("u_Projection", &camera->GetProjection());
		m_BoundMaterial->GetShader()->SetUniformMat4f("u_View", &camTransform);
		m_BoundMaterial->GetShader()->SetUniform1f("u_ClipNear", camera->GetNearClip());
		m_BoundMaterial->GetShader()->SetUniform1f("u_ClipFar", camera->GetFarClip());
		m_BoundMaterial->GetShader()->SetUniform1f("u_GameTime", GMTime::s_Time);
	}

	const glm::mat4 cameraViewProjection = camera->GetProjection() * camera->GetTransform();

	unsigned int numInstances = 0;
	unsigned int baseInstance = m_DrawCommandBaseInstance;

	unsigned int dataIndex = 0;
	//GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_TransformsBufferID));
	gpuData.resize(instances.size());

	std::unordered_map<const SkeletalMeshWorldObjectInstance*, unsigned int> animationDataOffsetMap;
	for (const MeshInstancess& instance : instances)
	{
		const MeshWorldObjectInstance* mwoi = instance.m_Instance;

		glm::mat4 instanceTransform;
		if (mwoi->GetRigidBody().IsValid() && renderPass == RenderPass::Debug)
		{
			instanceTransform = mwoi->GetRigidBody().GetWorldTransform();
		}
		else
		{
			instanceTransform = mwoi->GetTransform();
		}

		IData& pd = gpuData[dataIndex];
		++dataIndex;
		pd.instance = instanceTransform;
		pd.pid = { (float)instance.m_LightIndex, (float)instance.m_TargetLayerID };
		pd.mv = instance.m_MVP;

		// Upload animation data for this instance if needed
		const SkeletalMeshWorldObjectInstance* smwoi = dynamic_cast<const SkeletalMeshWorldObjectInstance*>(instance.m_Instance);
		if (smwoi == nullptr)
			continue;
		


		auto [iter, inserted] = animationDataOffsetMap.emplace(smwoi, 0);
		unsigned int& offset = (*iter).second;

		if (!inserted)
		{
			//Bone data already uploaded for this instance, simply put existing indices
			pd.pid.m_AnimationDataOffset = offset;
			continue;
		}

		const std::vector<glm::mat4>& animationBoneData = smwoi->GetAnimationBoneData();
		if (animationBoneData.size() == 0)
			continue;

		memcpy((char*)m_BatchAnimationDataBufferInter + sizeof(glm::mat4) * m_AnimationDataOffset, (void*)&animationBoneData[0], sizeof(glm::mat4) * animationBoneData.size());

		pd.pid.m_AnimationDataOffset = m_AnimationDataOffset;
		offset = m_AnimationDataOffset;
		m_AnimationDataOffset += animationBoneData.size();
	}

	if (m_UpdateVRAMBuffers)
	{
		m_UpdateVRAMBuffers = false;

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_TransformsBufferID));
		m_BatchTransformsBufferPtr = GetVRAMMemoryPointer(GL_ARRAY_BUFFER, 0, m_BatchTransformsBufferSize);
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

		GLCall(glBindVertexArray(m_VertexArrayID));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID));
		m_BatchVertexBufferPtr = GetVRAMMemoryPointer(GL_ARRAY_BUFFER, 0, m_BatchVertexBufferSize);
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindVertexArray(0));
	
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBufferID));
		m_BatchElementsBufferPtr = GetVRAMMemoryPointer(GL_ELEMENT_ARRAY_BUFFER, 0, m_BatchElementsBufferSize);
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBufferID));
		m_BatchDrawCommandsBufferPtr = GetVRAMMemoryPointer(GL_DRAW_INDIRECT_BUFFER, 0, m_BatchDrawCommandsBufferSize);
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	if (dataIndex > 0)
	{
		{
			SCOPED_TIMER("Moving Data To GPU Buffer");
			memcpy((char*)m_BatchTransformsBufferPtr + m_TransformsBufferDataOffset, (void*)&gpuData[0], sizeof(IData) * dataIndex);
		}
		m_TransformsBufferDataOffset += sizeof(IData) * dataIndex;
		numInstances += dataIndex;
		m_DrawCommandBaseInstance += dataIndex;
	}

	if (numInstances == 0)
	{
		return AddToBatchResult::Success;
	}

	if (renderPass == RenderPass::Geometry)
	{
		NUMBERED_NAMED_COUNTER("Vertex count", mesh.m_Vertices.size());
		NUMBERED_NAMED_COUNTER("Vertex count including instances", mesh.m_Vertices.size() * numInstances);
		NAMED_COUNTER("Mesh count (No instances)");
	}

	{
		SCOPED_TIMER("Moving Data To GPU Buffer");

		memcpy((char*)m_BatchVertexBufferPtr + m_VertexBufferDataOffset, (void*)&mesh.m_Vertices[0], sizeof(MeshWorldObject::Mesh::Vertex) * mesh.m_Vertices.size());
		memcpy((char*)m_BatchElementsBufferPtr + m_IndexBufferDataOffset, (void*)&mesh.m_VertexIndicies[0], sizeof(unsigned int) * mesh.m_VertexIndicies.size());
	}
	 
	//GLCall(glBindVertexArray(m_VertexArrayID));
	//GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID));
	//GLCall(glBufferSubData(GL_ARRAY_BUFFER, m_VertexBufferDataOffset, sizeof(MeshWorldObject::Mesh::Vertex) * mesh.m_Vertices.size(), &mesh.m_Vertices[0]));

	//GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBufferID));
	//GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferDataOffset, sizeof(unsigned int) * mesh.m_VertexIndicies.size(), &mesh.m_VertexIndicies[0]));

	DrawElementsIndirectCommand drawCommand;
	drawCommand.count = mesh.m_VertexIndicies.size();
	drawCommand.instanceCount = numInstances;
	drawCommand.firstIndex = m_DrawCommandFirstIndex;
	drawCommand.baseVertex = m_DrawCommandBaseIndex;
	drawCommand.baseInstance = baseInstance;

	//GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBufferID));
	//GLCall(glBufferSubData(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBufferDataOffset, sizeof(DrawElementsIndirectCommand), &drawCommand));

	{
		SCOPED_TIMER("Moving Data To GPU Buffer");
		memcpy((char*)m_BatchDrawCommandsBufferPtr + m_DrawIndirectBufferDataOffset, (void*)&drawCommand, sizeof(DrawElementsIndirectCommand));
	}

	m_DrawIndirectBufferDataOffset += sizeof(DrawElementsIndirectCommand);

	++m_NumDrawCommands;
	
	m_DrawCommandFirstIndex += mesh.m_VertexIndicies.size();
	m_DrawCommandBaseIndex += mesh.m_Vertices.size();

	m_VertexBufferDataOffset += mesh.m_Vertices.size() * sizeof(MeshWorldObject::Mesh::Vertex);
	m_IndexBufferDataOffset += mesh.m_VertexIndicies.size() * sizeof(unsigned int);

	return AddToBatchResult::Success;
}

void RenderBatch::Flush(RenderPass renderPass)
{
	using namespace RenderBatch_Private;

	if (m_NumDrawCommands > 0)
	{
		switch (renderPass)
		{
		case RenderPass::Debug:
			NAMED_COUNTER("Drawcall (Debug)");
			break;
		case RenderPass::LightDepth:
			NAMED_COUNTER("Drawcall (Light)");
			break;
		case RenderPass::Geometry:
			NAMED_COUNTER("Drawcall (Geometry)");
			break;
		default:
			break;
		}

		{
			NUMBERED_NAMED_COUNTER("Bytes Uploaded To GPU", sizeof(glm::mat4) * m_AnimationDataOffset);
			SCOPED_TIMER("Moving Data To GPU");
			if (m_AnimationDataOffset > 0)
				m_AnimationDataUBO->Write(0, sizeof(glm::mat4) * m_AnimationDataOffset, m_BatchAnimationDataBufferInter);
		}

		NUMBERED_NAMED_COUNTER("Bytes Uploaded To GPU", sizeof(IData) * m_TransformsBufferDataOffset);
		NUMBERED_NAMED_COUNTER("Bytes Uploaded To GPU", sizeof(MeshWorldObject::Mesh::Vertex) * m_VertexBufferDataOffset);
		NUMBERED_NAMED_COUNTER("Bytes Uploaded To GPU", sizeof(unsigned int) * m_IndexBufferDataOffset);
		NUMBERED_NAMED_COUNTER("Bytes Uploaded To GPU", sizeof(DrawElementsIndirectCommand) * m_DrawIndirectBufferDataOffset);

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_TransformsBufferID));
		GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));

		GLCall(glBindVertexArray(m_VertexArrayID));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID));
		GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));

		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBufferID));
		GLCall(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBufferID));
		GLCall(glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER));

		GLCall(glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)0, m_NumDrawCommands, 0));
		
		m_UpdateVRAMBuffers = true;
	}	

	if (m_BoundMaterial != nullptr)
	{
		m_BoundMaterial->Unbind();
		m_BoundMaterial = nullptr;
	}
	
	m_DrawCommandFirstIndex = 0;
	m_DrawCommandBaseIndex = 0;
	m_VertexBufferDataOffset = 0;
	m_IndexBufferDataOffset = 0;
	m_DrawIndirectBufferDataOffset = 0;
	m_TransformsBufferDataOffset = 0;
	m_DrawCommandBaseInstance = 0;
	m_NumDrawCommands = 0;
	m_AnimationDataOffset = 0;
}