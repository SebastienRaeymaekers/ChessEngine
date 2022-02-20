#include "Fen.hpp"

#include "Square.hpp"

#include <sstream>
#include <iostream>

const char* const Fen::StartingPos =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static std::string nextField(std::istream& stream) {
    auto field = std::string();
    stream >> field;
    return field;
}

static bool parsePlacement(const std::string& placement, Board& board) {
    auto currentFile = Square::Coordinate(0);
    auto currentRank = Square::Coordinate(7);

    for (auto c: placement) {
        if (c >= '1' && c <='8') {
            auto offset = c - '0';
            currentFile += offset;

            if (currentFile > 8) {
                return false;
            }
        } else if (c == '/') {
            if (currentRank == 0) {
                return false;
            }

            currentFile = 0;
            currentRank--;
        } else {
            auto optPiece = Piece::fromSymbol(c);

            if (!optPiece.has_value()) {
                return false;
            }

            auto optSquare = Square::fromCoordinates(currentFile, currentRank);

            if (!optSquare.has_value()) {
                return false;
            }

            board.setPiece(optSquare.value(), optPiece.value());
            currentFile++;
        }
    }

    return true;
}

static bool parseTurn(const std::string& turn, Board& board) {
    if (turn == "w") {
        board.setTurn(PieceColor::White);
        return true;
    } else if (turn == "b") {
        board.setTurn(PieceColor::Black);
        return true;
    } else {
        return false;
    }
}

static bool parseCastlingRights(const std::string& rights, Board& board) {
    if (rights == "-") {
        // No rights
        return true;
    }

    auto cr = CastlingRights::None;

    for (auto right : rights) {
        switch (right) {
            case 'K': cr |= CastlingRights::WhiteKingside; break;
            case 'Q': cr |= CastlingRights::WhiteQueenside; break;
            case 'k': cr |= CastlingRights::BlackKingside; break;
            case 'q': cr |= CastlingRights::BlackQueenside; break;
            default: return false;
        }
    }

    board.setCastlingRights(cr);
    return true;
}

static bool parseEnPassantSquare(const std::string& ep, Board& board) {
    if (ep == "-") {
        // No en passant square
        return true;
    }

    auto square = Square::fromName(ep);

    if (square.has_value()) {
        board.setEnPassantSquare(square);
        return true;
    } else {
        return false;
    }
}

Board::Optional Fen::createBoard(std::istream& fenStream) {
    auto placement = nextField(fenStream);

    if (placement.empty()) {
        return std::nullopt;
    }

    auto board = Board();

    if (!parsePlacement(placement, board)) {
        return std::nullopt;
    }

    auto turn = nextField(fenStream);

    if (turn.empty()) {
        return std::nullopt;
    }

    if (!parseTurn(turn, board)) {
        return std::nullopt;
    }

    auto castlingRights = nextField(fenStream);

    if (castlingRights.empty()) {
        return std::nullopt;
    }

    if (!parseCastlingRights(castlingRights, board)) {
        return std::nullopt;
    }

    auto ep = nextField(fenStream);

    if (ep.empty()) {
        return std::nullopt;
    }

    if (!parseEnPassantSquare(ep, board)) {
        return std::nullopt;
    }

    auto halfmove = nextField(fenStream);

    if (halfmove.empty()) {
        return std::nullopt;
    }

    auto fullmove = nextField(fenStream);

    if (fullmove.empty()) {
        return std::nullopt;
    }

    return board;
}

Board::Optional Fen::createBoard(const std::string& fen) {
    auto fenStream = std::stringstream(fen);
    return createBoard(fenStream);
}
