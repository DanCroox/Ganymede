#pragma once

struct AABB
{
	vec4 m_AABB[8];
};

struct ComputePassCounters
{
	uint m_NumEntities;
	uint m_NumAppends;
	uint m_NumIndirectCommands;
	uint m_NumRenderViews;
};

struct EntityData
{
	mat4 m_Transform;
	AABB m_AABB;
	uint m_MeshID;
	uint m_NumMeshIndices;
	uint m_Pad2;
	uint m_Pad3;
};

struct RenderView
{
	mat4 m_Transform;
	mat4 m_Perspective;
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
	uint m_Pad1;
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

layout(std430, binding = 20) buffer ComputePassCountersBuffer { ComputePassCounters m_Counters; };

layout(std430, binding = 21) buffer EntityDataBuffer { EntityData entityDatas[]; };
layout(std430, binding = 22) buffer RenderViewBuffer { RenderView renderViews[]; };
layout(std430, binding = 23) buffer InstanceDataBuffer { InstanceData instanceDatas[]; };
layout(std430, binding = 24) buffer DrawElementsIndirectCommandBuffer { DrawElementsIndirectCommand drawCommands[]; };
layout(std430, binding = 25) buffer RenderMeshInstanceCommandBuffer { RenderMeshInstanceCommand renderInfos[]; };

layout(local_size_x = 256) in;