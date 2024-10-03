#pragma once
#include "Ganymede/World/WorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/WorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <typeinfo> 

class MeshWorldObjectInstance;

class World
{
public:
	World() = default;
	~World();

	void AddToWorld(WorldObjectInstance* instance);

	WorldObjectInstance* CreateWorldObjectInstance(const std::string& worldObjectName);

	const WorldObject* FindWorldObjectByName(WorldObject::Type objectType, const std::string& worldObjectName);
	// TODO: Create "DestroyWorldObjectInstance"

	template<class T>
	T* CreateEmptyWorldObjectInstance();

	template<class T>
	const std::vector<T*>* GetWorldObjectInstancesByType() const;

	const std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>>& GetAllWorldObjectInstances() { return m_WorldObjectInstancesByType; }

	const std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>>& GetMeshInstances() const { return m_MeshesInstances; }

private:
	std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>> m_WorldObjectInstancesByType;
	std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>> m_MeshesInstances;
};