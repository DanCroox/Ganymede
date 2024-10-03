#include "ApplicationWindow.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Ganymede/Log/Log.h"

#include <algorithm>

ApplicationWindow::~ApplicationWindow()
{
    if (m_GLFWWindow == nullptr)
    {
        glfwTerminate();
        return;
    }

    glfwDestroyWindow(m_GLFWWindow);
    glfwTerminate();
}

void OnMouseMove(MouseMoveEvent& ev)
{
    GM_CORE_TRACE("Mouse Pos: {} x {}", ev.GetPosition().x, ev.GetPosition().y);
}

void OnWindowResized(WindowResizeEvent& ev)
{
    GM_CORE_TRACE("Window Resized: {} x {}", ev.GetSize().x, ev.GetSize().y);
}

bool ApplicationWindow::TryStart()
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

    EventSystem::GetInstance().NotifyEvent(WindowInitializeEvent());

    glfwSetWindowSizeCallback(m_GLFWWindow, [](GLFWwindow* window, int width, int height) {
        WindowResizeEvent event({ width, height });
        EventSystem::GetInstance().NotifyEvent(event);
        });

    glfwSetCursorPosCallback(m_GLFWWindow, [](GLFWwindow* window, double xpos, double ypos) {
        MouseMoveEvent event({ xpos, ypos });
        EventSystem::GetInstance().NotifyEvent(event);
    });

    glfwSetScrollCallback(m_GLFWWindow, [](GLFWwindow* window, double xOffset, double yOffset) {
        MouseScrollEvent event({ xOffset, yOffset });
        EventSystem::GetInstance().NotifyEvent(event);
        });

    double gameTime = glfwGetTime();
    double deltaTime = 0;

    while (!glfwWindowShouldClose(m_GLFWWindow))
    {
        glfwPollEvents();

        deltaTime = glfwGetTime() - gameTime;
        gameTime = glfwGetTime();
        EventSystem::GetInstance().NotifyEvent(WindowTickEvent(deltaTime, gameTime));

        glfwSwapBuffers(m_GLFWWindow);
    }

    glfwDestroyWindow(m_GLFWWindow);
    glfwTerminate();

    GM_CORE_INFO("glfw terminated");

    EventSystem::GetInstance().NotifyEvent(WindowCloseEvent());

    return true;
}

void ApplicationWindow::SetVSyncEnabled(bool isEnabled)
{
    m_IsVSyncEnabled = isEnabled;
    glfwSwapInterval(isEnabled ? 1 : 0);
}

bool ApplicationWindow::IsVSyncEnabled() const
{
    return m_IsVSyncEnabled;
}