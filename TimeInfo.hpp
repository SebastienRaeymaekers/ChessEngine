#ifndef CHESS_ENGINE_TIMEINFO_HPP
#define CHESS_ENGINE_TIMEINFO_HPP

#include <optional>
#include <chrono>

struct PlayerTimeInfo {
    std::chrono::milliseconds timeLeft;
    std::chrono::milliseconds increment;
};

struct TimeInfo {
    using Optional = std::optional<TimeInfo>;

    PlayerTimeInfo white;
    PlayerTimeInfo black;
    std::optional<unsigned> movesToGo;
};

#endif
