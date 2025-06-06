#pragma once
#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>

namespace Ganymede
{
	class World;
	class PhysicsWorld;
	class NavMesh;
	class PlayerCharacter;
	class AssetLoader;
	class Animation;

	struct GCCreature
	{
		enum AIState
		{
			Patroling,
			Searching,
			Attacking,
		};

		GCCreature() = delete;
		
		GCCreature(
			World& world,
			PhysicsWorld& physicsWorld,
			NavMesh& navMesh,
			PlayerCharacter& playerCharacter,
			AssetLoader& assetLoader) :
			m_World(world),
			m_PhysicsWorld(physicsWorld),
			m_NavMesh(navMesh),
			m_PlayerCharacter(playerCharacter),
			m_AssetLoader(assetLoader)
		{}

		AIState m_AIState = AIState::Patroling;

		World& m_World;
		PhysicsWorld& m_PhysicsWorld;
		NavMesh& m_NavMesh;
		PlayerCharacter& m_PlayerCharacter;
		AssetLoader& m_AssetLoader;

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