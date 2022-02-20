#ifndef CHESS_ENGINE_MOVE_HPP
#define CHESS_ENGINE_MOVE_HPP

#include "Square.hpp"
#include "Piece.hpp"

#include <iosfwd>
#include <optional>
#include <string>
#include <memory>

class Board;

class Move {
public:

    using Optional = std::optional<Move>;

    Move(const Square& from, const Square& to,
         const std::optional<PieceType>& promotion = std::nullopt);

    static Optional fromUci(const std::string& uci);

    Square from() const;
    Square to() const;
    std::optional<PieceType> promotion() const;
    
    int score(Board board, Piece movePiece, std::optional<Piece> capturedPiece = std::nullopt);
    
    int getScore();
    void setScore(int score);
    int score_;
    bool captureMove;
    
private:
    std::shared_ptr<Square> from_;
    std::shared_ptr<Square> to_;
    std::optional<PieceType> promotion_;
};

std::ostream& operator<<(std::ostream& os, const Move& move);

// Needed for std::map, std::set
bool operator<(const Move& lhs, const Move& rhs);
bool operator==(const Move& lhs, const Move& rhs);

#endif
