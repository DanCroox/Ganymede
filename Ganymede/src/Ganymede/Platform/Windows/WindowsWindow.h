#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Core/Core.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Platform/Window.h"

struct GLFWwindow;

namespace Ganymede
{
	class GANYMEDE_API WindowsWindow : public Window
	{
	public:

		WindowsWindow() = default;
		virtual ~WindowsWindow();

		void* GetNativeWindow() override;

		bool TryStart() override;
		void SetVSyncEnabled(bool isEnabled) override;
		bool IsVSyncEnabled() const override;

	private:
		GLFWwindow* m_GLFWWindow = nullptr;
		bool m_IsVSyncEnabled = false;
	};
}
#endif //GM_PLATFORM_WINDOWS