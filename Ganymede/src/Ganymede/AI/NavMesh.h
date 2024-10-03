#pragma once

#include "Ganymede/Core/Core.h"
// recast/detour stuff

#include <iostream>
#include <thread>
#include <mutex>
#include "glm/glm.hpp"

class MeshWorldObjectInstance;
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


// These are just sample areas to use consistent values across the samples.
// The use should specify these base on his needs.

// bzn most aren't used yet, just SAMPLE_POLYAREA_GROUND and SAMPLE_POLYFLAGS_WALK
enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};

enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_ALL = 0xffff		// All abilities.
};

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

#define MAX_PATHSLOT		128
#define MAX_PATHVERT		512
#define MAX_PATHPOLY		256

typedef struct
{
	float PosX[MAX_PATHVERT];
	float PosY[MAX_PATHVERT];
	float PosZ[MAX_PATHVERT];
	int MaxVertex;
	int Target;
} PATHDATA;

/*
class DistancePointFilter : public dtQueryFilter
{
public:
	DistancePointFilter(glm::vec3 center, const float distance)
		: m_center(center), m_distance(distance)
	{
	}

	bool passFilter(const dtPolyRef ref,
		const dtMeshTile* tile,
		const dtPoly* poly) const override
	{
		// Reject polygons outside the search area.
		glm::vec3 average;
		for (int i = 0; i < poly->vertCount; ++i)
		{
			unsigned short index = poly->verts[i * 3];
			average += glm::vec3(tile->verts[index],
				tile->verts[index+1],
				tile->verts[index+2]);
		}
		average /= poly->vertCount;

		const float distSq = glm::distance(average, m_center);
		if (distSq > m_distance)
			return false;

		// Accept all other polygons.
		return true;
	}

private:
	const glm::vec3 m_center;
	const float m_distance;
};
*/

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

	bool Generate(const std::vector<MeshWorldObjectInstance*> instances);
	void CleanUp();

	int FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget, std::vector<glm::vec3>& pathOut);

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

	rcConfig* m_cfg;
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