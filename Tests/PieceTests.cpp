#include "catch2/catch.hpp"

#include "Piece.hpp"

#include <sstream>
#include <cctype>

TEST_CASE("Pieces store color and type correctly", "[Piece][Fundamental]") {
    auto color = PieceColor::Black;
    auto type = PieceType::Queen;
    auto piece = Piece(color, type);
    REQUIRE(piece.color() == color);
    REQUIRE(piece.type() == type);
}

TEST_CASE("Pieces can be created from valid symbols", "[Piece][Fundamental]") {
    auto [blackSymbol, pieceType] = GENERATE(table<char, PieceType>({
        {'p', PieceType::Pawn},
        {'n', PieceType::Knight},
        {'b', PieceType::Bishop},
        {'r', PieceType::Rook},
        {'q', PieceType::Queen},
        {'k', PieceType::King}
    }));

    auto optBlackPiece = Piece::fromSymbol(blackSymbol);
    REQUIRE(optBlackPiece.has_value());

    auto createdBlackPiece = optBlackPiece.value();
    REQUIRE(createdBlackPiece == Piece(PieceColor::Black, pieceType));

    auto whiteSymbol = static_cast<char>(std::toupper(blackSymbol));
    auto optWhitePiece = Piece::fromSymbol(whiteSymbol);
    REQUIRE(optWhitePiece.has_value());

    auto createdWhitePiece = optWhitePiece.value();
    REQUIRE(createdWhitePiece == Piece(PieceColor::White, pieceType));
}

TEST_CASE("Pieces are not created from invalid symbols", "[Piece][Fundamental]") {
    auto symbol = GENERATE('x', 'Z');

    auto optPiece = Piece::fromSymbol(symbol);
    REQUIRE_FALSE(optPiece.has_value());
}

TEST_CASE("Pieces stream their symbol correctly", "[Piece][Fundamental]") {
    auto [blackSymbol, pieceType] = GENERATE(table<char, PieceType>({
        {'p', PieceType::Pawn},
        {'n', PieceType::Knight},
        {'b', PieceType::Bishop},
        {'r', PieceType::Rook},
        {'q', PieceType::Queen},
        {'k', PieceType::King}
    }));

    auto blackStream = std::stringstream();
    auto blackPiece = Piece(PieceColor::Black, pieceType);
    blackStream << blackPiece;
    auto blackSymbolStr = std::string(1, blackSymbol);
    REQUIRE(blackStream.str() == blackSymbolStr);

    auto whiteStream = std::stringstream();
    auto whitePiece = Piece(PieceColor::White, pieceType);
    whiteStream << whitePiece;
    auto whiteSymbol = static_cast<char>(std::toupper(blackSymbol));
    auto whiteSymbolStr = std::string(1, whiteSymbol);
    REQUIRE(whiteStream.str() == whiteSymbolStr);
}

TEST_CASE("PieceColor can be inverted correctly", "[Piece][Fundamental]") {
    REQUIRE((!PieceColor::White) == PieceColor::Black);
    REQUIRE((!PieceColor::Black) == PieceColor::White);
}
