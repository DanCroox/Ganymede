#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Platform/Input.h"#

namespace Ganymede
{
	class WindowsInput : public InputSystem
	{
	public:
		bool Initialize() override;

		virtual bool IsNativeKeyPressed(int nativeKeyCode) override;
		virtual bool IsNativeMouseButtonPressed(int nativeButtonCode) override;
		virtual glm::vec2 GetNativeMousePosition() override;

	protected:
		using InputSystem::InputSystem;
	};
}

#endif //GM_PLATFORM_WINDOWS