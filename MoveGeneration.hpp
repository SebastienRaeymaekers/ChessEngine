#ifndef CHESS_ENGINE_MOVEGENERATION_HPP
#define CHESS_ENGINE_MOVEGENERATION_HPP

#include "Piece.hpp"
#include "Square.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "CastlingRights.hpp"

#include <optional>
#include <iosfwd>
#include <vector>
#include <memory>
#include <map>

class MoveGeneration {
public:
    using MoveVec = std::vector<Move>;
    static void generatePseudoLegalMoves(const Board& board, MoveVec& moves, const std::optional<Square>& from = std::nullopt);
    static void generatePieceMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    static void generatePawnMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    static void generateKingMoves(Board board, Square startSquare, Piece piece, MoveVec& moves);
    static void generateQueenMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    static void generateRookMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    static void generateKnightMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    static void generateBishopMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves);
    
private:

};


#endif
