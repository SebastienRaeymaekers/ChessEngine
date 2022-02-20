#ifndef CHESS_ENGINE_BOARD_HPP
#define CHESS_ENGINE_BOARD_HPP

#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"
#include "CastlingRights.hpp"

#include <optional>
#include <iosfwd>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <stack>  

class ValueVisitor;

class Board {
public:

    using Optional = std::optional<Board>;
    using MoveVec = std::vector<Move>;

    Board();

    void setPiece(const Square& square, const Piece::Optional& piece);
    Piece::Optional piece(const Square& square) const;
    void setTurn(PieceColor turn);
    PieceColor turn() const;
    std::map<int,std::shared_ptr<Piece>> pieceMap() const;
    void setCastlingRights(CastlingRights cr);
    void addCastlingRights(CastlingRights cr);
    void removeCastlingRights(CastlingRights cr);
    CastlingRights castlingRights() const;
    void setEnPassantSquare(Square::Optional square);
    Square::Optional enPassantSquare() const;
    
    void makeMove(const Move& move);
    void reverseMove(const Move& move);

    void updateCastlingRights(Piece pieceToMove, const Move& move);
    bool castlingRightsHave(CastlingRights cr) const;
    bool pieceInBetween(Square square1, Square square2);
    
    MoveVec moves;
    void pseudoLegalMoves(MoveVec& moves) const;
    void pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const;
    
    void printPossibleMoves(const Board& board, std::set<Move> generatedMoves) const;
    
    int pieceValue(PieceType type) const;
    int evaluate(MoveVec& generatedMovesBoardColor, MoveVec& generatedMovesOtherColor);
        
    MoveVec orderMoves(MoveVec& generatedMoves);
    bool sortByScore(Move& move1, Move& move2);
    
    bool isCheck(MoveVec& generatedMovesOtherColor, std::optional< int > kingIndex = std::nullopt, std::optional< PieceColor > color = std::nullopt);
    bool isCheckMate(MoveVec& generatedMovesBoardColor, MoveVec& generatedMovesOtherColor);
    bool isStaleMate(MoveVec& generatedMovesOtherColor, std::optional< PieceColor > color = std::nullopt);
    
    std::stack<std::pair<Piece,Move>> captures;
    std::stack<std::pair<PieceType,Move>> promotions;
    std::stack<std::pair<Move,Move>> castlings;
    std::stack<std::pair<Square,Move>> enPassantSquares;
    
    bool whiteKingHasMoved = false;
    bool whiteLeftRookHasMoved = false;
    bool whiteRightRookHasMoved = false;
    
    bool blackKingHasMoved = false;
    bool blackLeftRookHasMoved = false;
    bool blackRightRookHasMoved = false;
    
    static bool pieceInCapturingZone(Board board, int index);
    
    std::vector<int> findPieceIndices(PieceType pieceType, PieceColor color) const;
    std::optional<int> findKingIndex(PieceColor color) const;
    std::vector<int> getMoveToIndices(MoveVec& moves) const;
    std::vector<int> calculateIntersection(std::vector<int>& vector1, std::vector<int>& vector2);
    
    void setGeneratedMovesBoardColor(std::shared_ptr<MoveVec> ptr);
    std::shared_ptr<MoveVec> getGeneratedMovesBoardColor() const;
    void setGeneratedMovesOtherColor(std::shared_ptr<MoveVec> ptr);
    std::shared_ptr<MoveVec> getGeneratedMovesOtherColor() const;
    
private:
    std::map<int,std::shared_ptr<Piece>> pieceMap_;
    PieceColor turn_;
    std::shared_ptr<std::optional<Piece>> piece_;
    Square::Optional enPassantSquare_;
    CastlingRights castlingright_;
    std::shared_ptr<MoveVec> generatedMovesBoardColor;
    std::shared_ptr<MoveVec> generatedMovesOtherColor;
};

std::ostream& operator<<(std::ostream& os, const Board& board);

#endif
