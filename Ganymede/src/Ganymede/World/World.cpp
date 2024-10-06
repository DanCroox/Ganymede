#include "World.h"
#include "Ganymede/System/Types.h"

#include "Ganymede/Data/AssetLoader.h"
#include "PointlightWorldObjectInstance.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include <algorithm>
#include "PointlightWorldObjectInstance.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/WorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/System/Thread.h"


namespace Ganymede
{
	World::World(AssetLoader& assetLoader) : m_AssetLoader(assetLoader) {};

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

	void World::Tick(double deltaTime)
	{
		const auto& worldObjectsInstances = GetAllWorldObjectInstances();
		if (m_TickThreadPool == nullptr || m_TickThreadPool->size() == 0)
		{
			for (std::pair<WorldObject::Type, std::vector<WorldObjectInstance*>> element : worldObjectsInstances)
			{
				for (WorldObjectInstance* instance : element.second)
				{
					instance->InternalTick(deltaTime);
				}
			}
		}
		else
		{
			for (std::pair<WorldObject::Type, std::vector<WorldObjectInstance*>> element : worldObjectsInstances)
			{
				const unsigned int numThreads = m_TickThreadPool->size();
				for (WorldObjectInstance* instance : element.second)
				{
					Thread* thread;
					int threadIndex = 0;
					while (true)
					{
						thread = &(*m_TickThreadPool)[threadIndex];
						if (!thread->IsRunning())
						{
							break;
						}
						threadIndex = (threadIndex + 1) % numThreads;
					}

					thread->m_WorkerFunction = [instance, deltaTime]()
						{
							instance->InternalTick(deltaTime);
						};
					thread->Start();
				}
			}
		}

		// Wait for all threads to finish
		if (m_TickThreadPool != nullptr)
		{
			std::array<Thread, 80>& threadPool = *m_TickThreadPool;
			for (const Thread& thread : threadPool)
			{
				while (thread.IsRunning());
			}
		}
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
		const WorldObject* worldObject = m_AssetLoader.GetWorldObjectByName(worldObjectName);

		if (worldObject == nullptr || worldObject->GetType() != objectType)
		{
			// No world object type with given name existent
			return nullptr;
		}

		return worldObject;
	}

	WorldObjectInstance* World::CreateWorldObjectInstance(const std::string& worldObjectName)
	{
		const WorldObject* worldObject = m_AssetLoader.GetWorldObjectByName(worldObjectName);

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

		if (worldObjectType == WorldObject::Type::Mesh || worldObjectType == WorldObject::Type::SkeletalMesh)
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

	const std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>>& World::GetAllWorldObjectInstances() const
	{ 
		return m_WorldObjectInstancesByType;
	}

	const std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>>& World::GetMeshInstances() const
	{
		return m_MeshesInstances;
	}

}