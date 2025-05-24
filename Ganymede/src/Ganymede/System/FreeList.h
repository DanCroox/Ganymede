#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	/*
	template <typename T>
	class IFreeListContainer
	{
	public:
		virtual ~IFreeListContainer() = default;
		virtual void Add(T* element, size_t elementOffset) = 0;
		virtual void DeleteElement(size_t elementIndex) = 0;
		virtual const T& GetElement(size_t elementIndex) = 0;
		virtual size_t GetSize() = 0;
	};

	template <template<typename> class ContainerType, typename DataType>
	class FreeList
	{
	public:
		template <typename... Args>
		explicit FreeList(Args&&... args)
			: m_Container(std::forward<Args>(args)...)
			, m_NumElements(m_Container.GetSize())
		{
		}

		void Add(DataType& element)
		{
			m_Container.Add(&element, m_NumElements);
			++m_NumElements;
		}

		void Remove(unsigned int elementIndex)
		{
			const size_t lastElementIndex = m_NumElements - 1;
			m_Container.Add(m_Container.GetElement(lastElementIndex), elementIndex);
			m_Container.DeleteElement(lastElementIndex);
			--m_NumElements;
		}

	private:
		ContainerType m_Container;
		size_t m_NumElements = 0;
	};
	*/
}