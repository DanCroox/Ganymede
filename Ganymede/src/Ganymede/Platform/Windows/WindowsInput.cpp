#ifdef GM_PLATFORM_WINDOWS

#include "WindowsInput.h"

#include "GLFW/glfw3.h"

namespace Ganymede
{
	bool WindowsInput::IsKeyPressedImpl(int keyCode)
	{
		return false;
	}

	InputSystem* InputSystem::Create()
	{
		return new WindowsInput();
	}
}

#endif //GM_PLATFORM_WINDOWS