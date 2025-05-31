#include "UpdateDrawDataPass.h"

#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCGPUEntityData.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/ECS/Components/GCSkeletal.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/World/World.h"
#include "glm/glm.hpp"
#include "GL/glew.h"

namespace Ganymede
{
	bool UpdateDrawDataPass::Initialize(RenderContext& renderContext)
	{
		m_AnimationDataSSBO = renderContext.CreateSSBO("AnimationData", 2, 1 * sizeof(glm::mat4), true);
		m_EntityDataSSBO = renderContext.GetSSBO("EntityData");

		return true;
	}
	
	void UpdateDrawDataPass::Execute(RenderContext& renderContext)
	{
		World& world = renderContext.GetWorld();

		auto entitiesToUpdate = world.GetEntities(Include<GCTransform, GCGPUEntityData, GCDynamicMobility>{});
		for (auto [entity, gcTransform, gcGPUEntityData] : entitiesToUpdate.each())
		{
			if (std::optional<GCRigidBody> gcRigidBody = world.GetComponentFromEntity<GCRigidBody>(entity))
			{
				const glm::mat4 rbTransform = gcRigidBody.value().m_RigidBody.GetWorldTransform();
				m_EntityDataSSBO->Write(gcGPUEntityData.m_SSBOIndex * sizeof(EntityData), sizeof(glm::mat4), (void*)&rbTransform);
			}
			else
			{
				m_EntityDataSSBO->Write(gcGPUEntityData.m_SSBOIndex * sizeof(EntityData), sizeof(glm::mat4), (void*)&gcTransform.GetMatrix());
			}
		}

		unsigned int animationDataOffset = 0;

		const std::vector<VisibleEntity>& visibleEntities = renderContext.GetVisibleEntities();
		for (const VisibleEntity& ve : visibleEntities)
		{
			if (std::optional<GCSkeletal> gcSkeletal = world.GetComponentFromEntity<GCSkeletal>(static_cast<Entity>(ve.m_EntityID)))
			{
				const std::vector<glm::mat4>& animationFrame = gcSkeletal.value().m_AnimationBoneData;
				m_AnimationDataSSBO->Write(animationDataOffset * sizeof(glm::mat4), animationFrame.size() * sizeof(glm::mat4), (void*)&animationFrame[0]);
				m_EntityDataSSBO->Write((ve.m_GPUBufferDataIndex * sizeof(EntityData)) + offsetof(EntityData, m_AnimationDataOffset), sizeof(EntityData::m_AnimationDataOffset), &animationDataOffset);
				animationDataOffset += animationFrame.size();
			}
		}

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	}
}