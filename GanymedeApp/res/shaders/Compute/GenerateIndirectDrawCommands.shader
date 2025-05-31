#shader compute

#version 460 core

#include "commoncompute.h"

void main() 
{
	uint appendDataIndex = gl_GlobalInvocationID.x;
	if (appendDataIndex >= ssbo_Counters.m_NumAppends) return;

	uint thisGroup = ssbo_InstanceData[appendDataIndex].m_RenderViewGroup;
	uint thisView = ssbo_InstanceData[appendDataIndex].m_ViewID;
	uint thisMesh = ssbo_InstanceData[appendDataIndex].m_MeshID;

	bool newCommand = (appendDataIndex == 0u)
		|| (ssbo_InstanceData[appendDataIndex-1].m_RenderViewGroup != thisGroup)
		|| (ssbo_InstanceData[appendDataIndex-1].m_MeshID != thisMesh);

	if (newCommand)
	{
		uint count = 1u;

		while (appendDataIndex + count < ssbo_Counters.m_NumAppends
		&& ssbo_InstanceData[appendDataIndex+count].m_RenderViewGroup == thisGroup
		&& ssbo_InstanceData[appendDataIndex+count].m_MeshID == thisMesh)
		{
			++count;
		}

		uint cmdIdx = atomicAdd(ssbo_Counters.m_NumIndirectCommands, 1);
		ssbo_IndirectDawCmds[cmdIdx].count = ssbo_InstanceData[appendDataIndex].m_NumMeshIndices;
		ssbo_IndirectDawCmds[cmdIdx].instanceCount = count;
		ssbo_IndirectDawCmds[cmdIdx].firstIndex = 0;
		ssbo_IndirectDawCmds[cmdIdx].baseVertex = 0;
		ssbo_IndirectDawCmds[cmdIdx].baseInstance = appendDataIndex;

		// We store the cmdIdx cause we need to sort ssbo_RenderInfos in a final pass based on ViewID and MeshID
		ssbo_RenderInfos[cmdIdx].m_MeshID = thisMesh;
		ssbo_RenderInfos[cmdIdx].m_ViewID = thisView;
		ssbo_RenderInfos[cmdIdx].m_IndirectCommandIndex = cmdIdx;
		ssbo_RenderInfos[cmdIdx].m_RenderViewGroup = thisGroup;
	}
}