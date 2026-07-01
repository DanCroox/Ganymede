#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/Window.h"

struct GLFWwindow;

namespace Ganymede
{
	class EventSystem;

	class GANYMEDE_API VKGLFWWindow : public Window
	{
	public:
		VKGLFWWindow(EventSystem& eventSystem);

		~VKGLFWWindow();

		bool Initialize() override;

		void* GetNativeWindow() override;

		bool TryStart() override;
		void SetVSyncEnabled(bool isEnabled) override;
		bool IsVSyncEnabled() const override;

	private:
		void TerminateWindow();

		GLFWwindow* m_GLFWWindow = nullptr;
		bool m_IsVSyncEnabled = false;
	};
}
#endif //GM_PLATFORM_WINDOWS