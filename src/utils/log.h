#pragma once
#ifdef WINDOWS_BUILD


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
#include <filesystem>

#define RFCT_TRACE(...) spdlog::trace("[{}:{}] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_INFO(...) spdlog::info("[{}:{}] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_WARN(...) spdlog::warn("[{}:{}] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_ERROR(...) spdlog::error("[{}:{}] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__))
#define RFCT_CRITICAL(...) { spdlog::error("[{}:{}] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, fmt::format(__VA_ARGS__)); RFCT_ASSERT(false) }

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

#include <android/log.h>
#include <format>

#define LOG_TAG "reflectEngine"

#define RFCT_TRACE(fmtStr, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", std::format(fmtStr, ##__VA_ARGS__).c_str())
#define RFCT_INFO(fmtStr, ...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", std::format(fmtStr, ##__VA_ARGS__).c_str())
#define RFCT_WARN(fmtStr, ...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", std::format(fmtStr, ##__VA_ARGS__).c_str())
#define RFCT_ERROR(fmtStr, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", std::format(fmtStr, ##__VA_ARGS__).c_str())

#define RFCT_CRITICAL(...) RFCT_ASSERT(false)


#endif