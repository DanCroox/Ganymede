#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/World/MeshWorldObject.h"
#include "glm/glm.hpp"

namespace Ganymede
{
	class MeshWorldObject::Mesh;
	class World;

	class GANYMEDE_API CullingSystem
	{
	public:
		static void UpdateRenderTags(World& world, const glm::mat4& viewProjection);

	private:
		static bool FrustumCulling(const MeshWorldObject::Mesh& mesh, const glm::mat4& mvp);
	};
}