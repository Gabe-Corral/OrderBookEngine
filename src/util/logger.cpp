#include <ob/util/logger.hpp>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>

namespace ob::util {

namespace {

const char* to_string(LogLevel level)
{
    switch (level) {
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
    }

    return "UNKNOWN";
}

const char* base_name(const char* path)
{
    const char* file = path;

    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            file = p + 1;
        }
    }

    return file;
}

char cached_timestamp[20]{};
std::time_t last_second = 0;

const char* get_timestamp()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto seconds = system_clock::to_time_t(now);

    if (seconds != last_second) {
        last_second = seconds;

        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &seconds);
#else
        localtime_r(&seconds, &tm);
#endif

        std::strftime(
            cached_timestamp,
            sizeof(cached_timestamp),
            "%Y-%m-%d %H:%M:%S",
            &tm
        );
    }

    return cached_timestamp;
}

}

void Logger::info(
    std::string_view message,
    const std::source_location& location)
{
    log(LogLevel::Info, message, location);
}

void Logger::warn(
    std::string_view message,
    const std::source_location& location)
{
    log(LogLevel::Warning, message, location);
}

void Logger::error(
    std::string_view message,
    const std::source_location& location)
{
    log(LogLevel::Error, message, location);
}

void Logger::log(
    LogLevel level,
    std::string_view message,
    const std::source_location& location)
{
    std::cout
        << "[" << to_string(level) << "] "
        << get_timestamp()
        << " | "
        << base_name(location.file_name())
        << ":" << location.line()
        << " | ";

    std::cout.write(message.data(),
                    static_cast<std::streamsize>(message.size()));

    std::cout << '\n';
}

}
