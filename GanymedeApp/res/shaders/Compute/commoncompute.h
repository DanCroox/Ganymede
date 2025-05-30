#pragma once

struct AABB
{
	vec4 m_AABB[8];
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
};

struct VisibilityMask
{
	uint m_EntityDataIndex;
	uint m_VisibilityMask; // 32 views total
};

struct InstanceData
{
	mat4 m_Transform;
	uint m_ViewID;
	uint m_MeshID;
	uint m_NumMeshIndices;
	uint m_FaceIndex;
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
	uint m_FaceIndex; //for testing only
};

layout(std430, binding = 18) buffer EntityDataCounterBuffer { uint numEntities; };
layout(std430, binding = 19) buffer EntityVisiblityMasksCounterBuffer { uint numVisibilityMasks; };
layout(std430, binding = 20) buffer AppendCounterBuffer { uint numAppends; };
layout(std430, binding = 21) buffer CommandCounterBuffer { uint numCommands; };
layout(std430, binding = 22) buffer RenderViewCounterBuffer { uint numRenderViews; };

layout(std430, binding = 23) buffer EntityDataBuffer { EntityData entityDatas[]; };
layout(std430, binding = 24) buffer RenderViewBuffer { RenderView renderViews[]; };
layout(std430, binding = 25) buffer VisibilityMaskBuffer { VisibilityMask entityVisibilityMasks[]; };
layout(std430, binding = 26) buffer InstanceDataBuffer { InstanceData instanceDatas[]; };
layout(std430, binding = 27) buffer DrawElementsIndirectCommandBuffer { DrawElementsIndirectCommand drawCommands[]; };
layout(std430, binding = 28) buffer RenderMeshInstanceCommandBuffer { RenderMeshInstanceCommand renderInfos[]; };

layout(local_size_x = 256) in;