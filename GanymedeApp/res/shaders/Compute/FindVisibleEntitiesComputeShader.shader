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
	if (entityDataIndex >= m_Counters.m_NumEntities) return;

	EntityData entity = entityDatas[entityDataIndex];

	uint numViews = m_Counters.m_NumRenderViews;

	for (uint viewIdx = 0; viewIdx < numViews; ++viewIdx)
	{
		RenderView view = renderViews[viewIdx];
		mat4 mvp = view.m_Perspective * view.m_Transform * entity.m_Transform;
		if (IsInFrustum(mvp, entity.m_AABB))
		{
			uint idx = atomicAdd(m_Counters.m_NumAppends, 1);
			instanceDatas[idx].m_Transform = entity.m_Transform;
			instanceDatas[idx].m_ViewID = view.m_ViewID;
			instanceDatas[idx].m_MeshID = entity.m_MeshID;
			instanceDatas[idx].m_NumMeshIndices = entity.m_NumMeshIndices;
			instanceDatas[idx].m_FaceIndex = view.m_FaceIndex;
			instanceDatas[idx].m_RenderViewGroup = view.m_RenderViewGroup;
		}
	}
}