#ifndef CHESS_CHESSENGINE_BOARD_HPP
#define CHESS_CHESSENGINE_BOARD_HPP

#include "Engine.hpp"
#include "PrincipalVariation.hpp"
#include <string>
#include "TimeInfo.hpp"

class ChessEngine : public Engine {
public:
    ~ChessEngine() = default;

    std::string name() const;
    std::string version() const;
    std::string author() const;

    void newGame();
    PrincipalVariation pv(const Board& board, const TimeInfo::Optional& timeInfo = std::nullopt);
};


#endif
