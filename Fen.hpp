#ifndef CHESS_ENGINE_FEN_HPP
#define CHESS_ENGINE_FEN_HPP

#include "Board.hpp"

#include <string>
#include <iosfwd>

namespace Fen {
    extern const char* const StartingPos;

    Board::Optional createBoard(std::istream& fenStream);
    Board::Optional createBoard(const std::string& fen);
}

#endif
