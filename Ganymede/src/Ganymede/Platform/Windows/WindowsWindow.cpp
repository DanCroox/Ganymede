#ifdef GM_PLATFORM_WINDOWS

#include "WindowsWindow.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Runtime/WindowEvents.h"
#include "Ganymede/Core/Application.h"

#include <algorithm>

namespace Ganymede
{
    Window* Window::Create()
    {
        return new WindowsWindow();
    }

    WindowsWindow::~WindowsWindow()
    {
        if (m_GLFWWindow == nullptr)
        {
            glfwTerminate();
            return;
        }

        glfwDestroyWindow(m_GLFWWindow);
        glfwTerminate();
    }

    void* WindowsWindow::GetNativeWindow()
    {
        return m_GLFWWindow;
    }

    bool WindowsWindow::TryStart()
    {
        if (!glfwInit())
        {
            GM_CORE_ERROR("glfwInit failed");
            return false;
        }
        GM_CORE_INFO("glfwInit succeeded");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_GLFWWindow = glfwCreateWindow(1920, 1080, "Render View", NULL, NULL);
        if (m_GLFWWindow == nullptr)
        {
            GM_CORE_ERROR("glfwCreateWindow failed");
            glfwTerminate();
            return false;
        }
        GM_CORE_INFO("glfwCreateWindow succeeded");

        glfwMakeContextCurrent(m_GLFWWindow);     //Make the window's context current  
        glfwSetWindowUserPointer(m_GLFWWindow, this);
        glfwSetWindowPos(m_GLFWWindow, 200, 100);
        SetVSyncEnabled(false);

        // Init Glew
        if (glewInit() != GLEW_OK)
        {
            GM_CORE_ERROR("glewInit failed");
            return false;
        }
        GM_CORE_INFO("glewInit succeeded");

        glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_GLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwSetWindowSizeCallback(m_GLFWWindow, [](GLFWwindow* window, int width, int height) {
            WindowResizeEvent event({ width, height });
            Application::Get().GetEventSystem().NotifyEvent(event);
            });

        glfwSetCursorPosCallback(m_GLFWWindow, [](GLFWwindow* window, double xpos, double ypos) {
            MouseMoveEvent event({ xpos, ypos });
            Application::Get().GetEventSystem().NotifyEvent(event);
            });

        glfwSetScrollCallback(m_GLFWWindow, [](GLFWwindow* window, double xOffset, double yOffset) {
            MouseScrollEvent event({ xOffset, yOffset });
            Application::Get().GetEventSystem().NotifyEvent(event);
            });

        glfwSetKeyCallback(m_GLFWWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            switch (action)
            {
            case GLFW_PRESS:
            {
                KeyPressEvent event({ key });
                Application::Get().GetEventSystem().NotifyEvent(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleaseEvent event({ key });
                Application::Get().GetEventSystem().NotifyEvent(event);
                break;
            }
            }
            });

        Application::Get().GetEventSystem().NotifyEvent(WindowInitializeEvent());

        double gameTime = glfwGetTime();
        double deltaTime = 0;
        unsigned int frameIndex = 0;

        while (!glfwWindowShouldClose(m_GLFWWindow))
        {
            glfwPollEvents();

            deltaTime = glfwGetTime() - gameTime;
            gameTime = glfwGetTime();
            Application::Get().GetEventSystem().NotifyEvent(WindowTickEvent(deltaTime, gameTime, frameIndex));
            ++frameIndex;

            glfwSwapBuffers(m_GLFWWindow);
        }

        glfwDestroyWindow(m_GLFWWindow);
        glfwTerminate();

        GM_CORE_INFO("glfw terminated");

        Application::Get().GetEventSystem().NotifyEvent(WindowCloseEvent());

        return true;
    }

    void WindowsWindow::SetVSyncEnabled(bool isEnabled)
    {
        m_IsVSyncEnabled = isEnabled;
        glfwSwapInterval(isEnabled ? 1 : 0);
    }

    bool WindowsWindow::IsVSyncEnabled() const
    {
        return m_IsVSyncEnabled;
    }
}
#endif //GM_PLATFORM_WINDOWS