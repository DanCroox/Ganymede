#include "GLFWInputSystem.h"

#include "Ganymede/Core/Application.h"
#include "Ganymede/Runtime/WindowEvents.h"

#include "GLFW/glfw3.h"

namespace Ganymede
{
	GLFWInputSystem::GLFWInputSystem(void* nativeWindow, EventSystem& eventSystem) :
		InputSystem(
			nativeWindow,
			eventSystem,
			{
				{ KeyCode::Key_Space, GLFW_KEY_SPACE },
				{ KeyCode::Key_Apostrophe, GLFW_KEY_APOSTROPHE },
				{ KeyCode::Key_Comma, GLFW_KEY_COMMA },
				{ KeyCode::Key_Minus, GLFW_KEY_MINUS },
				{ KeyCode::Key_Period, GLFW_KEY_PERIOD },
				{ KeyCode::Key_Slash, GLFW_KEY_SLASH },
				{ KeyCode::Key_0, GLFW_KEY_0 },
				{ KeyCode::Key_1, GLFW_KEY_1 },
				{ KeyCode::Key_2, GLFW_KEY_2 },
				{ KeyCode::Key_3, GLFW_KEY_3 },
				{ KeyCode::Key_4, GLFW_KEY_4 },
				{ KeyCode::Key_5, GLFW_KEY_5 },
				{ KeyCode::Key_6, GLFW_KEY_6 },
				{ KeyCode::Key_7, GLFW_KEY_7 },
				{ KeyCode::Key_8, GLFW_KEY_8 },
				{ KeyCode::Key_9, GLFW_KEY_9 },
				{ KeyCode::Key_Semicolon, GLFW_KEY_SEMICOLON },
				{ KeyCode::Key_Equal, GLFW_KEY_EQUAL },
				{ KeyCode::Key_A, GLFW_KEY_A },
				{ KeyCode::Key_B, GLFW_KEY_B },
				{ KeyCode::Key_C, GLFW_KEY_C },
				{ KeyCode::Key_D, GLFW_KEY_D },
				{ KeyCode::Key_E, GLFW_KEY_E },
				{ KeyCode::Key_F, GLFW_KEY_F },
				{ KeyCode::Key_G, GLFW_KEY_G },
				{ KeyCode::Key_H, GLFW_KEY_H },
				{ KeyCode::Key_I, GLFW_KEY_I },
				{ KeyCode::Key_J, GLFW_KEY_J },
				{ KeyCode::Key_K, GLFW_KEY_K },
				{ KeyCode::Key_L, GLFW_KEY_L },
				{ KeyCode::Key_M, GLFW_KEY_M },
				{ KeyCode::Key_N, GLFW_KEY_N },
				{ KeyCode::Key_O, GLFW_KEY_O },
				{ KeyCode::Key_P, GLFW_KEY_P },
				{ KeyCode::Key_Q, GLFW_KEY_Q },
				{ KeyCode::Key_R, GLFW_KEY_R },
				{ KeyCode::Key_S, GLFW_KEY_S },
				{ KeyCode::Key_T, GLFW_KEY_T },
				{ KeyCode::Key_U, GLFW_KEY_U },
				{ KeyCode::Key_V, GLFW_KEY_V },
				{ KeyCode::Key_W, GLFW_KEY_W },
				{ KeyCode::Key_X, GLFW_KEY_X },
				{ KeyCode::Key_Y, GLFW_KEY_Y },
				{ KeyCode::Key_Z, GLFW_KEY_Z },
				{ KeyCode::Key_Left_bracket, GLFW_KEY_LEFT_BRACKET },
				{ KeyCode::Key_Backslash, GLFW_KEY_BACKSLASH },
				{ KeyCode::Key_Right_Bracket, GLFW_KEY_RIGHT_BRACKET },
				{ KeyCode::Key_Grave_Accent, GLFW_KEY_GRAVE_ACCENT },
				{ KeyCode::Key_World_1, GLFW_KEY_WORLD_1 },
				{ KeyCode::Key_World_2, GLFW_KEY_WORLD_2 },
				{ KeyCode::Key_Escape, GLFW_KEY_ESCAPE },
				{ KeyCode::Key_Enter, GLFW_KEY_ENTER },
				{ KeyCode::Key_Tab, GLFW_KEY_TAB },
				{ KeyCode::Key_Backspace, GLFW_KEY_BACKSPACE },
				{ KeyCode::Key_Insert, GLFW_KEY_INSERT },
				{ KeyCode::Key_Delete, GLFW_KEY_DELETE },
				{ KeyCode::Key_Right, GLFW_KEY_RIGHT },
				{ KeyCode::Key_Left, GLFW_KEY_LEFT },
				{ KeyCode::Key_Down, GLFW_KEY_DOWN },
				{ KeyCode::Key_Up, GLFW_KEY_UP },
				{ KeyCode::Key_Page_Up, GLFW_KEY_PAGE_UP },
				{ KeyCode::Key_Page_Down, GLFW_KEY_PAGE_DOWN },
				{ KeyCode::Key_Home, GLFW_KEY_HOME },
				{ KeyCode::Key_End, GLFW_KEY_END },
				{ KeyCode::Key_Caps_Lock, GLFW_KEY_CAPS_LOCK },
				{ KeyCode::Key_Scroll_Lock, GLFW_KEY_SCROLL_LOCK },
				{ KeyCode::Key_Num_Lock, GLFW_KEY_NUM_LOCK },
				{ KeyCode::Key_Print_Screen, GLFW_KEY_PRINT_SCREEN },
				{ KeyCode::Key_Pause, GLFW_KEY_PAUSE },
				{ KeyCode::Key_F1, GLFW_KEY_F1 },
				{ KeyCode::Key_F2, GLFW_KEY_F2 },
				{ KeyCode::Key_F3, GLFW_KEY_F3 },
				{ KeyCode::Key_F4, GLFW_KEY_F4 },
				{ KeyCode::Key_F5, GLFW_KEY_F5 },
				{ KeyCode::Key_F6, GLFW_KEY_F6 },
				{ KeyCode::Key_F7, GLFW_KEY_F7 },
				{ KeyCode::Key_F8, GLFW_KEY_F8 },
				{ KeyCode::Key_F9, GLFW_KEY_F9 },
				{ KeyCode::Key_F10, GLFW_KEY_F10 },
				{ KeyCode::Key_F11, GLFW_KEY_F11 },
				{ KeyCode::Key_F12, GLFW_KEY_F12 },
				{ KeyCode::Key_F13, GLFW_KEY_F13 },
				{ KeyCode::Key_F14, GLFW_KEY_F14 },
				{ KeyCode::Key_F15, GLFW_KEY_F15 },
				{ KeyCode::Key_F16, GLFW_KEY_F16 },
				{ KeyCode::Key_F17, GLFW_KEY_F17 },
				{ KeyCode::Key_F18, GLFW_KEY_F18 },
				{ KeyCode::Key_F19, GLFW_KEY_F19 },
				{ KeyCode::Key_F20, GLFW_KEY_F20 },
				{ KeyCode::Key_F21, GLFW_KEY_F21 },
				{ KeyCode::Key_F22, GLFW_KEY_F22 },
				{ KeyCode::Key_F23, GLFW_KEY_F23 },
				{ KeyCode::Key_F24, GLFW_KEY_F24 },
				{ KeyCode::Key_F25, GLFW_KEY_F25 },
				{ KeyCode::Key_Numpad_0, GLFW_KEY_KP_0 },
				{ KeyCode::Key_Numpad_1, GLFW_KEY_KP_1 },
				{ KeyCode::Key_Numpad_2, GLFW_KEY_KP_2 },
				{ KeyCode::Key_Numpad_3, GLFW_KEY_KP_3 },
				{ KeyCode::Key_Numpad_4, GLFW_KEY_KP_4 },
				{ KeyCode::Key_Numpad_5, GLFW_KEY_KP_5 },
				{ KeyCode::Key_Numpad_6, GLFW_KEY_KP_6 },
				{ KeyCode::Key_Numpad_7, GLFW_KEY_KP_7 },
				{ KeyCode::Key_Numpad_8, GLFW_KEY_KP_8 },
				{ KeyCode::Key_Numpad_9, GLFW_KEY_KP_9 },
				{ KeyCode::Key_Numpad_Decimal, GLFW_KEY_KP_DECIMAL },
				{ KeyCode::Key_Numpad_Divide, GLFW_KEY_KP_DIVIDE },
				{ KeyCode::Key_Numpad_Multiply, GLFW_KEY_KP_MULTIPLY },
				{ KeyCode::Key_Numpad_Subtract, GLFW_KEY_KP_SUBTRACT },
				{ KeyCode::Key_Numpad_Add, GLFW_KEY_KP_ADD },
				{ KeyCode::Key_Numpad_Enter, GLFW_KEY_KP_ENTER },
				{ KeyCode::Key_Numpad_Equal, GLFW_KEY_KP_EQUAL },
				{ KeyCode::Key_Left_Shift, GLFW_KEY_LEFT_SHIFT },
				{ KeyCode::Key_Left_Control, GLFW_KEY_LEFT_CONTROL },
				{ KeyCode::Key_Left_Alt, GLFW_KEY_LEFT_ALT },
				{ KeyCode::Key_Left_Super, GLFW_KEY_LEFT_SUPER },
				{ KeyCode::Key_Right_Shift, GLFW_KEY_RIGHT_SHIFT },
				{ KeyCode::Key_Right_Control, GLFW_KEY_RIGHT_CONTROL },
				{ KeyCode::Key_Right_Alt, GLFW_KEY_RIGHT_ALT },
				{ KeyCode::Key_Right_Super, GLFW_KEY_RIGHT_SUPER },
				{ KeyCode::Key_Menu, GLFW_KEY_MENU }
			},
			{
				{ MouseButtonCode::Mouse_1, GLFW_MOUSE_BUTTON_1 },
				{ MouseButtonCode::Mouse_2, GLFW_MOUSE_BUTTON_2 },
				{ MouseButtonCode::Mouse_3, GLFW_MOUSE_BUTTON_3 },
				{ MouseButtonCode::Mouse_4, GLFW_MOUSE_BUTTON_4 },
				{ MouseButtonCode::Mouse_5, GLFW_MOUSE_BUTTON_5 },
				{ MouseButtonCode::Mouse_6, GLFW_MOUSE_BUTTON_6 },
				{ MouseButtonCode::Mouse_7, GLFW_MOUSE_BUTTON_7 },
				{ MouseButtonCode::Mouse_8, GLFW_MOUSE_BUTTON_8 },
				{ MouseButtonCode::Mouse_Last, GLFW_MOUSE_BUTTON_LAST },
				{ MouseButtonCode::Mouse_Left, GLFW_MOUSE_BUTTON_LEFT },
				{ MouseButtonCode::Mouse_Right, GLFW_MOUSE_BUTTON_RIGHT },
				{ MouseButtonCode::Mouse_Middle, GLFW_MOUSE_BUTTON_MIDDLE }
			}
		) {}

	bool GLFWInputSystem::Initialize()
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
						const GLFWInputSystem& input = *static_cast<GLFWInputSystem*>(glfwGetWindowUserPointer(window));
						MouseButtonPressEvent event(input.ConvertToMouseButtonCode(button));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const GLFWInputSystem& input = *static_cast<GLFWInputSystem*>(glfwGetWindowUserPointer(window));
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
						const GLFWInputSystem& input = *static_cast<GLFWInputSystem*>(glfwGetWindowUserPointer(window));
						KeyPressEvent event(input.ConvertToKeyCode(key));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const GLFWInputSystem& input = *static_cast<GLFWInputSystem*>(glfwGetWindowUserPointer(window));
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

	bool GLFWInputSystem::IsNativeKeyPressed(int nativeKeyCode)
	{
		auto state = glfwGetKey(static_cast<GLFWwindow*>(m_NativeWindow), nativeKeyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool GLFWInputSystem::IsNativeMouseButtonPressed(int nativeMouseButtonCode)
	{
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(m_NativeWindow), nativeMouseButtonCode);
		return state == GLFW_PRESS;
	}

	glm::vec2 GLFWInputSystem::GetNativeMousePosition()
	{
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(m_NativeWindow), &x, &y);
		return { x, y };
	}
}