#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/RenderPass.h"
#include "Ganymede/System/FreeList.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "glm/glm.hpp"
#include <memory>

namespace Ganymede
{
	class SSBO;
	class Shader;

	struct ComputePassCounters
	{
		glm::uint32 m_NumEntities;
		glm::uint32 m_NumAppends;
		glm::uint32 m_NumIndirectCommands;
		glm::uint32 m_NumRenderViews;
	};

	struct alignas(16) GPURenderView
	{
		glm::mat4 m_Transform;
		glm::mat4 m_Perspective;
		glm::vec4 m_WorldPosition;
		float m_NearClip;
		float m_FarClip;
		glm::uint m_ViewID;
		glm::uint m_FaceIndex;
		glm::uint m_RenderViewGroup;
		glm::uint m_Pad1;
		glm::uint m_Pad2;
		glm::uint m_Pad3;
	};

	struct DrawElementsIndirectCommand
	{
		glm::uint  count;
		glm::uint  instanceCount;
		glm::uint  firstIndex;
		glm::int32  baseVertex;
		glm::uint  baseInstance;
	};

	struct alignas(16) EntityData
	{
		glm::mat4 m_Transform;
		std::array<glm::vec4, 8> m_AABB;
		glm::uint m_MeshID;
		glm::uint m_NumMeshIndices;
		glm::uint m_Pad2;
		glm::uint m_Pad3;
	};

	struct alignas(16) InstanceData
	{
		glm::mat4 m_Transform;
		glm::uint m_ViewID;
		glm::uint m_MeshID;
		glm::uint m_NumMeshIndices;
		glm::uint m_FaceIndex;
		glm::uint m_RenderViewGroup;
		glm::uint m_Pad1;
		glm::uint m_Pad2;
		glm::uint m_Pad3;
	};

	struct GCUploaded
	{
		unsigned int m_SSBOIndex;
	};

	class GANYMEDE_API ComputePass : public RenderPass2
	{
	public:
		~ComputePass();

		bool Initialize(RenderContext& renderContext) override;
		void Execute(RenderContext& renderContext) override;

	private:
		SSBO* ssbo_ComputePassCounters; //ComputePassCounters

		SSBO* ssbo_EntityData; //EntityData
		SSBO* ssbo_RenderViews; //RenderView
		SSBO* ssbo_AppendBuffer; //InstanceData
		SSBO* ssbo_IndirectDrawCmds; //DrawElementsIndirectCommand
		SSBO* ssbo_RenderInfos; //RenderMeshInstanceCommand

		unsigned int m_NumEntities = 0;

		Shader* m_FindVisibleEntitiesCompute;
		Shader* m_GenerateIndirectDrawCommands;
	};
}