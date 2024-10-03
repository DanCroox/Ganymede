#pragma once

#ifdef GM_RETAIL
	#define GM_INIT_LOGGER

	#define GM_CORE_TRACE
	#define GM_CORE_INFO
	#define GM_CORE_WARN
	#define GM_CORE_ERROR
	#define GM_CORE_CRITICAL

	#define GM_TRACE
	#define GM_INFO
	#define GM_WARN
	#define GM_ERROR
	#define GM_CRITICAL
#else
	// Needed to unlock all loglevels for the SPDLOG_LOGGER_TRACE/DEBUG... defines. Leave it like that and change loglevel at the respective logger.
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

	#include "Ganymede/Core/Core.h"
	#include "spdlog/spdlog.h"

	// Although "Logger" is exported, this does not happen to the shared_ptr template-members implicitly. Need to forward declare to fix it.
	template class GANYMEDE_API std::shared_ptr<spdlog::logger>;

	namespace Ganymede
	{
		class GANYMEDE_API Logger
		{
		public:
			static void Init();
		
			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {	return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
		};
	}

	#define GM_INIT_LOGGER Ganymede::Logger::Init();

	#define GM_CORE_TRACE(...) SPDLOG_LOGGER_TRACE(Ganymede::Logger::GetCoreLogger(), __VA_ARGS__);
	#define GM_CORE_INFO(...) SPDLOG_LOGGER_INFO(Ganymede::Logger::GetCoreLogger(), __VA_ARGS__);
	#define GM_CORE_WARN(...) SPDLOG_LOGGER_WARN(Ganymede::Logger::GetCoreLogger(), __VA_ARGS__);
	#define GM_CORE_ERROR(...) SPDLOG_LOGGER_ERROR(Ganymede::Logger::GetCoreLogger(), __VA_ARGS__);
	#define GM_CORE_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Ganymede::Logger::GetCoreLogger(), __VA_ARGS__);

	#define GM_TRACE(...) SPDLOG_LOGGER_TRACE(Ganymede::Logger::GetClientLogger(), __VA_ARGS__);
	#define GM_INFO(...) SPDLOG_LOGGER_INFO(Ganymede::Logger::GetClientLogger(), __VA_ARGS__);
	#define GM_WARN(...) SPDLOG_LOGGER_WARN(Ganymede::Logger::GetClientLogger(), __VA_ARGS__);
	#define GM_ERROR(...) SPDLOG_LOGGER_ERROR(Ganymede::Logger::GetClientLogger(), __VA_ARGS__);
	#define GM_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Ganymede::Logger::GetClientLogger(), __VA_ARGS__);
#endif // GM_RETAIL