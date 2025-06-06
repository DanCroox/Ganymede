#pragma once

#include <vector>

namespace Ganymede
{
	template <typename T>
	class Handle
	{
	public:
		Handle(size_t index) : m_Index(index) {}
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
		size_t m_Index;
	};
}