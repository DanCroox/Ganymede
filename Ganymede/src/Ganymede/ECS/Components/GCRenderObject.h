#pragma once
#include "Ganymede/World/World.h"
#include "Ganymede/ECS/Components/GCGPUMeshData.h"
#include <memory>

namespace Ganymede
{
	struct GCRenderObject
	{
		std::vector<Entity> m_RenderEntities;
		size_t m_InstanceDataIndex = 0;
	};
}