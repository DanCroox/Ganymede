#include "Input.h"

#include "Ganymede/Events/Event.h"

namespace Ganymede
{
	InputSystem::InputSystem(
		void* nativeWindow,
		EventSystem& eventSystem,
		std::unordered_map<KeyCode, int>&& keyCodeMapping,
		std::unordered_map<MouseButtonCode, int>&& mouseButtonCodeMapping) : m_NativeWindow(nativeWindow), m_EventSystem(eventSystem)
	{
		// TODO: Add check if all keycodes are contained in map -> if not assert!
		m_KeyCodeToNativeKeyCodeLookup = std::move(keyCodeMapping);
		m_MouseButtonCodeToNativeMouseButtonCodeLookup = std::move(mouseButtonCodeMapping);

		// Build reverse lookup table for key codes
		for (const auto& [keyCode, nativeKeyCode] : m_KeyCodeToNativeKeyCodeLookup)
		{
			m_NativeKeyCodeToKeyCodeLookup[nativeKeyCode] = keyCode;
		}

		// Build reverse lookup table for mouse button codes
		for (const auto& [mouseButtonCode, nativeMouseButtonCode] : m_MouseButtonCodeToNativeMouseButtonCodeLookup)
		{
			m_NativeMouseButtonCodeToMouseButtonCodeLookup[nativeMouseButtonCode] = mouseButtonCode;
		}
	}
}