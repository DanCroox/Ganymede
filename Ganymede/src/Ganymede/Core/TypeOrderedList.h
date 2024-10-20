#pragma once

#include <list> 
#include <unordered_map>


namespace Ganymede
{
	/// <summary>
	/// TypeOrderedList is a 1 dimensional double linked list that stores objects according to its types in a special sorted order. 
	/// Same types and derived types are stored in groups together, so that the most generic types are stored on top, followed by derivate types.
	/// The start and end iterator of these groups are stored in a map with the given unique ClassID as key.
	/// The TypeOrderedList allows to retrieve all objects of given type and/or all subtypes as a range of iterators. Lookups are fairly cheap, cause
	/// it only requires a map lookup by a ClassID (which is a size_t in retail builds). This will return a const ObjectTypeGroup& which contains
	/// the iterators for the requested block of types in the list.
	/// </summary>
	/// <typeparam name="DataEntryType">Type the container shall maintain</typeparam>
	template <typename DataEntryType>
	class TypeOrderedList
	{
	private:
		using DataContainerType = std::list<DataEntryType>;
	
		// ObjectTypeGroup defines the type section within the list Data Container.
		struct ObjectTypeGroup
		{
			using ObjectTypeElement = DataContainerType::iterator;

			ObjectTypeElement First;
			ObjectTypeElement LocalLast;
			ObjectTypeElement GlobalLast;
			bool HasInstances = false;
		};

		/// <summary>
		/// Use by TransformView to transform iteratiors from base type into search type so client does not have to do it explicitly during iteration.
		/// </summary>
		/// <typeparam name="Iterator"></typeparam>
		/// <typeparam name="ConversionType"></typeparam>
		template<typename Iterator, typename ConversionType>
		class StaticCastTransformIterator
		{
		public:
			StaticCastTransformIterator(Iterator iter)
				: current(iter) {}

			auto operator*() const
			{
				return static_cast<ConversionType>(*current);
			}

			StaticCastTransformIterator& operator++()
			{
				++current;
				return *this;
			}

			bool operator!=(const StaticCastTransformIterator& other) const
			{
				return current != other.current;
			}

		private:
			Iterator current;
		};

	public:
		using iterator = DataContainerType::iterator;
		using const_iterator = DataContainerType::const_iterator;

		/// <summary>
		/// Transforms the returned iterators into a view that transforms the base type (DataEntryType) into the search type, so client does not have to static_cast
		/// elements during iteration.
		/// </summary>
		/// <typeparam name="ConversionType"></typeparam>
		/// <typeparam name="IteratorType"></typeparam>
		template<typename ConversionType, typename IteratorType>
		class TransformView
		{
		public:
			using QualifiedDataContainerType = std::conditional_t<std::is_same<IteratorType, const_iterator>::value, const DataContainerType, DataContainerType>;
			using QualifiedIteratorType = std::conditional_t<std::is_same<IteratorType, const_iterator>::value, typename QualifiedDataContainerType::const_iterator, typename QualifiedDataContainerType::iterator>;

			TransformView(QualifiedDataContainerType& container)
				: m_Container(container), m_Begin(container.begin()), m_End(container.end()), m_Size(std::distance(container.begin(), container.end())) {}
			TransformView(QualifiedDataContainerType& container, QualifiedIteratorType begin, QualifiedIteratorType end)
				: m_Container(container), m_Begin(begin), m_End(end), m_Size(std::distance(begin, end)) {}

			// Implicit conversion from non-const iterator TransformView into const_iterator TransformView
			operator TransformView<ConversionType, typename DataContainerType::const_iterator>() const
			{
				return TransformView<ConversionType, typename DataContainerType::const_iterator>(m_Container, m_Begin, m_End);
			}

			inline auto begin() const { return StaticCastTransformIterator<QualifiedIteratorType, ConversionType>(m_Begin); }
			inline auto end() const { return StaticCastTransformIterator<QualifiedIteratorType, ConversionType>(m_End); }
			inline unsigned int size() const { return m_Size; }
			inline unsigned int empty() const { return m_Size == 0; }

		private:
			QualifiedDataContainerType& m_Container;
			QualifiedIteratorType m_Begin;
			QualifiedIteratorType m_End;
			unsigned int m_Size;

		};

		/// <summary>
		/// Add new element into the list. New element will be sorted into the correct postion.
		/// </summary>
		/// <param name="instance"></param>
		void Add(DataEntryType instance)
		{
			const ClassID templateClassID = instance->GetClassTypeInfo().GetClassID();

			bool putOnTop = false;
			typename ObjectTypeGroup::ObjectTypeElement newElement;
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
					const typename ObjectTypeGroup::ObjectTypeElement& insertLocation = putOnTop ? it->second.First : std::next(it->second.LocalLast);
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
				ObjectTypeGroup& section = it->second;
				typename ObjectTypeGroup::ObjectTypeElement& first = section.First;
				typename ObjectTypeGroup::ObjectTypeElement& localLast = section.LocalLast;
				typename ObjectTypeGroup::ObjectTypeElement& globalLast = section.GlobalLast;

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

				typename ObjectTypeGroup::ObjectTypeElement nextElementInList = std::next(newElement);
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
		template <typename SearchType, typename IteratorType>
		const TransformView<SearchType, IteratorType> BaseGet(bool includeChildren = true) const
		{
			using BaseSearchType = typename std::remove_cvref_t<std::remove_pointer_t<SearchType>>;
			using BaseDataEntryType = typename std::remove_cvref_t<std::remove_pointer_t<DataEntryType>>;
			static_assert(std::is_base_of<BaseDataEntryType, BaseSearchType>::value, "Search type is not base of list base type.");
			const ClassID classID = BaseSearchType::GetStaticClassTypeInfo().GetClassID();

			const auto sectionsIt = m_SectionsMapping.find(classID);
			if (sectionsIt == m_SectionsMapping.end())
			{
				return TransformView<SearchType, IteratorType>(m_Elements, m_Elements.begin(), m_Elements.end());
			}

			ObjectTypeGroup mapping = sectionsIt->second;
			return TransformView<SearchType, IteratorType>(m_Elements, mapping.First, includeChildren ? std::next(mapping.GlobalLast) : std::next(mapping.LocalLast));
		}

	public:
		/// <summary>
		/// Returns an iterator view into the searched object group.
		/// </summary>
		/// <typeparam name="SearchType">Unqualified type to search for.</typeparam>
		/// <typeparam name="IteratorType">Const or non const iterator selector (TypeOrderedList<T>::const_iterator or ::iterator)</typeparam>
		/// <param name="includeChildren">True (Default) = Returns search type objects and all children. False = Will only return object of exact search type.</param>
		/// <returns>View into the searched object type group in the list.</returns>
		template <typename SearchType, typename IteratorType>
		inline const TransformView<SearchType, IteratorType> Get(bool includeChildren = true) const { return BaseGet<SearchType, IteratorType>(includeChildren); }

		/// <summary>
		/// Returns an iterator view into the searched object group.
		/// </summary>
		/// <typeparam name="SearchType">Unqualified type to search for.</typeparam>
		/// <typeparam name="IteratorType">Const or non const iterator selector (TypeOrderedList<T>::const_iterator or ::iterator)</typeparam>
		/// <param name="includeChildren">True (Default) = Returns search type objects and all children. False = Will only return object of exact search type.</param>
		/// <returns>View into the searched object type group in the list.</returns>
		template <typename SearchType, typename IteratorType>
		inline const TransformView<SearchType, IteratorType> Get(bool includeChildren = true) { return BaseGet<SearchType, IteratorType>(includeChildren); }

		mutable DataContainerType m_Elements;
		std::unordered_map<ClassID, ObjectTypeGroup> m_SectionsMapping;
	};
}