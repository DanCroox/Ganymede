#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class EventSystem;

	class GANYMEDE_API Window
	{
	public:
		virtual ~Window() = default;

		/// <summary>
		/// Creates the native window.
		/// </summary>
		/// <returns>Creating succeeded</returns>
		virtual bool Initialize() = 0;

		virtual void* GetNativeWindow() = 0;

		virtual bool TryStart() = 0;
		virtual void SetVSyncEnabled(bool isEnabled) = 0;
		virtual bool IsVSyncEnabled() const = 0;

		// Needs to be implemented per Platform
		static Window* Create(EventSystem& eventSystem);

	protected:
		Window() = delete;
		Window(EventSystem& eventSystem);

		EventSystem& m_EventSystem;
	};
}