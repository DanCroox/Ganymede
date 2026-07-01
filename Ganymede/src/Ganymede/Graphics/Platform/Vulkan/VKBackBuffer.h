#pragma once
#include <array>
#include <volk.h>

namespace Ganymede
{
	namespace VKBackBufferHelper
	{
		uint32_t GetFiFIndex();
		uint32_t GetSCIndex();
	}

	template<typename T>
	using VKBackBuffer = std::array<T, 5>;

	template<typename T>
	T g_VKFiFIndex(const VKBackBuffer<T>& backBuffer)
	{
		return backBuffer[VKBackBufferHelper::GetFiFIndex()];
	}

	template<typename T>
	T g_VKGetSCIndex(const VKBackBuffer<T>& backBuffer)
	{
		return backBuffer[VKBackBufferHelper::GetSCIndex()];
	}

	template<typename T>
	void g_VKClearBackBuffer(VKBackBuffer<T>& backBuffer)
	{
		for (uint32_t i = 0; i < backBuffer.size(); ++i)
		{
			backBuffer[i] = VK_NULL_HANDLE;
		}
	}
}