#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/Window.h"

struct GLFWwindow;

namespace Ganymede
{
	class EventSystem;

	class GANYMEDE_API OGLGLFWWindow : public Window
	{
	public:
		OGLGLFWWindow(EventSystem& eventSystem) : Window(eventSystem) {}

		~OGLGLFWWindow();

		bool Initialize() override;

		void* GetNativeWindow() override;

		bool TryStart() override;
		void SetVSyncEnabled(bool isEnabled) override;
		bool IsVSyncEnabled() const override;

	private:
		void DrawStats(double deltaTime, double gameTime);
		void TerminateWindow();

		GLFWwindow* m_GLFWWindow = nullptr;
		bool m_IsInitialized = false;
		bool m_IsVSyncEnabled = false;
	};
}
#endif //GM_PLATFORM_WINDOWS