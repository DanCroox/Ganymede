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

namespace Ganymede
{
	class Thread;
	class AssetLoader;
	class MeshWorldObject;
	class WorldObjectInstance;
	class PointlightWorldObjectInstance;

	typedef typename std::list<const WorldObjectInstance*>::iterator ListElement;

	struct ListSection
	{
		ListElement First;
		ListElement LocalLast;
		ListElement GlobalLast;
		bool HasInstances = false;
	};

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

		const std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>>& GetAllWorldObjectInstances() const;
		const std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>>& GetMeshInstances() const;

		template <typename T>
		bool TryGetWorldObjectInstances(ListSection& sliceOut)
		{
			static_assert(std::is_base_of<WorldObjectInstance, T>::value, "You can only access instances of type WorldObjectInstance");
			const ClassID classID = T::GetStaticClassTypeInfo().GetClassID();

			const auto sectionsIt = m_WorldObjectInstancesSections.find(classID);
			if (sectionsIt == m_WorldObjectInstancesSections.end())
			{
				// No sections with given type found
				return false;
			}

			sliceOut = sectionsIt->second;
			return true;
		}

		template <typename T>
		void AddWorldObjectInstance(const T* instance)
		{
			static_assert(std::is_base_of<WorldObjectInstance, T>::value, "You can only add instances of type WorldObjectInstance");
			const ClassID templateClassID = T::GetStaticClassTypeInfo().GetClassID();

			bool putOnTop = false;
			ListElement newElement;
			if (m_WorldObjectInstancesSections.empty())
			{
				// Empty instances array, just insert
				newElement = m_WorldObjectInstances.insert(m_WorldObjectInstances.begin(), instance);
			}
			else
			{
				// Find instance insert location 
				const ClassTypeInfo* typeInfo = &T::GetStaticClassTypeInfo();
				while (typeInfo->GetClassID().IsValid())
				{
					const ClassID classID = typeInfo->GetClassID();
					typeInfo = typeInfo->GetParentClassInfoType();
					auto it = m_WorldObjectInstancesSections.find(classID);
					if (it == m_WorldObjectInstancesSections.end())
					{
						continue;
					}
					const bool isSubClassOrEqual = T::GetStaticClassTypeInfo().IsSubClassOf((*it->second.First)->GetClassTypeInfo());
					putOnTop = (!isSubClassOrEqual) || templateClassID == classID;
					const ListElement& insertLocation = putOnTop ? it->second.First : std::next(it->second.LocalLast);
					newElement = m_WorldObjectInstances.insert(insertLocation, instance);
					break;
				}
			}

			// Upate Section references
			const ClassTypeInfo* typeInfo = &instance->GetClassTypeInfo();
			while (typeInfo->GetClassID().IsValid())
			{
				const ClassTypeInfo* currentClassTypeInfo = typeInfo;
				const ClassID classID = currentClassTypeInfo->GetClassID();
				typeInfo = typeInfo->GetParentClassInfoType();

				auto [it, inserted] = m_WorldObjectInstancesSections.try_emplace(classID);
				ListSection& section = it->second;
				ListElement& first = section.First;
				ListElement& localLast = section.LocalLast;
				ListElement& globalLast = section.GlobalLast;

				if (templateClassID == classID)
				{
					section.HasInstances = true;
				}

				if (inserted)
				{
					first = newElement;
					localLast = newElement;
					globalLast = newElement;
					continue;
				}

				if (putOnTop)
				{
					if (templateClassID != classID && section.HasInstances)
					{
						break;
					}
					first = newElement;
					continue;
				}

				ListElement nextElementInList = std::next(newElement);
				if (nextElementInList == m_WorldObjectInstances.end())
				{
					globalLast = newElement;
					return;
				}

				const ClassTypeInfo* nextElementClassTypeInfo = &(*nextElementInList)->GetClassTypeInfo();
				if (!nextElementClassTypeInfo->IsSubClassOf(*currentClassTypeInfo))
				{
					globalLast = newElement;
				}
			}
		}

	private:
		typedef std::array<Thread, 80> TickThreadPool;

		std::unique_ptr<TickThreadPool> m_TickThreadPool = nullptr;
		std::unordered_map<WorldObject::Type, std::vector<WorldObjectInstance*>> m_WorldObjectInstancesByType;
		std::unordered_map<const MeshWorldObject*, std::vector<MeshWorldObjectInstance*>> m_MeshesInstances;

		std::list<const WorldObjectInstance*> m_WorldObjectInstances;
		std::unordered_map<ClassID, ListSection> m_WorldObjectInstancesSections;
	};
}