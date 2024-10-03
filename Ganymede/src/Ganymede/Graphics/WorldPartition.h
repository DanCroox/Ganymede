#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include "glm/glm.hpp"
#include "Shader.h"
#include "ShaderManager.h"


class MeshWorldObjectInstance;
class FPSCamera;

struct WorldPartitionNodeID {
	glm::i32vec3 m_WorldLocation;
	glm::i32vec3 m_Dimension;
	unsigned int m_Level;

	bool operator==(const WorldPartitionNodeID& other) const {
		return m_WorldLocation == other.m_WorldLocation && m_Dimension == other.m_Dimension && m_Level == other.m_Level;
	}
};

namespace std {
	template <>
	struct hash<glm::i32vec3> {
		size_t operator()(const glm::i32vec3& k) const {
			return hash<int32_t>()(k.x) ^ hash<int32_t>()(k.y) ^ hash<int32_t>()(k.z);
		}
	};

	template <>
	struct hash<WorldPartitionNodeID> {
		size_t operator()(const WorldPartitionNodeID& k) const {
			return hash<glm::i32vec3>()(k.m_WorldLocation) ^ hash<unsigned int>()(k.m_Level);
		}
	};
}

class WorldPartitionNode
{
public:
	WorldPartitionNode(WorldPartitionNodeID id);
	~WorldPartitionNode();

	bool HasWorldPartitionNodeChild(WorldPartitionNodeID nodeID) const;
	void AddNodeChild(WorldPartitionNodeID nodeID, WorldPartitionNode* node);
	WorldPartitionNode* GetParentNode() const { return m_Parent; }
	const WorldPartitionNodeID& GetID() const { return m_ID; }
	void AddMeshWorldObjectInstance(const MeshWorldObjectInstance* mwoi) { m_MeshWorldObjectInstances.push_back(mwoi); }
	unsigned int GetOcclusionQueryID() const { return m_OcclusionQueryID; }

private:
	friend class WorldPartitionManager;

	void SetParentNode(WorldPartitionNode* parentNode) { m_Parent = parentNode; }
	std::unordered_map<WorldPartitionNodeID, WorldPartitionNode*> m_ChildNodes{};
	WorldPartitionNodeID m_ID;
	WorldPartitionNode* m_Parent = nullptr;
	std::vector<const MeshWorldObjectInstance*> m_MeshWorldObjectInstances{};
	unsigned int m_OcclusionQueryID = 0;
	bool m_IsVisible = false;
	unsigned int m_LastVisitedFrame = 0;
};

class WorldPartitionManager
{
public:
	WorldPartitionManager(glm::i32vec3 allMeshesBoundingBoxMin, glm::i32vec3 allMeshesBoundingBoxMax, ShaderManager& shaderManager);

	void Prepare(const FPSCamera& camera);
	void Finish();
	
	std::vector<const MeshWorldObjectInstance*> UpdateVisibility();
	void PullUpVisibility(WorldPartitionNode* node);
	void TraverseNode(WorldPartitionNode* node, std::vector<const MeshWorldObjectInstance*>& meshWorldObjectInstances);

	void AddWorldObjectInstance(const MeshWorldObjectInstance* mwoi);
	const WorldPartitionNode* GetRootNode() const { return m_RootNode; }
	WorldPartitionNode* FindWorldPartitionNodesByMeshWorldObjectInstance(const MeshWorldObjectInstance* mwoi) const;

	const WorldPartitionNode* m_DebugNode = nullptr;
private:
	void IssueOcclusionQuery(WorldPartitionNode* node);
	WorldPartitionNode& GetOrCreateWorldPartitionNode(const WorldPartitionNodeID& nodeID);
	WorldPartitionNodeID FindParentNodeWorldLocation(const WorldPartitionNodeID& nodeID) const;
	void SetRootNode(WorldPartitionNode* node) { m_RootNode = node; }
	bool IsInViewFrustum(const WorldPartitionNode* node);
	static bool IsPointInsideWorldPartitionNode(const glm::vec3& point, const WorldPartitionNode& node);

	const FPSCamera* m_Camera;
	const Shader* m_OcclusionQueryShader = nullptr;

	glm::i32vec3 m_AllMeshesBoundingBoxMin;
	glm::i32vec3 m_AllMeshesBoundingBoxMax;
	glm::u32vec3 m_AllMeshesBoundingDimension;
	WorldPartitionNode* m_RootNode = nullptr;
	std::unordered_map<WorldPartitionNodeID, WorldPartitionNode*> m_WorldPartitionNodes;
	std::unordered_map<const MeshWorldObjectInstance*, WorldPartitionNode*> m_MeshWorldObjectInstanceToWorldPartitionNodeLookup;
	std::unordered_map<WorldPartitionNode*, unsigned int> m_NodeVAOLookup;

	// Culling Data
	std::vector<WorldPartitionNode*> m_TraversalStack;
	std::vector<WorldPartitionNode*> m_QueryQueue;
};