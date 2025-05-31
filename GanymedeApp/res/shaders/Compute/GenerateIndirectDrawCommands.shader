#shader compute

#version 460 core

#include "commoncompute.h"

void main() 
{
	uint appendDataIndex = gl_GlobalInvocationID.x;
	if (appendDataIndex >= m_Counters.m_NumAppends) return;

	uint thisGroup = instanceDatas[appendDataIndex].m_RenderViewGroup;
	uint thisView = instanceDatas[appendDataIndex].m_ViewID;
	uint thisMesh = instanceDatas[appendDataIndex].m_MeshID;

	bool newCommand = (appendDataIndex == 0u)
		|| (instanceDatas[appendDataIndex-1].m_RenderViewGroup != thisGroup)
		|| (instanceDatas[appendDataIndex-1].m_MeshID != thisMesh);

	if (newCommand)
	{
		uint count = 1u;

		while (appendDataIndex + count < m_Counters.m_NumAppends
		&& instanceDatas[appendDataIndex+count].m_RenderViewGroup == thisGroup
		&& instanceDatas[appendDataIndex+count].m_MeshID == thisMesh)
		{
			++count;
		}

		uint cmdIdx = atomicAdd(m_Counters.m_NumIndirectCommands, 1);
		drawCommands[cmdIdx].count = instanceDatas[appendDataIndex].m_NumMeshIndices;
		drawCommands[cmdIdx].instanceCount = count;
		drawCommands[cmdIdx].firstIndex = 0;
		drawCommands[cmdIdx].baseVertex = 0;
		drawCommands[cmdIdx].baseInstance = appendDataIndex;

		// We store the cmdIdx cause we need to sort renderInfos in a final pass based on ViewID and MeshID
		renderInfos[cmdIdx].m_MeshID = thisMesh;
		renderInfos[cmdIdx].m_ViewID = thisView;
		renderInfos[cmdIdx].m_IndirectCommandIndex = cmdIdx;
		renderInfos[cmdIdx].m_RenderViewGroup = thisGroup;
	}
}