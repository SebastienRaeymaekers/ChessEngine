#ifndef CHESS_ENGINE_CASTLINGRIGHTS_HPP
#define CHESS_ENGINE_CASTLINGRIGHTS_HPP

#include <iosfwd>

enum class CastlingRights {
    None           = 0,
    WhiteKingside  = 1 << 0,
    WhiteQueenside = 1 << 1,
    BlackKingside  = 1 << 2,
    BlackQueenside = 1 << 3,
    White = WhiteKingside | WhiteQueenside,
    Black = BlackKingside | BlackQueenside,
    All = White | Black
};

CastlingRights operator&(CastlingRights lhs, CastlingRights rhs);
CastlingRights& operator&=(CastlingRights& lhs, CastlingRights rhs);
CastlingRights operator|(CastlingRights lhs, CastlingRights rhs);
CastlingRights& operator|=(CastlingRights& lhs, CastlingRights rhs);
CastlingRights operator~(CastlingRights cr);

std::ostream& operator<<(std::ostream& os, CastlingRights cr);

#endif
