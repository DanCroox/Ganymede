#include "GSCreature.h"

#include "Ganymede/AI/NavMesh.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Data/AssetLoader.h"
#include "Ganymede/Data/StaticData.h"
#include "../../ECS/Components/GCCreature.h"
#include "Ganymede/ECS/Components/GCName.h"
#include "Ganymede/ECS/Components/GCSkeletal.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Player/PlayerCharacter.h"
#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	void GSCreature::Initialize(World& world, entt::entity entity, float deltaTime)
	{
		GCName& name = world.GetComponentFromEntity<GCName>(entity).value();
		GCCreature& creatureData = world.GetComponentFromEntity<GCCreature>(entity).value();
		GCTransform& transform = world.GetComponentFromEntity<GCTransform>(entity).value();

		if (std::strcmp(name.m_Name.c_str(), "Matschkopf") == 0)
		{
			AssetLoader& assetLoader = creatureData.m_AssetLoader;
			creatureData.m_IdleAnimation = &StaticData::Instance->m_SceneAnimations[0];
			creatureData.m_WalkingAnimation = &StaticData::Instance->m_SceneAnimations[1];
			creatureData.motionSpeed = Helpers::Random::RandomFloatInRange(1.8f, 2.2f);
			transform.SetScale(Helpers::Random::RandomFloatInRange(.75f, .85f));
		}

		creatureData.m_WalkingAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);
		creatureData.m_IdleAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);

		// Spawn creature in a circular distance to world 0 position
		NavMesh& navMesh = creatureData.m_NavMesh;
		float distance = 100;
		float innerDistance = 10;
		glm::vec3 randompoint;
		navMesh.GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		while (randompoint.x > distance || randompoint.x < -distance || glm::length(randompoint - glm::vec3(0)) < innerDistance)
		{
			navMesh.GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		}
		float x = randompoint.x;

		while (randompoint.z > distance || randompoint.z < -distance || glm::length(randompoint - glm::vec3(0)) < innerDistance)
		{
			navMesh.GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		}
		float z = randompoint.z;

		// Set initial world position of the creature
		transform.SetPosition(x, transform.GetPosition().y, z);

		// Set random point of walk-to destination
		navMesh.GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		TryGoto(transform, creatureData, randompoint);
	}

	void GSCreature::Tick(World& world, entt::entity entity, float deltaTime)
	{
		GCCreature& creatureData = world.GetComponentFromEntity<GCCreature>(entity).value();
		GCTransform& transform = world.GetComponentFromEntity<GCTransform>(entity).value();

		const glm::vec3 thisPostion = transform.GetPosition();
		const glm::vec3 creatureForward = glm::rotate(transform.GetRotation(), glm::vec3(0, 0, -1));
		const glm::vec3 creatureEyePosition = thisPostion + glm::vec3(0, 1, 0);
		const glm::vec3 rootPlayerPosition = creatureData.m_PlayerCharacter.GetPosition();
		const glm::vec3 playerEyePosition = rootPlayerPosition + glm::vec3(0, 1, 0);
		const glm::vec3 creatureToPlayer = glm::normalize(creatureEyePosition - playerEyePosition);
		
		// Update AI State
		const float creaturePlayerSightAngle = glm::degrees(glm::acos(glm::dot(glm::normalize(creatureForward), glm::normalize(creatureToPlayer))));
		if (creatureData.m_AIState == GCCreature::AIState::Attacking)
		{
			// Check last frame ai state. AI shall go to last known player location by finishing the currently stored path
			// We concider this action as "Searching"
			creatureData.m_AIState = GCCreature::AIState::Searching;
		}
		else if (creatureData.m_AIState != GCCreature::AIState::Searching)
		{
			creatureData.m_AIState = GCCreature::AIState::Patroling;
		}

		// Test if player is in angular sight of creature and not hidden behind obstacle
		if (creaturePlayerSightAngle <= 80 && !creatureData.m_PhysicsWorld.RayCast(creatureEyePosition, playerEyePosition).m_HasHit)
		{
			creatureData.m_AIState = GCCreature::AIState::Attacking;
		}

		switch (creatureData.m_AIState)
		{
		case GCCreature::AIState::Patroling:
		{
			if (creatureData.m_AIGotoWaypointsInProgress)
			{
				// Finish current path first before picking a new one
			}
			else
			{
				// Find new random waypoint and move to
				glm::vec3 randomPoint(1);
				if (creatureData.m_NavMesh.GetRandomPointOnNavMesh(randomPoint, glm::vec3(0), 10.f))
				{
					TryGoto(transform, creatureData, randomPoint);
				}
			}
			break;
		}
		case GCCreature::AIState::Attacking:
		{
			if (GMTime::s_Time - creatureData.m_LastUpdateTime > 1)
			{
				TryGoto(transform, creatureData, rootPlayerPosition);
				creatureData.m_LastUpdateTime = GMTime::s_Time;
			}
			break;
		}
		case GCCreature::AIState::Searching:
		{
			if (!creatureData.m_AIGotoWaypointsInProgress)
			{
				creatureData.m_AIState = GCCreature::AIState::Patroling;
			}
			break;
		}
		}

		// Find other creatures to keep distance from. Also let other creatures share awareness of player.
		float closestNPCDistance = 100000.f;

		const auto creatures = world.GetEntities(Include<GCCreature, GCTransform>{});
		for (auto [entity, otherCreatureData, otherCreateTransform] : creatures.each())
		{
			if (&otherCreatureData == &creatureData)
				break;

			if (otherCreatureData.m_AIState == GCCreature::AIState::Attacking && creatureData.m_AIState != GCCreature::AIState::Attacking)
			{
				creatureData.m_AIState = GCCreature::AIState::Attacking;
				TryGoto(transform, creatureData, rootPlayerPosition);
			}

			const glm::vec3 npcPosition = otherCreateTransform.GetPosition();

			float distanceToNPC = glm::length(npcPosition - thisPostion);

			const glm::vec3 creatureToNPC = glm::normalize(thisPostion - npcPosition);
			const float angle = glm::degrees(glm::acos(glm::dot(glm::normalize(creatureForward), glm::normalize(creatureToNPC))));
			if (angle >= 30)
				continue;

			if (distanceToNPC < closestNPCDistance)
				closestNPCDistance = distanceToNPC;
		}
		creatureData.motionSpeedMulti = closestNPCDistance - .5f;
		creatureData.motionSpeedMulti = glm::clamp(creatureData.motionSpeedMulti, 0.f, 1.f);
		float distanceToPlayer = glm::length(thisPostion - rootPlayerPosition);
		distanceToPlayer = glm::pow(distanceToPlayer, 100.f);
		creatureData.motionSpeedMulti *= glm::clamp(distanceToPlayer, 0.f, 1.f);

		UpdateMotion(transform, creatureData, deltaTime);

		GCSkeletal& gcSkeletal = world.GetComponentFromEntity<GCSkeletal>(entity).value();
		UpdateAnimation(creatureData, gcSkeletal, deltaTime);
	}

	bool GSCreature::TryGoto(const GCTransform& gcTransform, GCCreature& gcCreature, glm::vec3 destination)
	{
		const glm::vec3 start = gcTransform.GetPosition();
		const int numWaypoints = gcCreature.m_NavMesh.FindPath(start, destination, 0, 0, gcCreature.m_AIGotoWaypoints);
		gcCreature.m_AIGotoWaypointsInProgress = numWaypoints > 0;
		gcCreature.m_AICurrentWaypointIndex = 0;
		gcCreature.aiWaypointLerp = 0;

		gcCreature.aiFromRotation = gcTransform.GetRotation();
		gcCreature.aiRotationLerp = 0;

		return numWaypoints > 0;
	}

	void GSCreature::UpdateMotion(GCTransform& gcTransform, GCCreature& gcCreature, float deltaTime)
	{
		if (!gcCreature.m_AIGotoWaypointsInProgress || gcCreature.m_AIGotoWaypoints.size() < 2) {
			return;
		}

		// Ensure we don't go out of bounds by checking if we're at the last waypoint
		if (gcCreature.m_AICurrentWaypointIndex >= gcCreature.m_AIGotoWaypoints.size() - 1) {
			gcCreature.m_AIGotoWaypointsInProgress = false;
			return;
		}

		const auto& from = gcCreature.m_AIGotoWaypoints[gcCreature.m_AICurrentWaypointIndex];
		const auto& to = gcCreature.m_AIGotoWaypoints[gcCreature.m_AICurrentWaypointIndex + 1];

		float distance = glm::distance(from, to);

		gcCreature.aiWaypointLerp = glm::min(1.0f, gcCreature.aiWaypointLerp + (deltaTime * gcCreature.motionSpeed * gcCreature.motionSpeedMulti) / distance);
		gcCreature.aiRotationLerp = glm::min(1.0f, gcCreature.aiRotationLerp + deltaTime * 4);

		gcTransform.SetPosition(glm::mix(from, to, gcCreature.aiWaypointLerp));

		glm::vec3 lookAt = glm::normalize(from - to);
		glm::vec3 lookAt2D = glm::normalize(glm::vec3(lookAt.x, 0.f, lookAt.z));

		glm::mat4 rot = glm::lookAt(glm::vec3(0), lookAt2D, glm::vec3(0, 1.f, 0));
		glm::quat targetOrientation = glm::conjugate(glm::toQuat(rot));
		gcTransform.SetRotation(glm::slerp(gcCreature.aiFromRotation, targetOrientation, gcCreature.aiRotationLerp));

		if (gcCreature.aiWaypointLerp >= 1.0f && gcCreature.m_AICurrentWaypointIndex < gcCreature.m_AIGotoWaypoints.size() - 1) {
			gcCreature.m_AICurrentWaypointIndex++;
			gcCreature.aiWaypointLerp = 0;
			gcCreature.aiRotationLerp = 0;
			gcCreature.aiFromRotation = gcTransform.GetRotation();
		}

		if (gcCreature.m_AICurrentWaypointIndex == gcCreature.m_AIGotoWaypoints.size() - 1) {
			gcCreature.m_AIGotoWaypointsInProgress = false;
		}
	}

	void GSCreature::UpdateAnimation(GCCreature& gcCreature, GCSkeletal& gcSkeletal, float deltaTime)
	{
		gcCreature.targetinterp = glm::clamp(gcCreature.motionSpeedMulti * gcCreature.motionSpeed, 0.1f, 1.f);

		gcSkeletal.m_AnimationBoneData.clear();
		for (unsigned int i = 0; i < gcCreature.m_WalkingAnimation->m_BoneFrames.size(); ++i)
		{
			const float idleFrame2 = std::fmod(std::ceil(gcCreature.m_IdleAnimationFrame), gcCreature.m_IdleAnimation->m_BoneFrames[0].size());
			const float walkingFrame2 = std::fmod(std::ceil(gcCreature.m_WalkingAnimationFrame), gcCreature.m_WalkingAnimation->m_BoneFrames[0].size());
			const float frameInterp = glm::fract(gcCreature.m_WalkingAnimationFrame);

			const glm::mat4& idleTrans = gcCreature.m_IdleAnimation->m_BoneFrames[i][(unsigned int)gcCreature.m_IdleAnimationFrame];
			const glm::mat4& walkingTrans = gcCreature.m_WalkingAnimation->m_BoneFrames[i][(unsigned int)gcCreature.m_WalkingAnimationFrame];

			const glm::mat4& idleTrans2 = gcCreature.m_IdleAnimation->m_BoneFrames[i][(unsigned int)idleFrame2];
			const glm::mat4& walkingTrans2 = gcCreature.m_WalkingAnimation->m_BoneFrames[i][(unsigned int)walkingFrame2];

			const glm::mat4 idalFinal = (1 - frameInterp) * idleTrans + frameInterp * idleTrans2;
			const glm::mat4 walkingFinal = (1 - frameInterp) * walkingTrans + frameInterp * walkingTrans2;

			glm::mat4 fin = (1 - gcCreature.interp) * idalFinal + gcCreature.interp * walkingFinal;
			gcSkeletal.m_AnimationBoneData.push_back(fin);
		}

		gcCreature.interp = glm::mix(gcCreature.interp, gcCreature.targetinterp, .2f);

		gcCreature.m_WalkingAnimationFrame = std::fmod((gcCreature.m_WalkingAnimationFrame + GMTime::s_DeltaTime * gcCreature.m_WalkingAnimation->m_FPS * .75f * gcCreature.motionSpeed), gcCreature.m_WalkingAnimation->m_BoneFrames[0].size());
		gcCreature.m_IdleAnimationFrame = std::fmod((gcCreature.m_IdleAnimationFrame + GMTime::s_DeltaTime * gcCreature.m_IdleAnimation->m_FPS), gcCreature.m_IdleAnimation->m_BoneFrames[0].size());
		GM_CORE_ASSERT(gcCreature.m_IdleAnimationFrame < gcCreature.m_IdleAnimation->m_BoneFrames[0].size(), "Current frame must not exceed total number of frames for idle animation.");
		GM_CORE_ASSERT(gcCreature.m_WalkingAnimationFrame < gcCreature.m_WalkingAnimation->m_BoneFrames[0].size(), "Current frame must not exceed total number of frames for walking animation.");
	}
}