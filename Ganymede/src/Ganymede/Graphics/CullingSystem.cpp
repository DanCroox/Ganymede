#include "CullingSystem.h"

#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/ECS/Components/GCRenderObject.h"
#include "Ganymede/ECS/Components/GCLoadRenderObject.h"
#include "Ganymede/ECS/Components/GCUnloadRenderObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	void CullingSystem::UpdateRenderTags(World& world, const glm::mat4& viewProjection)
	{
		const auto entities = world.GetEntities(Include<GCMesh, GCTransform>{});
		for (auto [entity, gcMesh, gcTransform] : entities.each())
		{
			const glm::mat4 mvp = viewProjection * gcTransform.GetMatrix();

			bool isInFrustum = false;
			for (const MeshWorldObject::Mesh* mesh : gcMesh.m_Meshes)
			{
				isInFrustum = FrustumCulling(*mesh, mvp);
				if (isInFrustum) break;
			}

			//TODO: Right now we load all the submeshes of an entity. Theoretically we can only load those which are not culled - 
			//but with some extra effort. So for now we keep it straight forward and load all meshes if at least one is not culled.
			const bool hasRenderObjectComponent = world.HasComponents<GCRenderObject>(entity);
			if (isInFrustum && !hasRenderObjectComponent)
			{
				world.AddComponent<GCLoadRenderObject>(entity);
			}
			else if (!isInFrustum && hasRenderObjectComponent)
			{
				world.AddComponent<GCUnloadRenderObject>(entity);
			}
		}
	}

	bool CullingSystem::FrustumCulling(const MeshWorldObject::Mesh& mesh, const glm::mat4& mvp)
	{
		int clipSides[6] = { 0 };
		for (const MeshWorldObject::Mesh::BoundingBoxVertex& bbVert : mesh.m_BoundingBoxVertices)
		{
			glm::vec4 clipPoint = mvp * glm::vec4(bbVert.m_Position, 1);

			clipSides[0] += clipPoint.x < -clipPoint.w; //left of Left plane
			clipSides[1] += clipPoint.x > clipPoint.w;  //right of Right plane
			clipSides[2] += clipPoint.y < -clipPoint.w; //below Bottom plane
			clipSides[3] += clipPoint.y > clipPoint.w;  //above Top plane
			clipSides[4] += clipPoint.z < -clipPoint.w; //in front of Near plane
			clipSides[5] += clipPoint.z > clipPoint.w;  //behind Far plane
		}

		const bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
			clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

		return !isOutSideFrustum;
	}
}