#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/World/WorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObjectInstance.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <typeinfo> 
#include <list> 
#include "Ganymede/World/Common/Types.h"

namespace Ganymede
{
	class Thread;
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

		AssetLoader& m_AssetLoader;

		void AddToWorld(WorldObjectInstance* instance);

		WorldObjectInstance* CreateWorldObjectInstance(const std::string& worldObjectName);

		const WorldObject* FindWorldObjectByName(WorldObject::Type objectType, const std::string& worldObjectName);
		// TODO: Create "DestroyWorldObjectInstance"

		template<class T>
		T* CreateEmptyWorldObjectInstance();

		template<class T>
		const std::vector<T*>* GetWorldObjectInstancesByType() const = delete;

		template<>
		inline const std::vector<MeshWorldObjectInstance*>* GetWorldObjectInstancesByType() const
		{
			auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::Mesh);
			auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
			if (instances == nullptr)
				return nullptr;

			return reinterpret_cast<const std::vector<MeshWorldObjectInstance*>*>(instances);
		}

		template<>
		inline const std::vector<SkeletalMeshWorldObjectInstance*>* GetWorldObjectInstancesByType() const
		{
			auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::SkeletalMesh);
			auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
			if (instances == nullptr)
				return nullptr;

			return reinterpret_cast<const std::vector<SkeletalMeshWorldObjectInstance*>*>(instances);
		}

		template<>
		inline const std::vector<PointlightWorldObjectInstance*>* GetWorldObjectInstancesByType() const
		{
			auto it = m_WorldObjectInstancesByType.find(WorldObject::Type::PointLight);
			auto instances = it != m_WorldObjectInstancesByType.end() ? &it->second : nullptr;
			if (instances == nullptr)
				return nullptr;

			return reinterpret_cast<const std::vector<PointlightWorldObjectInstance*>*>(instances);
		}

		template <typename T>
		ConstListSlice<T*> GetWorldObjectInstances() const
		{
			return m_WorldObjectInstancesSections.Get<T*, WorldObjectInstanceList::const_iterator>();
		}

		template <typename T>
		ListSlice<T*> GetWorldObjectInstances()
		{
			return m_WorldObjectInstancesSections.Get<T*, WorldObjectInstanceList::iterator>();
		}

		template <typename T>
		void AddWorldObjectInstance(T* instance)
		{
			m_WorldObjectInstancesSections.Add(instance);
		}

	private:
		typedef std::array<Thread, 80> TickThreadPool;

		std::unique_ptr<TickThreadPool> m_TickThreadPool = nullptr;
		std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>> m_WorldObjectInstancesByType;
		std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>> m_MeshesInstances;

		WorldObjectInstanceList m_WorldObjectInstancesSections;
	};
}