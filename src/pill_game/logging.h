//
// Date       : 18/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include <string>
#include <cstdint>
#include <source_location>

namespace pill_game::logging {

using std::uint8_t;

// NOLINTNEXTLINE
enum LogLevel : uint8_t {
    Info = 0,
    Trace,
    Warn,
    Err,
};

void log(LogLevel level, const std::source_location& loc, const std::string& message) noexcept;

#define PG_LOG(level, ...) (             \
    ::pill_game::logging::log(           \
        level,                           \
        std::source_location::current(), \
        std::format(__VA_ARGS__)         \
    )                                    \
)

}  // namespace pill_game::logging
