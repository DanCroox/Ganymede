#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Data/SerializerTraits.h"
#include <vector>

namespace Ganymede
{
	template <typename T>
	class GANYMEDE_API Handle
	{
	public:
		Handle(size_t index);
		const T& GetData() const;

		bool operator==(const Handle<T>& other) const
		{
			return m_Index == other.m_Index;
		}

		bool operator!=(const Handle<T>& other) const
		{
			return m_Index != other.m_Index;
		}

		size_t GetID() const { return m_Index; };

	private:
		GM_SERIALIZABLE_TEMPLATED(Handle<T>, typename T);
		Handle() = default;

		size_t m_Index;
#ifndef RETAIL
		T* m_Data;
#endif //RETAIL
	};
}