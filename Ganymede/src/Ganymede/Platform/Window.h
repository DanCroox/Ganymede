#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class GANYMEDE_API Window
	{
	public:
		virtual ~Window() = default;

		virtual void* GetNativeWindow() = 0;

		virtual bool TryStart() = 0;
		virtual void SetVSyncEnabled(bool isEnabled) = 0;
		virtual bool IsVSyncEnabled() const = 0;

		// Needs to be implemented per Platform
		static Window* Create();

	protected:
		Window() = default;
	};
}