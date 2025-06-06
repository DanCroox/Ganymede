#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/World/MeshWorldObject.h"

namespace Ganymede
{
	struct GCMesh
	{
		 std::vector<MeshWorldObject::Mesh*> m_Meshes;
	};
}