#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API FreeList {
	public:
		[[nodiscard]] size_t Append()
		{
			if (!m_FreeIndices.empty())
			{
				size_t index = m_FreeIndices.back();
				m_FreeIndices.pop_back();
				return index;
			}

			return m_HighestIndex++;
		}

		void Free(size_t index)
		{
			GM_CORE_ASSERT(index < m_HighestIndex, "Invalid index");
			GM_CORE_ASSERT(std::find(m_FreeIndices.begin(), m_FreeIndices.end(), index) == m_FreeIndices.end(), "Double freeing index. This should never happen!");
			m_FreeIndices.push_back(index);
		}

		void Clear()
		{
			m_FreeIndices.clear();
			m_HighestIndex = 0;
		}

	private:
		std::vector<size_t> m_FreeIndices;
		size_t m_HighestIndex = 0;
	};
}