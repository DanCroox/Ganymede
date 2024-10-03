#pragma once
#include "SkeletalMeshWorldObjectInstance.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Data/AssetLoader.h"
#include "Ganymede/AI/NavMesh.h"
#include "Ganymede/Player/PlayerCharacter.h"
#include <iostream>

class CreatureMeshWorldObjectInstance : public SkeletalMeshWorldObjectInstance
{
public:
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
		World& world) : SkeletalMeshWorldObjectInstance(meshWorldObject)
	{
		m_NavMesh = &navMesh;
		m_PlayerCharacter = &playerCharacter;
		m_PhysicsWorld = &physicsWorld;
		m_World = &world;
	}

	void OnCreate() override
	{
		if (std::strcmp(GetWorldObject()->GetName().c_str(), "Matschkopf") == 0)
		{
			AssetLoader& assetLoader = AssetLoader::GetInstance();
			m_IdleAnimation = assetLoader.GetAnimationByName(std::string("Idle"));
			m_WalkingAnimation = assetLoader.GetAnimationByName(std::string("Walking"));
			motionSpeed = Helpers::Random::RandomFloatInRange(2.2f, 2.2f);
			SetScale(Helpers::Random::RandomFloatInRange(.75f, .85f));
		}

		if (std::strcmp(GetWorldObject()->GetName().c_str(), "Object_6") == 0)
		{
			AssetLoader& assetLoader = AssetLoader::GetInstance();
			m_WalkingAnimation = assetLoader.GetAnimationByName(std::string("mixamo.com"));
			m_AnimationPlaySpeedWalking = 2.f;
			m_AnimationPlaySpeedIdle = 2.f;
			motionSpeed = Helpers::Random::RandomFloatInRange(.25f, .85f);
			motionSpeed = .05f;
			//SetScale(Helpers::Random::RandomFloatInRange(1.3f, 1.5f));
		}

		m_WalkingAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);
		m_IdleAnimationFrame = Helpers::Random::RandomFloatInRange(0, 50);

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

		SetPosition(x, GetPosition().y, z);

		m_NaviAgentID = m_NavMesh->TryRegisterAgent(GetPosition());


		TryGoto(m_PlayerCharacter->GetPosition());
	}

	void Tick(float deltaTime) override;

	void UpdateMotion(float deltaTime);
	void UpdateAnimation(float deltaTime);
	bool TryGoto(glm::vec3 destination);

	AIState aiState = AIState::Patroling;

	World* m_World;
	PhysicsWorld* m_PhysicsWorld;
	NavMesh* m_NavMesh;
	PlayerCharacter* m_PlayerCharacter;

	std::vector<glm::vec3> aiGotoWaypoints;
	bool aiGotoWaypointsInProgress = false;
	unsigned int aiCurrentWaypointIndex = 0;
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

	float interp;
	float targetinterp;

	double m_LastUpdateTime = 0;

	int m_NaviAgentID = -1;

	glm::vec3 m_NavDestionation;
};

