#include "CastlingRights.hpp"

#include <type_traits>
#include <ostream>

CastlingRights operator&(CastlingRights lhs, CastlingRights rhs) {
    using T = std::underlying_type_t<CastlingRights>;
    return static_cast<CastlingRights>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

CastlingRights& operator&=(CastlingRights& lhs, CastlingRights rhs) {
    lhs = lhs & rhs;
    return lhs;
}

CastlingRights operator|(CastlingRights lhs, CastlingRights rhs) {
    using T = std::underlying_type_t<CastlingRights>;
    return static_cast<CastlingRights>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

CastlingRights& operator|=(CastlingRights& lhs, CastlingRights rhs) {
    lhs = lhs | rhs;
    return lhs;
}

CastlingRights operator~(CastlingRights cr) {
    using T = std::underlying_type_t<CastlingRights>;
    return static_cast<CastlingRights>(~static_cast<T>(cr));
}

std::ostream& operator<<(std::ostream& os, CastlingRights cr) {
    if (cr == CastlingRights::None) {
        return os << "-";
    } else {
        if ((cr & CastlingRights::WhiteKingside) != CastlingRights::None) {
            os << "K";
        }

        if ((cr & CastlingRights::WhiteQueenside) != CastlingRights::None) {
            os << "Q";
        }

        if ((cr & CastlingRights::BlackKingside) != CastlingRights::None) {
            os << "k";
        }

        if ((cr & CastlingRights::BlackQueenside) != CastlingRights::None) {
            os << "q";
        }

        return os;
    }
}
