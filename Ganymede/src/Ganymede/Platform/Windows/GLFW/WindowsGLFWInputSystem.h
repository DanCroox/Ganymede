#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/Windows/GLFW/GLFWInputSystem.h"

namespace Ganymede
{
	class WindowsGLFWInputSystem : public GLFWInputSystem
	{
	public:
		WindowsGLFWInputSystem(void* nativeWindow, EventSystem& eventSystem) : GLFWInputSystem(nativeWindow, eventSystem) {}

		bool Initialize() override;

		virtual bool IsNativeKeyPressed(int nativeKeyCode) override;
		virtual bool IsNativeMouseButtonPressed(int nativeButtonCode) override;
		virtual glm::vec2 GetNativeMousePosition() override;
	};
}

#endif //GM_PLATFORM_WINDOWS