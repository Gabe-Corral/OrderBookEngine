#pragma once

#include <source_location>
#include <string_view>

namespace ob::util {

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void info(
        std::string_view message,
        const std::source_location& location = std::source_location::current());

    static void warn(
        std::string_view message,
        const std::source_location& location = std::source_location::current());

    static void error(
        std::string_view message,
        const std::source_location& location = std::source_location::current());

private:
    static void log(
        LogLevel level,
        std::string_view message,
        const std::source_location& location);
};

}
