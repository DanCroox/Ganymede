#pragma once

#include <unordered_map>
#include <list> 

namespace Ganymede
{
	template <typename T>
	struct TypeListSection
	{
		typedef typename std::list<T>::iterator TypeListElement;

		TypeListElement First;
		TypeListElement LocalLast;
		TypeListElement GlobalLast;
		bool HasInstances = false;
	};

	template <typename T>
	class TypeOrderedList
	{
	public:
		template <typename SearchType>
		TypeListSection<T> Get()
		{
			const ClassID classID = SearchType::GetStaticClassTypeInfo().GetClassID();

			const auto sectionsIt = m_SectionsMapping.find(classID);
			if (sectionsIt == m_SectionsMapping.end())
			{
				//static TypeListSection<T> empty = { m_SectionsMapping.begin(), m_SectionsMapping.end(), m_SectionsMapping.end(), false };
				return TypeListSection<T>();
			}

			return sectionsIt->second;
		}

		void Add(T instance)
		{
			const ClassID templateClassID = instance->GetClassTypeInfo().GetClassID();

			bool putOnTop = false;
			typename TypeListSection<T>::TypeListElement newElement;
			if (m_SectionsMapping.empty())
			{
				// Empty instances array, just insert
				newElement = m_Elements.insert(m_Elements.begin(), instance);
			}
			else
			{
				// Find instance insert location 
				const ClassTypeInfo* typeInfo = &instance->GetClassTypeInfo();
				while (typeInfo->GetClassID().IsValid())
				{
					const ClassID classID = typeInfo->GetClassID();
					typeInfo = typeInfo->GetParentClassInfoType();
					auto it = m_SectionsMapping.find(classID);
					if (it == m_SectionsMapping.end())
					{
						continue;
					}
					const bool isSubClassOrEqual = instance->GetClassTypeInfo().IsSubClassOf((*it->second.First)->GetClassTypeInfo());
					putOnTop = (!isSubClassOrEqual) || templateClassID == classID;
					const typename TypeListSection<T>::TypeListElement& insertLocation = putOnTop ? it->second.First : std::next(it->second.LocalLast);
					newElement = m_Elements.insert(insertLocation, instance);
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

				auto [it, inserted] = m_SectionsMapping.try_emplace(classID);
				TypeListSection<T>& section = it->second;
				typename TypeListSection<T>::TypeListElement& first = section.First;
				typename TypeListSection<T>::TypeListElement& localLast = section.LocalLast;
				typename TypeListSection<T>::TypeListElement& globalLast = section.GlobalLast;

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

				typename TypeListSection<T>::TypeListElement nextElementInList = std::next(newElement);
				if (nextElementInList == m_Elements.end())
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
		std::list<T> m_Elements;
		std::unordered_map<ClassID, TypeListSection<T>> m_SectionsMapping;
	};
}