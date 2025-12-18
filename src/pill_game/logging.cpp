//
// Date       : 18/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/logging.h"

namespace pill_game::logging {

namespace {

constexpr std::string_view log_levels[] = {
    "Info",
    "Trace",
    "Warn",
    "Err",
};

std::string_view truncate_left(std::string_view str, size_t len) {
    if (str.size() <= len) {
        return str;
    }
    return str.substr(str.size() - len);
}

}  // namespace

void log(LogLevel level, const std::source_location& loc, const std::string& message) noexcept {
    std::string msg = std::format(
        "[{:>5}] | {}\n",
        // NOLINTNEXTLINE
        log_levels[static_cast<size_t>(level)],
        message
    );
    std::cout << msg;
}

}  // namespace pill_game::logging