#pragma once

#include "Ganymede/Events/Event.h"
#include "Ganymede/Input/KeyCodes.h"
#include "Ganymede/Input/MouseButtonCodes.h"

namespace Ganymede
{
	class GANYMEDE_API WindowInitializeEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(WindowInitializeEvent);
	};

	class GANYMEDE_API WindowResizeEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(WindowResizeEvent);

		WindowResizeEvent(glm::ivec2 size) : m_Size(size) {};
		inline glm::ivec2 GetSize() const { return m_Size; }

	private:
		glm::ivec2 m_Size;
	};

	class GANYMEDE_API WindowTickEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(WindowTickEvent);

		WindowTickEvent(double frameDelta, double gameTime, unsigned int frameIndex) : m_FrameDelta(frameDelta), m_GameTime(gameTime), m_FrameIndex(frameIndex) {};
		inline double GetFrameDelta() const { return m_FrameDelta; }
		inline double GetGameTime() const { return m_GameTime; }
		inline unsigned int GetFrameIndex() const { return m_FrameIndex; }

	private:
		double m_FrameDelta = 0.0f;
		double m_GameTime = 0.0f;
		unsigned int m_FrameIndex = 0;
	};

	class GANYMEDE_API WindowCloseEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(WindowCloseEvent);
	};

	class GANYMEDE_API MouseMoveEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(MouseMoveEvent);

		MouseMoveEvent(glm::vec2 position) : m_Position(position) {};
		inline glm::vec2 GetPosition() const { return m_Position; }

	private:
		glm::vec2 m_Position;
	};

	class GANYMEDE_API MouseScrollEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(MouseScrollEvent);

		MouseScrollEvent(glm::vec2 offset) : m_Offset(offset) {};
		glm::vec2 GetPosition() const { return m_Offset; }

	private:
		glm::vec2 m_Offset;
	};

	class GANYMEDE_API MouseButtonPressEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(MouseButtonPressEvent);

		MouseButtonPressEvent(MouseButtonCode mouseButtonCode) : m_MouseButtonCode(mouseButtonCode) {};
		MouseButtonCode GetMouseButtonCode() const { return m_MouseButtonCode; }

	private:
		MouseButtonCode m_MouseButtonCode;
	};

	class GANYMEDE_API MouseButtonReleaseEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(MouseButtonReleaseEvent);

		MouseButtonReleaseEvent(MouseButtonCode mouseButtonCode) : m_MouseButtonCode(mouseButtonCode) {};
		MouseButtonCode GetMouseButtonCode() const { return m_MouseButtonCode; }

	private:
		MouseButtonCode m_MouseButtonCode;
	};

	class GANYMEDE_API KeyPressEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(KeyPressEvent);

		KeyPressEvent(KeyCode keyCode) : m_KeyCode(keyCode) {};
		KeyCode GetKeyCode() const { return m_KeyCode; }

	private:
		KeyCode m_KeyCode = KeyCode::_Invalid;
	};

	class GANYMEDE_API KeyReleaseEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID(KeyReleaseEvent);

		KeyReleaseEvent(KeyCode keyCode) : m_KeyCode(keyCode) {};
		KeyCode GetKeyCode() const { return m_KeyCode; }

	private:
		KeyCode m_KeyCode = KeyCode::_Invalid;
	};
}