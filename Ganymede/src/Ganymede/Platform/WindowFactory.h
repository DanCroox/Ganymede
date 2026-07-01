#pragma once

#include <memory>

namespace Ganymede
{
	class EventSystem;
	class InputSystem;
	class Window;

	namespace WindowFactory
	{
		std::unique_ptr<InputSystem> CreateInputSystem(void* nativeWindow, EventSystem& eventSystem);
		std::unique_ptr<Window> CreateApplicationWindow(EventSystem& eventSystem);
	}
}