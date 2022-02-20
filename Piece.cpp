#include "Piece.hpp"

#include <ostream>

Piece::Piece(PieceColor color, PieceType type)
{
    color_ = color;
    type_ = type;
}

Piece::Optional Piece::fromSymbol(char symbol) {
    PieceColor pieceColor = PieceColor::Black;
    if (isupper(symbol)){ 
        pieceColor = PieceColor::White;
        symbol = tolower(symbol);
    }
    auto pieceTypeOpt = Piece::charToPieceType(symbol);
    if(!pieceTypeOpt.has_value()) return std::nullopt;
    PieceType pieceType = (PieceType) *Piece::charToPieceType(symbol);
    return Piece(pieceColor, pieceType);
}

std::optional<PieceType> Piece::charToPieceType(char symbol){
        switch (symbol){
        case 'p':
            return PieceType::Pawn;
        case 'n':
            return PieceType::Knight;
        case 'b':
            return PieceType::Bishop;
        case 'r':
            return PieceType::Rook;
        case 'q':
            return PieceType::Queen;
        case 'k':
            return PieceType::King;
    }
    return {};
}

PieceColor Piece::color() const {
    return color_;
}

PieceType Piece::type() const {
    return type_;
}

int Piece::zobristIndexOf()
{
    if (type()==PieceType::Pawn && color() == PieceColor::White)
        return 0;
    if (type()==PieceType::Knight && color() == PieceColor::White)
        return 1;
    if (type()==PieceType::Bishop && color() == PieceColor::White)
        return 2;
    if (type()==PieceType::Rook && color() == PieceColor::White)
        return 3;
    if (type()==PieceType::Queen && color() == PieceColor::White)
        return 4;
    if (type()==PieceType::King && color() == PieceColor::White)
        return 5;
    if (type()==PieceType::Pawn && color() == PieceColor::Black)
        return 6;
    if (type()==PieceType::Knight && color() == PieceColor::Black)
        return 7;
    if (type()==PieceType::Bishop && color() == PieceColor::Black)
        return 8;
    if (type()==PieceType::Rook && color() == PieceColor::Black)
        return 9;
    if (type()==PieceType::Queen && color() == PieceColor::Black)
        return 10;
    if (type()==PieceType::King && color() == PieceColor::Black)
        return 11;
    else
        return -1;
}

bool operator==(const Piece& lhs, const Piece& rhs) {
    return lhs.color() == rhs.color() && lhs.type() == rhs.type();
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    bool white = false;
    if (piece.color() == PieceColor::White) white = true;
    switch (piece.type()){
        case PieceType::Pawn:
            return white ? os << 'P': os << 'p';
        case PieceType::Knight:
            return white ? os << 'N': os << 'n';
        case PieceType::Bishop:
            return white ? os << 'B': os << 'b';
        case PieceType::Rook:
            return white ? os << 'R': os << 'r';
        case PieceType::Queen:
            return white ? os << 'Q': os << 'q';
        case PieceType::King:
            return white ? os << 'K': os << 'k';
    }
    return os << "";
}

PieceColor operator!(PieceColor color) {
    return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
}
