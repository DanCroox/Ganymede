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
	if (entityDataIndex >= numEntities) return;

	EntityData entity = entityDatas[entityDataIndex];

	uint visibilityMaskIndex = 0;
	uint numViews = 1;
	
	for (uint viewIdx = 0; viewIdx < numViews; ++viewIdx)
	{
		RenderView view = renderViews[0];
		mat4 mvp = view.m_Perspective * view.m_Transform * entity.m_Transform;
		if (IsInFrustum(mvp, entity.m_AABB))
		{
			if (viewIdx == 0)
			{
				visibilityMaskIndex = atomicAdd(numVisibilityMasks, 1);
				entityVisibilityMasks[visibilityMaskIndex].m_EntityDataIndex = entityDataIndex;
			}
			entityVisibilityMasks[visibilityMaskIndex].m_VisibilityMask |= (1u << viewIdx);
		}
	}
}