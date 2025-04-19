#include "GPUDebugHandler.h"

#include "gl/glew.h"

namespace Ganymede
{
    void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
        if (id == 131185) return;
        if(severity != GL_DEBUG_SEVERITY_HIGH) return;

        std::cerr << "---- OpenGL Debug Message ----\n";
        std::cerr << "Nachricht: " << message << "\n";
        std::cerr << "Quelle: ";
        switch (source) {
        case GL_DEBUG_SOURCE_API:             std::cerr << "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Other"; break;
        }
        std::cerr << "\nTyp: ";
        switch (type) {
        case GL_DEBUG_TYPE_ERROR:               std::cerr << "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cerr << "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cerr << "Other"; break;
        }
        std::cerr << "\nSchweregrad: ";
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Notification"; break;
        }
        std::cerr << "\nID: " << id << "\n";
        std::cerr << "-------------------------------\n";
        __debugbreak();
    }

    bool GPUDebugHandler::m_IsEnabled = false;

	void GPUDebugHandler::Enable()
	{
        if (m_IsEnabled)
        {
            GM_CORE_ASSERT(false, "Already enabled.");
            return;
        }

		glDebugMessageCallback(glDebugOutput, nullptr);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        m_IsEnabled = true;
	}

	void GPUDebugHandler::Disable()
	{
        if (!m_IsEnabled)
        {
            GM_CORE_ASSERT(false, "Already disabled.");
            return;
        }

		glDisable(GL_DEBUG_OUTPUT);
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(nullptr, nullptr);

        m_IsEnabled = false;
	}
}