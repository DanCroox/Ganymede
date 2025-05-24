#pragma once

#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/VertexObject.h"
#include <memory>

namespace Ganymede
{
	struct GCGPUMeshData
	{
		std::shared_ptr<VertexObject> m_VO;
		Material* m_Material;
		size_t m_InstanceDataIndex = 0;
	};
}