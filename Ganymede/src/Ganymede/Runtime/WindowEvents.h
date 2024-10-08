#pragma once

#include "Ganymede/Events/Event.h"
#include "Ganymede/Input/KeyCodes.h"
#include "Ganymede/Input/MouseButtonCodes.h"

namespace Ganymede
{
	class GANYMEDE_API WindowInitializeEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();
	};

	class GANYMEDE_API WindowResizeEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

		WindowResizeEvent(glm::ivec2 size) : m_Size(size) {};
		inline glm::ivec2 GetSize() const { return m_Size; }

	private:
		glm::ivec2 m_Size;
	};

	class GANYMEDE_API WindowTickEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

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
		CREATE_EVENT_CLASS_ID();
	};

	class GANYMEDE_API MouseMoveEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

		MouseMoveEvent(glm::vec2 position) : m_Position(position) {};
		inline glm::vec2 GetPosition() const { return m_Position; }

	private:
		glm::vec2 m_Position;
	};

	class GANYMEDE_API MouseScrollEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

		MouseScrollEvent(glm::vec2 offset) : m_Offset(offset) {};
		glm::vec2 GetPosition() const { return m_Offset; }

	private:
		glm::vec2 m_Offset;
	};

	class GANYMEDE_API KeyPressEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

		KeyPressEvent(KeyCode keyCode) : m_KeyCode(keyCode) {};
		KeyCode GetKeyCode() const { return m_KeyCode; }

	private:
		KeyCode m_KeyCode = KeyCode::_Invalid;
	};

	class GANYMEDE_API KeyReleaseEvent : public Event
	{
	public:
		CREATE_EVENT_CLASS_ID();

		KeyReleaseEvent(KeyCode keyCode) : m_KeyCode(keyCode) {};
		KeyCode GetKeyCode() const { return m_KeyCode; }

	private:
		KeyCode m_KeyCode = KeyCode::_Invalid;
	};
}