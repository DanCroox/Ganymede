#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Core/Application.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Platform/Window.h"
#include "Ganymede/Runtime/WindowEvents.h"
#include "GLFW/glfw3.h"
#include "WindowsInput.h"

namespace Ganymede
{
	InputSystem* InputSystem::Create(void* nativeWindow, EventSystem& eventSystem)
	{
		// Create keycode mapping
		std::unordered_map<KeyCode, int> keyCodeMapping;
		keyCodeMapping[KeyCode::Key_Space] = GLFW_KEY_SPACE;
		keyCodeMapping[KeyCode::Key_Apostrophe] = GLFW_KEY_APOSTROPHE;
		keyCodeMapping[KeyCode::Key_Comma] = GLFW_KEY_COMMA;
		keyCodeMapping[KeyCode::Key_Minus] = GLFW_KEY_MINUS;
		keyCodeMapping[KeyCode::Key_Period] = GLFW_KEY_PERIOD;
		keyCodeMapping[KeyCode::Key_Slash] = GLFW_KEY_SLASH;
		keyCodeMapping[KeyCode::Key_0] = GLFW_KEY_0;
		keyCodeMapping[KeyCode::Key_1] = GLFW_KEY_1;
		keyCodeMapping[KeyCode::Key_2] = GLFW_KEY_2;
		keyCodeMapping[KeyCode::Key_3] = GLFW_KEY_3;
		keyCodeMapping[KeyCode::Key_4] = GLFW_KEY_4;
		keyCodeMapping[KeyCode::Key_5] = GLFW_KEY_5;
		keyCodeMapping[KeyCode::Key_6] = GLFW_KEY_6;
		keyCodeMapping[KeyCode::Key_7] = GLFW_KEY_7;
		keyCodeMapping[KeyCode::Key_8] = GLFW_KEY_8;
		keyCodeMapping[KeyCode::Key_9] = GLFW_KEY_9;
		keyCodeMapping[KeyCode::Key_Semicolon] = GLFW_KEY_SEMICOLON;
		keyCodeMapping[KeyCode::Key_Equal] = GLFW_KEY_EQUAL;
		keyCodeMapping[KeyCode::Key_A] = GLFW_KEY_A;
		keyCodeMapping[KeyCode::Key_B] = GLFW_KEY_B;
		keyCodeMapping[KeyCode::Key_C] = GLFW_KEY_C;
		keyCodeMapping[KeyCode::Key_D] = GLFW_KEY_D;
		keyCodeMapping[KeyCode::Key_E] = GLFW_KEY_E;
		keyCodeMapping[KeyCode::Key_F] = GLFW_KEY_F;
		keyCodeMapping[KeyCode::Key_G] = GLFW_KEY_G;
		keyCodeMapping[KeyCode::Key_H] = GLFW_KEY_H;
		keyCodeMapping[KeyCode::Key_I] = GLFW_KEY_I;
		keyCodeMapping[KeyCode::Key_J] = GLFW_KEY_J;
		keyCodeMapping[KeyCode::Key_K] = GLFW_KEY_K;
		keyCodeMapping[KeyCode::Key_L] = GLFW_KEY_L;
		keyCodeMapping[KeyCode::Key_M] = GLFW_KEY_M;
		keyCodeMapping[KeyCode::Key_N] = GLFW_KEY_N;
		keyCodeMapping[KeyCode::Key_O] = GLFW_KEY_O;
		keyCodeMapping[KeyCode::Key_P] = GLFW_KEY_P;
		keyCodeMapping[KeyCode::Key_Q] = GLFW_KEY_Q;
		keyCodeMapping[KeyCode::Key_R] = GLFW_KEY_R;
		keyCodeMapping[KeyCode::Key_S] = GLFW_KEY_S;
		keyCodeMapping[KeyCode::Key_T] = GLFW_KEY_T;
		keyCodeMapping[KeyCode::Key_U] = GLFW_KEY_U;
		keyCodeMapping[KeyCode::Key_V] = GLFW_KEY_V;
		keyCodeMapping[KeyCode::Key_W] = GLFW_KEY_W;
		keyCodeMapping[KeyCode::Key_X] = GLFW_KEY_X;
		keyCodeMapping[KeyCode::Key_Y] = GLFW_KEY_Y;
		keyCodeMapping[KeyCode::Key_Z] = GLFW_KEY_Z;
		keyCodeMapping[KeyCode::Key_Left_bracket] = GLFW_KEY_LEFT_BRACKET;
		keyCodeMapping[KeyCode::Key_Backslash] = GLFW_KEY_BACKSLASH;
		keyCodeMapping[KeyCode::Key_Right_Bracket] = GLFW_KEY_RIGHT_BRACKET;
		keyCodeMapping[KeyCode::Key_Grave_Accent] = GLFW_KEY_GRAVE_ACCENT;
		keyCodeMapping[KeyCode::Key_World_1] = GLFW_KEY_WORLD_1;
		keyCodeMapping[KeyCode::Key_World_2] = GLFW_KEY_WORLD_2;
		keyCodeMapping[KeyCode::Key_Escape] = GLFW_KEY_ESCAPE;
		keyCodeMapping[KeyCode::Key_Enter] = GLFW_KEY_ENTER;
		keyCodeMapping[KeyCode::Key_Tab] = GLFW_KEY_TAB;
		keyCodeMapping[KeyCode::Key_Backspace] = GLFW_KEY_BACKSPACE;
		keyCodeMapping[KeyCode::Key_Insert] = GLFW_KEY_INSERT;
		keyCodeMapping[KeyCode::Key_Delete] = GLFW_KEY_DELETE;
		keyCodeMapping[KeyCode::Key_Right] = GLFW_KEY_RIGHT;
		keyCodeMapping[KeyCode::Key_Left] = GLFW_KEY_LEFT;
		keyCodeMapping[KeyCode::Key_Down] = GLFW_KEY_DOWN;
		keyCodeMapping[KeyCode::Key_Up] = GLFW_KEY_UP;
		keyCodeMapping[KeyCode::Key_Page_Up] = GLFW_KEY_PAGE_UP;
		keyCodeMapping[KeyCode::Key_Page_Down] = GLFW_KEY_PAGE_DOWN;
		keyCodeMapping[KeyCode::Key_Home] = GLFW_KEY_HOME;
		keyCodeMapping[KeyCode::Key_End] = GLFW_KEY_END;
		keyCodeMapping[KeyCode::Key_Caps_Lock] = GLFW_KEY_CAPS_LOCK;
		keyCodeMapping[KeyCode::Key_Scroll_Lock] = GLFW_KEY_SCROLL_LOCK;
		keyCodeMapping[KeyCode::Key_Num_Lock] = GLFW_KEY_NUM_LOCK;
		keyCodeMapping[KeyCode::Key_Print_Screen] = GLFW_KEY_PRINT_SCREEN;
		keyCodeMapping[KeyCode::Key_Pause] = GLFW_KEY_PAUSE;
		keyCodeMapping[KeyCode::Key_F1] = GLFW_KEY_F1;
		keyCodeMapping[KeyCode::Key_F2] = GLFW_KEY_F2;
		keyCodeMapping[KeyCode::Key_F3] = GLFW_KEY_F3;
		keyCodeMapping[KeyCode::Key_F4] = GLFW_KEY_F4;
		keyCodeMapping[KeyCode::Key_F5] = GLFW_KEY_F5;
		keyCodeMapping[KeyCode::Key_F6] = GLFW_KEY_F6;
		keyCodeMapping[KeyCode::Key_F7] = GLFW_KEY_F7;
		keyCodeMapping[KeyCode::Key_F8] = GLFW_KEY_F8;
		keyCodeMapping[KeyCode::Key_F9] = GLFW_KEY_F9;
		keyCodeMapping[KeyCode::Key_F10] = GLFW_KEY_F10;
		keyCodeMapping[KeyCode::Key_F11] = GLFW_KEY_F11;
		keyCodeMapping[KeyCode::Key_F12] = GLFW_KEY_F12;
		keyCodeMapping[KeyCode::Key_F13] = GLFW_KEY_F13;
		keyCodeMapping[KeyCode::Key_F14] = GLFW_KEY_F14;
		keyCodeMapping[KeyCode::Key_F15] = GLFW_KEY_F15;
		keyCodeMapping[KeyCode::Key_F16] = GLFW_KEY_F16;
		keyCodeMapping[KeyCode::Key_F17] = GLFW_KEY_F17;
		keyCodeMapping[KeyCode::Key_F18] = GLFW_KEY_F18;
		keyCodeMapping[KeyCode::Key_F19] = GLFW_KEY_F19;
		keyCodeMapping[KeyCode::Key_F20] = GLFW_KEY_F20;
		keyCodeMapping[KeyCode::Key_F21] = GLFW_KEY_F21;
		keyCodeMapping[KeyCode::Key_F22] = GLFW_KEY_F22;
		keyCodeMapping[KeyCode::Key_F23] = GLFW_KEY_F23;
		keyCodeMapping[KeyCode::Key_F24] = GLFW_KEY_F24;
		keyCodeMapping[KeyCode::Key_F25] = GLFW_KEY_F25;
		keyCodeMapping[KeyCode::Key_Numpad_0] = GLFW_KEY_KP_0;
		keyCodeMapping[KeyCode::Key_Numpad_1] = GLFW_KEY_KP_1;
		keyCodeMapping[KeyCode::Key_Numpad_2] = GLFW_KEY_KP_2;
		keyCodeMapping[KeyCode::Key_Numpad_3] = GLFW_KEY_KP_3;
		keyCodeMapping[KeyCode::Key_Numpad_4] = GLFW_KEY_KP_4;
		keyCodeMapping[KeyCode::Key_Numpad_5] = GLFW_KEY_KP_5;
		keyCodeMapping[KeyCode::Key_Numpad_6] = GLFW_KEY_KP_6;
		keyCodeMapping[KeyCode::Key_Numpad_7] = GLFW_KEY_KP_7;
		keyCodeMapping[KeyCode::Key_Numpad_8] = GLFW_KEY_KP_8;
		keyCodeMapping[KeyCode::Key_Numpad_9] = GLFW_KEY_KP_9;
		keyCodeMapping[KeyCode::Key_Numpad_Decimal] = GLFW_KEY_KP_DECIMAL;
		keyCodeMapping[KeyCode::Key_Numpad_Divide] = GLFW_KEY_KP_DIVIDE;
		keyCodeMapping[KeyCode::Key_Numpad_Multiply] = GLFW_KEY_KP_MULTIPLY;
		keyCodeMapping[KeyCode::Key_Numpad_Subtract] = GLFW_KEY_KP_SUBTRACT;
		keyCodeMapping[KeyCode::Key_Numpad_Add] = GLFW_KEY_KP_ADD;
		keyCodeMapping[KeyCode::Key_Numpad_Enter] = GLFW_KEY_KP_ENTER;
		keyCodeMapping[KeyCode::Key_Numpad_Equal] = GLFW_KEY_KP_EQUAL;
		keyCodeMapping[KeyCode::Key_Left_Shift] = GLFW_KEY_LEFT_SHIFT;
		keyCodeMapping[KeyCode::Key_Left_Control] = GLFW_KEY_LEFT_CONTROL;
		keyCodeMapping[KeyCode::Key_Left_Alt] = GLFW_KEY_LEFT_ALT;
		keyCodeMapping[KeyCode::Key_Left_Super] = GLFW_KEY_LEFT_SUPER;
		keyCodeMapping[KeyCode::Key_Right_Shift] = GLFW_KEY_RIGHT_SHIFT;
		keyCodeMapping[KeyCode::Key_Right_Control] = GLFW_KEY_RIGHT_CONTROL;
		keyCodeMapping[KeyCode::Key_Right_Alt] = GLFW_KEY_RIGHT_ALT;
		keyCodeMapping[KeyCode::Key_Right_Super] = GLFW_KEY_RIGHT_SUPER;
		keyCodeMapping[KeyCode::Key_Menu] = GLFW_KEY_MENU;

		// Create mouse button code mapping
		std::unordered_map<MouseButtonCode, int> mouseButtonCodeMapping;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_1] = GLFW_MOUSE_BUTTON_1;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_2] = GLFW_MOUSE_BUTTON_2;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_3] = GLFW_MOUSE_BUTTON_3;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_4] = GLFW_MOUSE_BUTTON_4;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_5] = GLFW_MOUSE_BUTTON_5;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_6] = GLFW_MOUSE_BUTTON_6;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_7] = GLFW_MOUSE_BUTTON_7;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_8] = GLFW_MOUSE_BUTTON_8;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_8] = GLFW_MOUSE_BUTTON_8;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_Last] = GLFW_MOUSE_BUTTON_LAST;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_Left] = GLFW_MOUSE_BUTTON_LEFT;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_Right] = GLFW_MOUSE_BUTTON_RIGHT;
		mouseButtonCodeMapping[MouseButtonCode::Mouse_Middle] = GLFW_MOUSE_BUTTON_MIDDLE;

		return new WindowsInput(nativeWindow, eventSystem, std::move(keyCodeMapping), std::move(mouseButtonCodeMapping));
	}

	bool WindowsInput::Initialize()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(m_NativeWindow);
		// Dont overwrite user pointer elsewhere!
		glfwSetWindowUserPointer(window, this);
		
		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			WindowResizeEvent event({ width, height });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			MouseMoveEvent event({ xpos, ypos });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
			MouseScrollEvent event({ xOffset, yOffset });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
				switch (action)
				{
					case GLFW_PRESS:
					{
						const WindowsInput& input = *static_cast<WindowsInput*>(glfwGetWindowUserPointer(window));
						MouseButtonPressEvent event(input.ConvertToMouseButtonCode(button));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const WindowsInput& input = *static_cast<WindowsInput*>(glfwGetWindowUserPointer(window));
						MouseButtonReleaseEvent event(input.ConvertToMouseButtonCode(button));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					default:
						break;
				}
			});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				switch (action)
				{
					case GLFW_PRESS:
					{
						const WindowsInput& input = *static_cast<WindowsInput*>(glfwGetWindowUserPointer(window));
						KeyPressEvent event(input.ConvertToKeyCode(key));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const WindowsInput& input = *static_cast<WindowsInput*>(glfwGetWindowUserPointer(window));
						KeyReleaseEvent event(input.ConvertToKeyCode(key));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					default:
						break;
				}
			});

		return true;
	}

	bool WindowsInput::IsNativeKeyPressed(int nativeKeyCode)
	{
		auto state = glfwGetKey(static_cast<GLFWwindow*>(m_NativeWindow), nativeKeyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsNativeMouseButtonPressed(int nativeMouseButtonCode)
	{
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(m_NativeWindow), nativeMouseButtonCode);
		return state == GLFW_PRESS;
	}

	glm::vec2 WindowsInput::GetNativeMousePosition()
	{
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(m_NativeWindow), &x, &y);
		return { x, y };
	}
}

#endif //GM_PLATFORM_WINDOWS