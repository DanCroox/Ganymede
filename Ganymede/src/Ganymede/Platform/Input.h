#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Input/KeyCodes.h"
#include "Ganymede/Input/MouseButtonCodes.h"
#include <glm/glm.hpp>
#include <memory>

namespace Ganymede
{
	class EventSystem;

	class GANYMEDE_API InputSystem
	{
	public:
		virtual ~InputSystem() = default;

		virtual bool Initialize() = 0;

		inline bool IsKeyPressed(KeyCode keyCode) { return IsNativeKeyPressed(ConvertToNativeKeyCode(keyCode)); }
		inline bool IsMouseButtonPressed(MouseButtonCode mouseButtonCode) { return IsNativeMouseButtonPressed(ConvertToNativeMouseButtonCode(mouseButtonCode)); }
		inline glm::vec2 GetMousePosition() { return GetNativeMousePosition(); }

		// Needs to be implemented per Platform
		static InputSystem* Create(void* nativeWindow, EventSystem& eventSystem);

	protected:
		InputSystem() = delete;
		InputSystem(
			void* nativeWindow,
			EventSystem& eventSystem,
			std::unordered_map<KeyCode, int>&& keyCodeMapping,
			std::unordered_map<MouseButtonCode, int>&& mouseButtonCodeMapping);

		inline KeyCode ConvertToKeyCode(int nativeKeyCode) const { return m_NativeKeyCodeToKeyCodeLookup.find(nativeKeyCode)->second; }
		inline int ConvertToNativeKeyCode(KeyCode KeyCode) const { return m_KeyCodeToNativeKeyCodeLookup.find(KeyCode)->second; }

		inline MouseButtonCode ConvertToMouseButtonCode(int nativeMouseButtonCode) const { return m_NativeMouseButtonCodeToMouseButtonCodeLookup.find(nativeMouseButtonCode)->second; }
		inline int ConvertToNativeMouseButtonCode(MouseButtonCode mouseButtonCode) const { return m_MouseButtonCodeToNativeMouseButtonCodeLookup.find(mouseButtonCode)->second; }

		virtual bool IsNativeKeyPressed(int nativeKeyCode) = 0;
		virtual bool IsNativeMouseButtonPressed(int nativeButtonCode) = 0;
		virtual glm::vec2 GetNativeMousePosition() = 0;

	protected:
		void* m_NativeWindow;
		EventSystem& m_EventSystem;

	private:
		std::unordered_map<int, KeyCode> m_NativeKeyCodeToKeyCodeLookup;
		std::unordered_map<KeyCode, int> m_KeyCodeToNativeKeyCodeLookup;
		std::unordered_map<int, MouseButtonCode> m_NativeMouseButtonCodeToMouseButtonCodeLookup;
		std::unordered_map<MouseButtonCode, int> m_MouseButtonCodeToNativeMouseButtonCodeLookup;
	};
}