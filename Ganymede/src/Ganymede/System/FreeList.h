#pragma once
#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API FreeList
	{
	public:
		size_t Append()
		{
			if (!m_NextFreeIndices.empty())
			{
				const size_t index = m_NextFreeIndices.back();
				m_NextFreeIndices.pop_back();
				return index;
			}

			return m_HighestIndex++;
		}

		void Free(size_t index)
		{
			// TODO: Currently possible to "free" same index multiple times.
			// In this case we will have a bug cause "Append" will hand-out indices which are in use.
			GM_CORE_ASSERT(index <= m_HighestIndex, "FreeList does not contain given index.");
			m_NextFreeIndices.push_back(index);
		}

	private:
		std::vector<size_t> m_NextFreeIndices;
		size_t m_HighestIndex = 0;
	};
}