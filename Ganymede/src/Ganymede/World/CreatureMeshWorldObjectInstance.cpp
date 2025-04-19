#include "CreatureMeshWorldObjectInstance.h"
#include "World.h"
#include "Ganymede/Log/Log.h"

#include "Ganymede/Log/Log.h"
#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/Common/Helpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Ganymede/Physics/PhysicsWorld.h"

namespace Ganymede
{
	CreatureMeshWorldObjectInstance::CreatureMeshWorldObjectInstance(const MeshWorldObject* meshWorldObject,
		NavMesh& navMesh,
		PlayerCharacter& playerCharacter,
		PhysicsWorld& physicsWorld,
		World& world,
		AssetLoader& assetLoader) : SkeletalMeshWorldObjectInstance(meshWorldObject)
	{
		m_NavMesh = &navMesh;
		m_PlayerCharacter = &playerCharacter;
		m_PhysicsWorld = &physicsWorld;
		m_World = &world;
		m_AssetLoader = &assetLoader;
	}

	void CreatureMeshWorldObjectInstance::OnCreate()
	{
		if (std::strcmp(GetWorldObject()->GetName().c_str(), "Matschkopf") == 0)
		{
			m_IdleAnimation = m_AssetLoader->GetAnimationByName(std::string("Idle"));
			m_WalkingAnimation = m_AssetLoader->GetAnimationByName(std::string("Walking"));
			motionSpeed = Helpers::Random::RandomFloatInRange(1.8f, 2.2f);
			SetScale(Helpers::Random::RandomFloatInRange(.75f, .85f));
		}

		m_WalkingAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);
		m_IdleAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);

		// Spawn creature in a circular distance to world 0 position
		float distance = 100;
		float innerDistance = 10;
		glm::vec3 randompoint;
		m_NavMesh->GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		while (randompoint.x > distance || randompoint.x < -distance || glm::length(randompoint - glm::vec3(0)) < innerDistance)
		{
			m_NavMesh->GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		}
		float x = randompoint.x;

		while (randompoint.z > distance || randompoint.z < -distance || glm::length(randompoint - glm::vec3(0)) < innerDistance)
		{
			m_NavMesh->GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		}
		float z = randompoint.z;

		// Set initial world position of the creature
		SetPosition(x, GetPosition().y, z);

		// Set random point of walk-to destination
		m_NavMesh->GetRandomPointOnNavMesh(randompoint, glm::vec3(0), 10.f);
		TryGoto(randompoint);
	}

	void CreatureMeshWorldObjectInstance::Tick(float deltaTime)
	{
		const glm::vec3 thisPostion = GetPosition();
		const glm::vec3 creatureForward = glm::rotate(GetQuaternion(), glm::vec3(0, 0, -1));
		const glm::vec3 creatureEyePosition = thisPostion + glm::vec3(0, 1, 0);
		const glm::vec3 rootPlayerPosition = m_PlayerCharacter->GetPosition();
		const glm::vec3 playerEyePosition = rootPlayerPosition + glm::vec3(0, 1, 0);
		const glm::vec3 creatureToPlayer = glm::normalize(creatureEyePosition - playerEyePosition);

		// Update AI State
		const float creaturePlayerSightAngle = glm::degrees(glm::acos(glm::dot(glm::normalize(creatureForward), glm::normalize(creatureToPlayer))));
		if (m_AIState == AIState::Attacking)
		{
			// Check last frame ai state. AI shall go to last known player location by finishing the currently stored path
			// We concider this action as "Searching"
			m_AIState = AIState::Searching;
		}
		else if (m_AIState != AIState::Searching)
		{
			m_AIState = AIState::Patroling;
		}

		// Test if player is in angular sight of creature and not hidden behind obstacle
		if (creaturePlayerSightAngle <= 80 && !m_PhysicsWorld->RayCast(creatureEyePosition, playerEyePosition).m_HasHit)
		{
			m_AIState = AIState::Attacking;
		}

		switch (m_AIState)
		{
			case AIState::Patroling:
			{
				if (m_AIGotoWaypointsInProgress)
				{
					// Finish current path first before picking a new one
				}
				else
				{
					// Find new random waypoint and move to
					glm::vec3 randomPoint(1);
					if (m_NavMesh->GetRandomPointOnNavMesh(randomPoint, glm::vec3(0), 10.f))
					{
						TryGoto(randomPoint);
					}
				}
				break;
			}
			case AIState::Attacking:
			{
				if (GMTime::s_Time - m_LastUpdateTime > 1)
				{
					TryGoto(rootPlayerPosition);
					m_LastUpdateTime = GMTime::s_Time;
				}
				break;
			}
			case AIState::Searching:
			{
				if (!m_AIGotoWaypointsInProgress)
				{
					m_AIState = AIState::Patroling;
				}
				break;
			}
		}

		// Find other creatures to keep distance from. Also let other creatures share awareness of player.
		float closestNPCDistance = 100000.f;

		const ConstListSlice<SkeletalMeshWorldObjectInstance*> npcs = m_World->GetWorldObjectInstances<SkeletalMeshWorldObjectInstance>();
		for (const SkeletalMeshWorldObjectInstance* npc : npcs)
		{
			if (npc == this)
				break;

			if (const CreatureMeshWorldObjectInstance* creatur = dynamic_cast<const CreatureMeshWorldObjectInstance*>(npc))
			{
				if (creatur->m_AIState == AIState::Attacking && m_AIState != AIState::Attacking)
				{
					m_AIState = AIState::Attacking;
					TryGoto(rootPlayerPosition);
				}
			}

			const glm::vec3 npcPosition = npc->GetPosition();

			float distanceToNPC = glm::length(npcPosition - thisPostion);

			const glm::vec3 creatureToNPC = glm::normalize(thisPostion - npcPosition);
			const float angle = glm::degrees(glm::acos(glm::dot(glm::normalize(creatureForward), glm::normalize(creatureToNPC))));
			if (angle >= 30)
				continue;

			if (distanceToNPC < closestNPCDistance)
				closestNPCDistance = distanceToNPC;
		}
		motionSpeedMulti = closestNPCDistance - .5f;
		motionSpeedMulti = glm::clamp(motionSpeedMulti, 0.f, 1.f);
		float distanceToPlayer = glm::length(thisPostion - rootPlayerPosition);
		distanceToPlayer = glm::pow(distanceToPlayer, 100.f);
		motionSpeedMulti *= glm::clamp(distanceToPlayer, 0.f, 1.f);

		UpdateMotion(deltaTime);
		UpdateAnimation(deltaTime);
	};

	void CreatureMeshWorldObjectInstance::UpdateMotion(float deltaTime)
	{
		if (!m_AIGotoWaypointsInProgress || m_AIGotoWaypoints.size() < 2) {
			return;
		}

		// Ensure we don't go out of bounds by checking if we're at the last waypoint
		if (m_AICurrentWaypointIndex >= m_AIGotoWaypoints.size() - 1) {
			m_AIGotoWaypointsInProgress = false;
			return;
		}

		const auto& from = m_AIGotoWaypoints[m_AICurrentWaypointIndex];
		const auto& to = m_AIGotoWaypoints[m_AICurrentWaypointIndex + 1];

		float distance = glm::distance(from, to);

		aiWaypointLerp = glm::min(1.0f, aiWaypointLerp + (deltaTime * motionSpeed * motionSpeedMulti) / distance);
		aiRotationLerp = glm::min(1.0f, aiRotationLerp + deltaTime * 4);

		SetPosition(glm::mix(from, to, aiWaypointLerp));

		glm::vec3 lookAt = glm::normalize(from - to);
		glm::vec3 lookAt2D = glm::normalize(glm::vec3(lookAt.x, 0.f, lookAt.z));

		glm::mat4 rot = glm::lookAt(glm::vec3(0), lookAt2D, glm::vec3(0, 1.f, 0));
		glm::quat targetOrientation = glm::conjugate(glm::toQuat(rot));
		SetQuaternion(glm::slerp(aiFromRotation, targetOrientation, aiRotationLerp));

		if (aiWaypointLerp >= 1.0f && m_AICurrentWaypointIndex < m_AIGotoWaypoints.size() - 1) {
			m_AICurrentWaypointIndex++;
			aiWaypointLerp = 0;
			aiRotationLerp = 0;
			aiFromRotation = GetQuaternion();
		}

		if (m_AICurrentWaypointIndex == m_AIGotoWaypoints.size() - 1) {
			m_AIGotoWaypointsInProgress = false;
		}
	}

	bool CreatureMeshWorldObjectInstance::TryGoto(glm::vec3 destination)
	{
		glm::vec3 start = GetPosition();
		const int numWaypoints = m_NavMesh->FindPath(start, destination, 0, 0, m_AIGotoWaypoints);
		m_AIGotoWaypointsInProgress = numWaypoints > 0;
		m_AICurrentWaypointIndex = 0;
		aiWaypointLerp = 0;

		aiFromRotation = GetQuaternion();
		aiRotationLerp = 0;

		return numWaypoints > 0;
	}

	void CreatureMeshWorldObjectInstance::UpdateAnimation(float frameDelta)
	{
		targetinterp = glm::clamp(motionSpeedMulti * motionSpeed, 0.1f, 1.f);

		m_AnimationBoneData.clear();
		for (unsigned int i = 0; i < m_WalkingAnimation->m_Bones.size(); ++i)
		{
			const float idleFrame2 = std::fmod(std::ceil(m_IdleAnimationFrame), m_IdleAnimation->m_Bones[0].m_Frames.size());
			const float walkingFrame2 = std::fmod(std::ceil(m_WalkingAnimationFrame), m_WalkingAnimation->m_Bones[0].m_Frames.size());
			const float frameInterp = glm::fract(m_WalkingAnimationFrame);

			const glm::mat4& idleTrans = m_IdleAnimation->m_Bones[i].m_Frames[(unsigned int)m_IdleAnimationFrame];
			const glm::mat4& walkingTrans = m_WalkingAnimation->m_Bones[i].m_Frames[(unsigned int)m_WalkingAnimationFrame];

			const glm::mat4& idleTrans2 = m_IdleAnimation->m_Bones[i].m_Frames[(unsigned int)idleFrame2];
			const glm::mat4& walkingTrans2 = m_WalkingAnimation->m_Bones[i].m_Frames[(unsigned int)walkingFrame2];

			const glm::mat4 idalFinal = (1 - frameInterp) * idleTrans + frameInterp * idleTrans2;
			const glm::mat4 walkingFinal = (1 - frameInterp) * walkingTrans + frameInterp * walkingTrans2;

			glm::mat4 fin = (1 - interp) * idalFinal + interp * walkingFinal;
			m_AnimationBoneData.push_back(fin);
		}

		interp = glm::mix(interp, targetinterp, .2f);

		m_WalkingAnimationFrame = std::fmod((m_WalkingAnimationFrame + GMTime::s_DeltaTime * m_WalkingAnimation->m_FPS * .75f * motionSpeed), m_WalkingAnimation->m_Bones[0].m_Frames.size());
		m_IdleAnimationFrame = std::fmod((m_IdleAnimationFrame + GMTime::s_DeltaTime * m_IdleAnimation->m_FPS), m_IdleAnimation->m_Bones[0].m_Frames.size());
		GM_CORE_ASSERT(m_IdleAnimationFrame < m_IdleAnimation->m_Bones[0].m_Frames.size(), "Current frame must not exceed total number of frames for idle animation.");
		GM_CORE_ASSERT(m_WalkingAnimationFrame < m_WalkingAnimation->m_Bones[0].m_Frames.size(), "Current frame must not exceed total number of frames for walking animation.");
	}
}