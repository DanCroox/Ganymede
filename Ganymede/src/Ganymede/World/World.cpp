#include "World.h"
#include "Ganymede/System/Types.h"

#include "Ganymede/Data/AssetLoader.h"
#include "MeshWorldObjectInstance.h"
#include "PointlightWorldObjectInstance.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include <algorithm>

World::~World()
{
	for (std::pair<WorldObject::Type, std::vector<WorldObjectInstance*>> element : m_WorldObjectInstancesByType)
	{
		for (WorldObjectInstance* instance : element.second)
		{
			delete instance;
		}
	}

	m_WorldObjectInstancesByType.clear();
}

void World::AddToWorld(WorldObjectInstance* instance)
{
	// TODO: Currently only isntances witha meshworld object are allowed... theoretically we could create them without a world object - but implement once needed
	if (instance == nullptr)
	{
		return;
	}

	const WorldObject* worldObject = instance->GetWorldObject();

	const WorldObject::Type worldObjectType = worldObject->GetType();

	if (m_WorldObjectInstancesByType.find(worldObjectType) == m_WorldObjectInstancesByType.end())
	{
		m_WorldObjectInstancesByType.insert(std::make_pair(worldObjectType, std::vector<WorldObjectInstance*>()));
	}

	std::vector<WorldObjectInstance*>& worldObjectTypes = m_WorldObjectInstancesByType[worldObjectType];

	if (worldObjectType == WorldObject::Type::Mesh || worldObjectType == WorldObject::Type::SkeletalMesh)
	{
		const MeshWorldObject* object = static_cast<const MeshWorldObject*>(instance->GetWorldObject());

		instance->SetVisible(true);

		if (m_MeshesInstances.find(object) == m_MeshesInstances.end())
		{
			m_MeshesInstances[object] = std::vector<MeshWorldObjectInstance*>();
		}

		std::vector<MeshWorldObjectInstance*>& meshInstances = m_MeshesInstances[object];
		meshInstances.push_back(static_cast<MeshWorldObjectInstance*>(instance));

		worldObjectTypes.push_back(instance);
		return;
	}

	if (worldObjectType == WorldObject::Type::PointLight)
	{
		const PointlightWorldObject* object = static_cast<const PointlightWorldObject*>(worldObject);
		PointlightWorldObjectInstance* woInstance = new PointlightWorldObjectInstance(object);
		woInstance->SetVisible(true);

		worldObjectTypes.push_back(woInstance);

		return;
	}

	// unknown type
	ASSERT("Unkown world object type!");
}

const WorldObject* World::FindWorldObjectByName(WorldObject::Type objectType, const std::string& worldObjectName)
{
	AssetLoader& assetLoader = AssetLoader::GetInstance();
	const WorldObject* worldObject = assetLoader.GetWorldObjectByName(worldObjectName);

	if (worldObject == nullptr || worldObject->GetType() != objectType)
	{
		// No world object type with given name existent
		return nullptr;
	}

	return worldObject;
}

WorldObjectInstance* World::CreateWorldObjectInstance(const std::string& worldObjectName)
{
	AssetLoader& assetLoader = AssetLoader::GetInstance();

	const WorldObject* worldObject = assetLoader.GetWorldObjectByName(worldObjectName);

	if (worldObject == nullptr)
	{
		// No world object with given name existent
		return nullptr;
	}

	const WorldObject::Type worldObjectType = worldObject->GetType();
	if (m_WorldObjectInstancesByType.find(worldObjectType) == m_WorldObjectInstancesByType.end())
	{
		m_WorldObjectInstancesByType.insert(std::make_pair(worldObjectType, std::vector<WorldObjectInstance*>()));
	}

	std::vector<WorldObjectInstance*>& worldObjectTypes = m_WorldObjectInstancesByType[worldObjectType];

	if (worldObjectType == WorldObject::Type::Mesh)
	{
		const MeshWorldObject* object = static_cast<const MeshWorldObject*>(worldObject);

		MeshWorldObjectInstance* woInstance = new MeshWorldObjectInstance(object);
		woInstance->SetVisible(true);

		if (m_MeshesInstances.find(object) == m_MeshesInstances.end())
		{
			m_MeshesInstances[object] = std::vector<MeshWorldObjectInstance*>();
		}

		m_MeshesInstances[object].push_back(woInstance);

		worldObjectTypes.push_back(woInstance);
		return woInstance;
	}
	
	if (worldObjectType == WorldObject::Type::PointLight)
	{
		const PointlightWorldObject* object = static_cast<const PointlightWorldObject*>(worldObject);
		PointlightWorldObjectInstance* woInstance = new PointlightWorldObjectInstance(object);
		woInstance->SetVisible(true);

		worldObjectTypes.push_back(woInstance);
		return woInstance;
	}
	
	// Type unknown or None!
	return nullptr;
}

template<>
PointlightWorldObjectInstance* World::CreateEmptyWorldObjectInstance()
{
	const WorldObject::Type worldObjectType = WorldObject::Type::PointLight;
	if (m_WorldObjectInstancesByType.find(worldObjectType) == m_WorldObjectInstancesByType.end())
	{
		m_WorldObjectInstancesByType.insert(std::make_pair(worldObjectType, std::vector<WorldObjectInstance*>()));
	}

	std::vector<WorldObjectInstance*>& worldObjectTypes = m_WorldObjectInstancesByType[worldObjectType];
	PointlightWorldObjectInstance* woInstance = new PointlightWorldObjectInstance(nullptr);

	worldObjectTypes.push_back(woInstance);
	return woInstance;
}

template<>
const std::vector<MeshWorldObjectInstance*>* World::GetWorldObjectInstancesByType() const
{
	auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::Mesh);
	auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
	if (instances == nullptr)
		return nullptr;

	return reinterpret_cast<const std::vector<MeshWorldObjectInstance*>*>(instances);
}

template<>
const std::vector<SkeletalMeshWorldObjectInstance*>* World::GetWorldObjectInstancesByType() const
{
	auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::SkeletalMesh);
	auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
	if (instances == nullptr)
		return nullptr;

	return reinterpret_cast<const std::vector<SkeletalMeshWorldObjectInstance*>*>(instances);
}

template<>
const std::vector<PointlightWorldObjectInstance*>* World::GetWorldObjectInstancesByType() const
{
	auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::PointLight);
	auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
	if (instances == nullptr)
		return nullptr;

	return reinterpret_cast<const std::vector<PointlightWorldObjectInstance*>*>(instances);
}
