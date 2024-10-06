#pragma once

#include <memory>

namespace Ganymede
{
	class InputSystem
	{
	public:
		virtual ~InputSystem() = default;

		inline static bool IsKeyPressed(int keyCode) { return s_Instance->IsKeyPressedImpl(keyCode); }

		// Needs to be implemented per Platform
		static InputSystem* Create();

	protected:
		InputSystem() = default;
		virtual bool IsKeyPressedImpl(int keyCode) = 0;

	private:
		static InputSystem* s_Instance;
	};
}