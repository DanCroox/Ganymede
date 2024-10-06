#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/Input.h"#

namespace Ganymede
{
	class WindowsInput : public InputSystem
	{
	public:
		virtual bool IsKeyPressedImpl(int keyCode) override;
	};
}

#endif //GM_PLATFORM_WINDOWS