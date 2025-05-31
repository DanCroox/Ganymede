#shader compute

#version 460 core

#include "commoncompute.h"

bool IsInFrustum(mat4 mvp, AABB aabb)
{
	int clipSides[6] = { 0,0,0,0,0,0 };
	for (int i = 0 ; i < 8; ++i)
	{
		vec4 clipPoint = mvp * aabb.m_AABB[i];

		if (clipPoint.x < -clipPoint.w) ++clipSides[0];
		if (clipPoint.x > clipPoint.w) ++clipSides[1];
		if (clipPoint.y < -clipPoint.w) ++clipSides[2];
		if (clipPoint.y > clipPoint.w) ++clipSides[3];
		if (clipPoint.z < -clipPoint.w) ++clipSides[4];
		if (clipPoint.z > clipPoint.w) ++clipSides[5];
	}

	bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
		clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

	return !isOutSideFrustum;
}

void main() 
{
	uint entityDataIndex = gl_GlobalInvocationID.x;
	if (entityDataIndex >= ssbo_Counters.m_NumEntities) return;

	EntityData entity = ssbo_EntityData[entityDataIndex];

	bool visibleByAnyView = false;
	uint numViews = ssbo_Counters.m_NumRenderViews;

	for (uint viewIdx = 0; viewIdx < numViews; ++viewIdx)
	{
		RenderView view = ssbo_RenderViews[viewIdx];
		mat4 mvp = view.m_Projection * view.m_Transform * entity.m_Transform;
		if (IsInFrustum(mvp, entity.m_AABB))
		{
			if (visibleByAnyView == false)
			{
				uint mappingIndex = atomicAdd(ssbo_Counters.m_NumVisibleEntities, 1);
				ssbo_VisibleEntities[mappingIndex].m_EntityID = entity.m_EntityID;
				ssbo_VisibleEntities[mappingIndex].m_GPUBufferDataIndex = entityDataIndex;
				visibleByAnyView = true;
			}

			uint idx = atomicAdd(ssbo_Counters.m_NumAppends, 1);
			ssbo_InstanceData[idx].m_Transform = entity.m_Transform;
			ssbo_InstanceData[idx].m_ViewID = view.m_ViewID;
			ssbo_InstanceData[idx].m_MeshID = entity.m_MeshID;
			ssbo_InstanceData[idx].m_NumMeshIndices = entity.m_NumMeshIndices;
			ssbo_InstanceData[idx].m_FaceIndex = view.m_FaceIndex;
			ssbo_InstanceData[idx].m_RenderViewGroup = view.m_RenderViewGroup;
			ssbo_InstanceData[idx].m_EntityDataIndex = entityDataIndex;
		}
	}
}