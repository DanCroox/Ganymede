#pragma once

#include "Ganymede/Platform/Input.h"

namespace Ganymede
{
	class EventSystem;

	class GLFWInputSystem : public InputSystem
	{
	protected:
		GLFWInputSystem(void* nativeWindow, EventSystem& eventSystem);
	};
}