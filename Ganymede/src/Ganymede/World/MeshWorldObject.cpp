#include "MeshWorldObject.h"


namespace Ganymede
{
	std::uint32_t MeshWorldObject::Mesh::s_NextMeshID = 0;

	MeshWorldObject::MeshWorldObject(const std::string& name) :
		WorldObject(name) {}
}