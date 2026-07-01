#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API GPUDebugHandler
	{
	public:
		virtual void Enable() = 0;
		virtual void Disable() = 0;
		virtual bool IsEnabled() const = 0;
	
	protected:
		GPUDebugHandler() = default;
	};
}