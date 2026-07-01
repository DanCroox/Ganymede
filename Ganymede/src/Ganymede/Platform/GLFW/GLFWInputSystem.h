#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/InputSystem.h"

namespace Ganymede
{
	class GLFWInputSystem : public InputSystem
	{
	public:
		GLFWInputSystem::GLFWInputSystem(void* nativeWindow, EventSystem& eventSystem);
		bool Initialize() override;

		virtual bool IsNativeKeyPressed(int nativeKeyCode) override;
		virtual bool IsNativeMouseButtonPressed(int nativeButtonCode) override;
		virtual glm::vec2 GetNativeMousePosition() override;
	};
}

#endif //GM_PLATFORM_WINDOWS