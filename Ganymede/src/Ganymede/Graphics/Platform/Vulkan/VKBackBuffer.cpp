#include "VKBackBuffer.h"
#include "VKContext.h"

namespace Ganymede
{
	uint32_t VKBackBufferHelper::GetFiFIndex()
	{
		return VKContext::GetInstance().m_FiFIndex;
	}

	uint32_t VKBackBufferHelper::GetSCIndex()
	{
		return VKContext::GetInstance().m_SCIndex;
	}
}