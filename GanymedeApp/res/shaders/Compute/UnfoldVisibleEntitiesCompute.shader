#shader compute

#version 460 core

#include "commoncompute.h"

void main() 
{
	uint visibilityMaskIndex = gl_GlobalInvocationID.x;
    if (visibilityMaskIndex >= numVisibilityMasks) return;

	VisibilityMask visibilityMask = entityVisibilityMasks[visibilityMaskIndex];

    uint viewMask = visibilityMask.m_VisibilityMask;

    EntityData entityData = entityDatas[visibilityMask.m_EntityDataIndex];

	while (viewMask != 0u)
    {
        uint viewID = findLSB(viewMask);          // Index des niedrigsten gesetzten Bits
        viewMask &= ~(1u << viewID);              // dieses Bit löschen
        uint idx = atomicAdd(numAppends, 1);
        instanceDatas[idx].m_Transform = entityData.m_Transform;
        instanceDatas[idx].m_ViewID = viewID;
        instanceDatas[idx].m_MeshID = entityData.m_MeshID;
        instanceDatas[idx].m_NumMeshIndices = entityData.m_NumMeshIndices;
    }
}