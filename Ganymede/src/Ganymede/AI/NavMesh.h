#pragma once

#include "Ganymede/Core/Core.h"
// recast/detour stuff

#include <iostream>
#include <thread>
#include <mutex>
#include "glm/glm.hpp"

#include "Ganymede/World/World.h"
#include "Ganymede/ECS/Components/GCStaticMobility.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCIgnoreForNavMesh.h"

class rcContext;
class dtCrowd;
class dtNavMesh;
class dtNavMeshQuery;
struct rcConfig;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcPolyMeshDetail;


namespace Ganymede
{
	class World;

	// These are just sample areas to use consistent values across the samples.
	// The use should specify these base on his needs.

	// bzn most aren't used yet, just SAMPLE_POLYAREA_GROUND and SAMPLE_POLYFLAGS_WALK
	enum GANYMEDE_API SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};

	enum GANYMEDE_API SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_ALL = 0xffff		// All abilities.
	};

	enum GANYMEDE_API SamplePartitionType
	{
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

#define MAX_PATHSLOT		128
#define MAX_PATHVERT		512
#define MAX_PATHPOLY		256

	typedef GANYMEDE_API struct
	{
		float PosX[MAX_PATHVERT];
		float PosY[MAX_PATHVERT];
		float PosZ[MAX_PATHVERT];
		int MaxVertex;
		int Target;
	} PATHDATA;

	class GANYMEDE_API NavMesh
	{
	public:
		NavMesh();

		struct Agent
		{
			Agent()
			{
				Reset();
			}

			bool m_InUse = false;
			glm::vec3 m_Position;
			glm::vec3 m_Velocity;

			void Reset()
			{
				m_InUse = false;
				m_Position = glm::vec3(0);
				m_Velocity = glm::vec3(0);
			};
		};

		bool Generate(EntityView<Include<GCMesh, GCTransform, GCStaticMobility>, Exclude<GCIgnoreForNavMesh>> entityView);
		void CleanUp();

		int FindPath(const glm::vec3& pStartPos, const glm::vec3& pEndPos, int nPathSlot, int nTarget, std::vector<glm::vec3>& pathOut);

		void NavigateAgentToDestination(int agentID, glm::vec3 to);

		bool GetRandomPointOnNavMesh(glm::vec3& pointOut, glm::vec3 center, float distance);

		void UpdateCrowd(float deltaTime);

		// Returns -1 if no agent available. Otherwise the agent id which is 0 or bigger
		int TryRegisterAgent(const glm::vec3& startPosition);
		/*
		void SetAgentSpeed(int agentIndex, float agentSpeed)
		{
			//m_Crowd->updateAgentParameters
			//m_Crowd->getEditableAgent(agentIndex)->params.maxSpeed = glm::clamp(agentSpeed, 1.f,1.f);
		}
		*/

		bool TryUnregisterAgent(int agentID);

		std::vector<Agent> m_Agents;

		PATHDATA m_PathStore[MAX_PATHSLOT];

		static const int MAX_OFFMESH_CONNECTIONS = 256;
		float m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS * 3 * 2];
		float m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
		unsigned char m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
		unsigned char m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
		unsigned short m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
		unsigned int m_offMeshConId[MAX_OFFMESH_CONNECTIONS];
		int m_offMeshConCount;

		rcConfig* m_cfgPtr;
		rcHeightfield* m_solid;
		unsigned char* m_triareas;
		bool m_keepInterResults;
		rcCompactHeightfield* m_chf;
		rcContourSet* m_cset;
		rcPolyMesh* m_pmesh;
		rcPolyMeshDetail* m_dmesh;
		rcContext* m_ctx;

		dtCrowd* m_Crowd;

		//InputGeom* m_geom;
		dtNavMesh* m_navMesh;
		dtNavMeshQuery* m_navQuery;
		//dtCrowd* m_crowd;

		//float m_cellSize;
		//float m_cellHeight;
		float m_agentHeight;
		float m_agentRadius;
		float m_agentMaxClimb;
		float m_agentMaxSlope;
		float m_regionMinSize;
		float m_regionMergeSize;
		float m_edgeMaxLen;
		float m_edgeMaxError;
		float m_vertsPerPoly;
		float m_detailSampleDist;
		float m_detailSampleMaxError;
		int m_partitionType;

		bool m_filterLowHangingObstacles = true;
		bool m_filterLedgeSpans = true;
		bool m_filterWalkableLowHeightSpans = true;

		std::mutex m_Mutex;
	};
}