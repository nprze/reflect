#pragma once
#ifndef ANDROID_BUILD


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
#include <filesystem>

#define RFCT_TRACE(...) spdlog::trace("[{}:{}] [trace] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_INFO(...) spdlog::info("[{}:{}] [info] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_WARN(...) spdlog::warn("[{}:{}] [warn] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_ERROR(...) spdlog::error("[{}:{}] [error] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_CRITICAL(...) spdlog::error("[{}:{}] [critical error] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__)); RFCT_ASSERT(false)

#define RFCT_LOGGER_INIT() initialize_logger();

inline void initialize_logger() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    console_sink->set_pattern("%^[%H:%M:%S:%e] %v%$");
    

    auto logger = std::make_shared<spdlog::logger>("console", console_sink);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}
#else

#define RFCT_TRACE(...) 
#define RFCT_INFO(...)
#define RFCT_WARN(...)
#define RFCT_ERROR(...)
#define RFCT_CRITICAL(...) RFCT_ASSERT(false)

#define RFCT_LOGGER_INIT() initialize_logger();
#endif // ANDROID_BUILD