#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API GPUDebugHandler
	{
	public:
		static void Enable();
		static void Disable();

		static inline bool IsEnabled() { return m_IsEnabled; }

	private:
		static bool m_IsEnabled;
	};
}