#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/System/Thread.h"
#include "Ganymede/World/Common/Types.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include <array>
#include <string>


namespace Ganymede
{
	class AssetLoader;
	class MeshWorldObject;
	class WorldObjectInstance;
	class PointlightWorldObjectInstance;

	class GANYMEDE_API World
	{
	public:
		World(AssetLoader& assetLoader);
		~World();

		void Tick(double deltaTime);

		WorldObjectInstance* CreateWorldObjectInstance(const std::string& worldObjectName);

		template <typename T>
		void AddWorldObjectInstance(T* instance)
		{
			m_WorldObjectInstances.Add(instance);
		}

		template <typename T>
		ConstListSlice<T*> GetWorldObjectInstances() const
		{
			return m_WorldObjectInstances.Get<T*, WorldObjectInstanceList::const_iterator>();
		}

		template <typename T>
		ListSlice<T*> GetWorldObjectInstances()
		{
			return m_WorldObjectInstances.Get<T*, WorldObjectInstanceList::iterator>();
		}

	private:
		AssetLoader& m_AssetLoader;
		std::array<Thread, 20> m_TickThreadPool;
		WorldObjectInstanceList m_WorldObjectInstances;
	};
}