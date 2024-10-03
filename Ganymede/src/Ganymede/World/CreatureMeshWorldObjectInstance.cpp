#include "CreatureMeshWorldObjectInstance.h"
#include "World.h"

#include "Ganymede/Runtime/GMTime.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Physics/PhysicsWorld.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void CreatureMeshWorldObjectInstance::Tick(float deltaTime)
{
	const glm::vec3 thisPostion = GetPosition();

	const glm::vec3 creatureForward = glm::rotate(GetQuaternion(), glm::vec3(0, 0, -1));
	const glm::vec3 creaturePosition = thisPostion + glm::vec3(0,1,0);

	const glm::vec3 rootPlayerPosition = m_PlayerCharacter->GetPosition();
	const glm::vec3 playerPosition = rootPlayerPosition + glm::vec3(0,1,0);
	
	const glm::vec3 creatureToPlayer = glm::normalize(creaturePosition - playerPosition);

	const float angle = glm::degrees(glm::acos(glm::dot(glm::normalize(creatureForward), glm::normalize(creatureToPlayer))));

	if (aiState == AIState::Attacking)
	{
		// Check last frame ai state. AI shall go to last known player location by finishing the path
		aiState = AIState::Searching;
	}
	else if (aiState != AIState::Searching)
		aiState = AIState::Patroling;

	if (angle <= 80)
	{
		const RayResult result = m_PhysicsWorld->RayCast(creaturePosition, playerPosition);
		if (!result.m_HasHit)
		{
			aiState = AIState::Attacking;
		}
	}

	switch (aiState)
	{
	case AIState::Patroling:

		if (!aiGotoWaypointsInProgress)
		{
			glm::vec3 randomPoint(1);
			if (m_NavMesh->GetRandomPointOnNavMesh(randomPoint, glm::vec3(0), 10.f))
			{
				
				TryGoto(randomPoint);
			}
		}
		break;
	case AIState::Attacking:
		if (GMTime::s_Time - m_LastUpdateTime > 1)
		{
			TryGoto(rootPlayerPosition);
			m_LastUpdateTime = GMTime::s_Time;
		}
		break;
	case AIState::Searching:
		if (!aiGotoWaypointsInProgress)
			aiState = AIState::Patroling;
		break;
	}

	float closestNPCDistance = 100000.f;
	

	const std::vector<SkeletalMeshWorldObjectInstance*>& npcs = *m_World->GetWorldObjectInstancesByType<SkeletalMeshWorldObjectInstance>();
	for (const SkeletalMeshWorldObjectInstance* npc : npcs)
	{
		if (npc == this)
			break;

		if (const CreatureMeshWorldObjectInstance* creatur = dynamic_cast<const CreatureMeshWorldObjectInstance*>(npc))
		{
			if (creatur->aiState == AIState::Attacking && aiState != AIState::Attacking)
			{
				aiState = AIState::Attacking;
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
	distanceToPlayer = glm::pow(distanceToPlayer, 50.f);

	motionSpeedMulti *= glm::clamp(distanceToPlayer, 0.f, 1.f);

	//motionSpeedMulti = 1.f;
	//Globals::navMesh->SetAgentSpeed(m_NaviAgentID, motionSpeedMulti);
	UpdateMotion(deltaTime);
	UpdateAnimation(deltaTime);
};

void CreatureMeshWorldObjectInstance::UpdateMotion(float deltaTime)
{
	if (!aiGotoWaypointsInProgress)
	{
		return;
	}
	
	/*
	NavMesh& navmesh = *Globals::navMesh;
	const float* newpos = navmesh.m_Crowd->getAgent(m_NaviAgentID)->npos;
	const float* newDir = navmesh.m_Crowd->getAgent(m_NaviAgentID)->vel;
	glm::vec3 newPosiion = glm::vec3(newpos[0], newpos[1], newpos[2]);
	glm::vec3 newDirection = -glm::vec3(newDir[0], newDir[1], newDir[2]);
	

	//motionSpeedMulti = glm::clamp(glm::length(newDirection), 0.f,1.f);


	newDirection = glm::normalize(newDirection);

	glm::mat4 rot = glm::lookAt(newPosiion, newPosiion + newDirection, glm::vec3(0, 1.f, 0));
	glm::vec3 eulerAngles;
	glm::quat orientation = glm::conjugate(glm::toQuat(rot));

	SetQuaternion(orientation);
	SetQuaternion(glm::slerp(GetQuaternion(), orientation, .5f));


	SetPosition(newPosiion);
	if (glm::length(m_NavDestionation - newPosiion) < -.2f)
	{
		aiGotoWaypointsInProgress = false;
	}

	return;
	*/
	
	if (aiGotoWaypoints.size() == 0)
		return;
	
	const glm::vec3 from = aiGotoWaypoints[aiCurrentWaypointIndex];
	const glm::vec3 to = aiGotoWaypoints[aiCurrentWaypointIndex + 1];

	float distance = glm::distance(from, to);

	aiWaypointLerp += (deltaTime * (1.f / distance) * motionSpeed * motionSpeedMulti);
	aiWaypointLerp = glm::min(1.0f, aiWaypointLerp);

	aiRotationLerp += deltaTime * 4;
	aiRotationLerp = glm::min(1.0f, aiRotationLerp);

	glm::vec3 newPosition = glm::mix(from, to, aiWaypointLerp);

	SetPosition(newPosition + glm::vec3(0, -.05f, 0));

	const glm::vec3 lookAt = glm::normalize(from - to);
	glm::vec3 lookAt2D(lookAt.x, 0.f, lookAt.z);
	lookAt2D = glm::normalize(lookAt2D);

	glm::mat4 rot = glm::lookAt(newPosition, newPosition + lookAt2D, glm::vec3(0, 1.f, 0));
	glm::vec3 eulerAngles;
	glm::quat orientation = glm::conjugate(glm::toQuat(rot));

	SetQuaternion(glm::slerp(aiFromRotation, orientation, aiRotationLerp));

	if (aiWaypointLerp == 1.0f && aiCurrentWaypointIndex < aiGotoWaypoints.size() - 1)
	{
		++aiCurrentWaypointIndex;
		aiWaypointLerp = 0;
		aiRotationLerp = 0;
		aiFromRotation = GetQuaternion();
	}

	if (aiCurrentWaypointIndex == aiGotoWaypoints.size() - 1)
	{
		aiGotoWaypointsInProgress = false;
	}
	
}

bool CreatureMeshWorldObjectInstance::TryGoto(glm::vec3 destination)
{
	/*
	m_NavDestionation = destination;
	Globals::navMesh->NavigateAgentToDestination(m_NaviAgentID, destination);
	//aiGotoWaypointsInProgress = true;
	return true;
	*/

	glm::vec3 start = GetPosition();
	const int result = m_NavMesh->FindPath(&start.x, &destination.x, 0, 0, aiGotoWaypoints);

	if (result > 0)
	{
		aiGotoWaypointsInProgress = true;
		aiCurrentWaypointIndex = 0;
		aiWaypointLerp = 0;
	}

	aiFromRotation = GetQuaternion();
	aiRotationLerp = 0;

	return result > 0;
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
	ASSERT(m_IdleAnimationFrame < m_IdleAnimation->m_Bones[0].m_Frames.size());
	ASSERT(m_WalkingAnimationFrame < m_WalkingAnimation->m_Bones[0].m_Frames.size());
}