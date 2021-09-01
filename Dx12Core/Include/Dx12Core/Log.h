#pragma once

#include <memory>
#include "spdlog/spdlog.h"

namespace Dx12Core
{
	class Log
	{
	public:
		static void Initialize();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// -- Core log macros ----
#define LOG_CORE_TRACE(...) ::Dx12Core::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...)	::Dx12Core::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...)	::Dx12Core::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Dx12Core::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) ::Dx12Core::Log::GetCoreLogger()->critical(__VA_ARGS__)


#define LOG_TRACE(...)	::Dx12Core::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)	::Dx12Core::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)	::Dx12Core::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)	::Dx12Core::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)	::Dx12Core::Log::GetClientLogger()->critical(__VA_ARGS__)


// Strip out for distribution builds.