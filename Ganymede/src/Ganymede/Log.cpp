#ifndef GM_RETAIL
	#include "Log.h"

	#include "spdlog/sinks/stdout_color_sinks.h"

	namespace Ganymede
	{
		std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

		void Logger::Init()
		{
			spdlog::set_pattern("%^[%l] %s (%#): [%c %z] [%n] [thread %t]: %v%$");
			s_CoreLogger = spdlog::stdout_color_mt("Ganymede");
			s_CoreLogger->set_level(spdlog::level::trace);

			s_ClientLogger = spdlog::stdout_color_mt("App");
			s_ClientLogger->set_level(spdlog::level::trace);
		}
	}
#endif // GM_RETAIL