#ifndef GM_RETAIL
	#include "Log.h"

	#include "spdlog/sinks/stdout_color_sinks.h"
	#include <assimp/Importer.hpp>
	#include "btBulletDynamicsCommon.h"
	#include "GL/glew.h"
	#include "GLFW/glfw3.h"
	#include "glm/glm.hpp"
	#include "Recast.h"
	#include "DetourNavMesh.h"
	#include "DetourNavMeshBuilder.h"
	#include "DetourNavMeshQuery.h"
	#include "DetourCrowd.h"

	namespace Ganymede
	{
		std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

		void Logger::Init()
		{
			Assimp::Importer importer;
			btTransform groundTransform = btTransform::getIdentity();
			glfwCreateWindow(10, 10, "window", nullptr, nullptr);
			glm::vec3 vec(1.0f);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			rcContext* rc = new rcContext(true);
			delete rc;

			spdlog::set_pattern("%^[%l] %s (%#): [%c %z] [%n] [thread %t]: %v%$");
			s_CoreLogger = spdlog::stdout_color_mt("Ganymede");
			s_CoreLogger->set_level(spdlog::level::trace);

			s_ClientLogger = spdlog::stdout_color_mt("App");
			s_ClientLogger->set_level(spdlog::level::trace);

			GM_CORE_INFO(vec.x);
		}
	}
#endif // GM_RETAIL