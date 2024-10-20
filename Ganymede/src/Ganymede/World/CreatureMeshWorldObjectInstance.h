#pragma once

#include "Ganymede/Core/Core.h"

#include "SkeletalMeshWorldObjectInstance.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Data/AssetLoader.h"
#include "Ganymede/AI/NavMesh.h"
#include "Ganymede/Player/PlayerCharacter.h"
#include <iostream>

namespace Ganymede
{

	class GANYMEDE_API CreatureMeshWorldObjectInstance : public SkeletalMeshWorldObjectInstance
	{
	public:
		GM_GENERATE_CLASSTYPEINFO(CreatureMeshWorldObjectInstance, SkeletalMeshWorldObjectInstance);

		enum AIState
		{
			Patroling,
			Searching,
			Attacking,
		};

		CreatureMeshWorldObjectInstance(const MeshWorldObject* meshWorldObject,
			NavMesh& navMesh,
			PlayerCharacter& playerCharacter,
			PhysicsWorld& physicsWorld,
			World& world,
			AssetLoader& assetLoader);

		void OnCreate() override;

		void Tick(float deltaTime) override;

		void UpdateMotion(float deltaTime);
		void UpdateAnimation(float deltaTime);
		bool TryGoto(glm::vec3 destination);

		AIState m_AIState = AIState::Patroling;

		World* m_World;
		PhysicsWorld* m_PhysicsWorld;
		NavMesh* m_NavMesh;
		PlayerCharacter* m_PlayerCharacter;
		AssetLoader* m_AssetLoader;

		std::vector<glm::vec3> m_AIGotoWaypoints;
		bool m_AIGotoWaypointsInProgress = false;
		unsigned int m_AICurrentWaypointIndex = 0;
		float aiWaypointLerp = 0;
		glm::quat aiFromRotation;
		float aiRotationLerp = .5f;
		float motionSpeed = 2.0f;
		float motionSpeedMulti = 1.0f;

		const Animation* m_WalkingAnimation;
		const Animation* m_IdleAnimation;
		float m_IdleAnimationFrame = 0;
		float m_WalkingAnimationFrame = 0;
		float m_AnimationPlaySpeedWalking = 0;
		float m_AnimationPlaySpeedIdle = 0;

		float interp = 0.0f;
		float targetinterp = 0.0f;

		double m_LastUpdateTime = 0;

		int m_NaviAgentID = -1;

		glm::vec3 m_NavDestionation;
		glm::vec3 test;
	};
}