#pragma once

struct AABB
{
	vec4 m_AABB[8];
};

struct VisibleEntity
{
	uint m_EntityID; // entt ID
	uint m_GPUBufferDataIndex;
};

struct ComputePassCounters
{
	uint m_NumEntities;
	uint m_NumAppends;
	uint m_NumIndirectCommands;
	uint m_NumRenderViews;
	uint m_NumVisibleEntities;
	uint m_Pad1;
	uint m_Pad2;
	uint m_Pad3;
};

struct EntityData
{
	mat4 m_Transform;
	AABB m_AABB;
	uint m_MeshID;
	uint m_NumMeshIndices;
	uint m_AnimationDataOffset;
	uint m_EntityID; // entt ID
};

struct RenderView
{
	mat4 m_Transform;
	mat4 m_Projection;
	vec4 m_WorldPosition;
	float m_NearClip;
	float m_FarClip;
	uint m_ViewID;
	uint m_FaceIndex;
	uint m_RenderViewGroup;
	uint m_Pad1;
	uint m_Pad2;
	uint m_Pad3;
};

struct InstanceData
{
	mat4 m_Transform;
	uint m_ViewID;
	uint m_MeshID;
	uint m_NumMeshIndices;
	uint m_FaceIndex;
	uint m_RenderViewGroup;
	uint m_EntityDataIndex;
	uint m_Pad2;
	uint m_Pad3;
};

struct DrawElementsIndirectCommand
{
	uint  count;
	uint  instanceCount;
	uint  firstIndex;
	int  baseVertex;
	uint  baseInstance;
};

struct RenderMeshInstanceCommand
{
	uint m_MeshID;
	uint m_ViewID;
	uint m_IndirectCommandIndex;
	uint m_RenderViewGroup;
};

struct CommonShaderData
{
	float m_GameTime;
	float m_DeltaTime;
	uint m_FrameNumber;
	uint m_Pad1;
};

layout(std430, binding = 4) buffer CommonShaderDataBlock { CommonShaderData ssbo_CommonData; };

layout(std430, binding = 20) buffer ComputePassCountersBuffer { ComputePassCounters ssbo_Counters; };

// Stores EntityData per entitiy in the world
layout(std430, binding = 21) buffer EntityDataBuffer { EntityData ssbo_EntityData[]; };

// Stores all RenderViews (basically cameras) - Used for culling and to generate InsatnceData list
layout(std430, binding = 22) buffer RenderViewBuffer { RenderView ssbo_RenderViews[]; };

// Stores one entry per visible Entity per RenderView - Used to generate the actual Indirect command list
layout(std430, binding = 23) buffer InstanceDataBuffer { InstanceData ssbo_InstanceData[]; };

// Stores the indirect render commands
layout(std430, binding = 24) buffer DrawElementsIndirectCommandBuffer { DrawElementsIndirectCommand ssbo_IndirectDawCmds[]; };

// Stores one entry per indirect command. Read back to CPU. CPU iterates over the list, binds related mesh via m_MeshID and issues the indirect render command via m_IndirectCommandIndex
layout(std430, binding = 25) buffer RenderMeshInstanceCommandBuffer { RenderMeshInstanceCommand ssbo_RenderInfos[]; };

// Stores one entry per visible Entity (once an Entity is visible by at least one camera) - Read back to CPU to update data like animation etc.
layout(std430, binding = 26) buffer VisibleEntitiesBuffer { VisibleEntity ssbo_VisibleEntities[]; };

layout(local_size_x = 256) in;