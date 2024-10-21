#ifdef GM_PLATFORM_WINDOWS

#include "WindowsWindow.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Runtime/WindowEvents.h"
#include "Ganymede/Common/Helpers.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <algorithm>

namespace Ganymede
{
    Window* Window::Create(EventSystem& eventSystem)
    {
        return new WindowsWindow(eventSystem);
    }

    WindowsWindow::~WindowsWindow()
    {
        TerminateWindow();
    }

    bool WindowsWindow::Initialize()
    {
        if (!glfwInit())
        {
            GM_CORE_CRITICAL("glfwInit failed");
            return false;
        }
        GM_CORE_INFO("glfwInit succeeded");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_GLFWWindow = glfwCreateWindow(1920, 1080, "Render View", NULL, NULL);
        if (m_GLFWWindow == nullptr)
        {
            GM_CORE_CRITICAL("glfwCreateWindow failed");
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
            GM_CORE_CRITICAL("glewInit failed");
            return false;
        }
        GM_CORE_INFO("glewInit succeeded");

        glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_GLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_GLFWWindow, true);
#ifdef __EMSCRIPTEN__
        ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
        ImGui_ImplOpenGL3_Init("#version 460");

        return true;
    }

    void* WindowsWindow::GetNativeWindow()
    {
        return m_GLFWWindow;
    }

    bool WindowsWindow::TryStart()
    {
        m_EventSystem.NotifyEvent(WindowInitializeEvent());

        double gameTime = glfwGetTime();
        double deltaTime = 0;
        unsigned int frameIndex = 0;

        bool show_demo_window = true;
        bool show_another_window = true;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        while (!glfwWindowShouldClose(m_GLFWWindow))
        {
            glfwPollEvents();

            deltaTime = glfwGetTime() - gameTime;
            gameTime = glfwGetTime();

            m_EventSystem.NotifyEvent(WindowTickEvent(deltaTime, gameTime, frameIndex));
            DrawStats(deltaTime, gameTime);

            ++frameIndex;
            glfwSwapBuffers(m_GLFWWindow);
        }

        TerminateWindow();

        m_EventSystem.NotifyEvent(WindowCloseEvent());

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

    void WindowsWindow::DrawStats(double deltaTime, double gameTime)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        ImGui::Begin("Overlay", nullptr, window_flags);
        ImGui::Text("FPS: %f", 1.0f / deltaTime);
        for (const auto& pair : Helpers::ScopedTimer::GetData())
        {
            ImGui::Text("%s: %f", pair.first, pair.second * 0.001f);
        }
        for (const auto& pair : Helpers::NamedCounter::GetData())
        {
            ImGui::Text("%s: %d", pair.first, pair.second);
        }
        Helpers::ScopedTimer::ClearData();
        Helpers::NamedCounter::ClearData();

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_GLFWWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void WindowsWindow::TerminateWindow()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_GLFWWindow);
        GM_CORE_INFO("glfw window destroyed.");

        glfwTerminate();
        GM_CORE_INFO("glfw window terminated.");
    }
}
#endif //GM_PLATFORM_WINDOWS