#include "World.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Data/AssetLoader.h"
#include "Ganymede/System/Types.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include "Ganymede/World/WorldObjectInstance.h"
#include "PointlightWorldObjectInstance.h"


namespace Ganymede
{
	World::World(AssetLoader& assetLoader) : m_AssetLoader(assetLoader) {}

	World::~World()
	{
		auto instances = GetWorldObjectInstances<WorldObjectInstance>();
		for (auto instance : instances)
		{
			delete instance;
		}
	}

	void World::Tick(double deltaTime)
	{
		SCOPED_TIMER("World Tick");
		auto worldObjectsInstances = GetWorldObjectInstances<WorldObjectInstance>();
		if (m_TickThreadPool.size() == 0)
		{
			for (WorldObjectInstance* woi : worldObjectsInstances)
			{
				woi->InternalTick(deltaTime);
			}
		}
		else
		{
			const unsigned int numThreads = m_TickThreadPool.size();
			for (WorldObjectInstance* instance : worldObjectsInstances)
			{
				Thread* thread;
				int threadIndex = 0;
				while (true)
				{
					thread = &(m_TickThreadPool[threadIndex]);
					if (!thread->IsRunning())
					{
						break;
					}
					// Check if next thread is not busy
					threadIndex = (threadIndex + 1) % numThreads;
				}

				thread->m_WorkerFunction = [instance, deltaTime]()
					{
						instance->InternalTick(deltaTime);
					};
				thread->Start();
			}
		}

		// Wait for all threads to finish

		for (Thread& thread : m_TickThreadPool)
		{
			thread.Join();
		}
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

		if (worldObjectType == WorldObject::Type::Mesh || worldObjectType == WorldObject::Type::SkeletalMesh)
		{
			const MeshWorldObject* object = static_cast<const MeshWorldObject*>(worldObject);

			MeshWorldObjectInstance* woInstance = new MeshWorldObjectInstance(object);
			woInstance->SetVisible(true);

			return woInstance;
		}
		else if (worldObjectType == WorldObject::Type::PointLight)
		{
			const PointlightWorldObject* object = static_cast<const PointlightWorldObject*>(worldObject);
			PointlightWorldObjectInstance* woInstance = new PointlightWorldObjectInstance(object);
			woInstance->SetVisible(true);

			return woInstance;
		}

		// Type unknown or None!
		return nullptr;
	}
}