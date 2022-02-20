#ifndef CHESS_ENGINE_PRINCIPALVARIATION_HPP
#define CHESS_ENGINE_PRINCIPALVARIATION_HPP

#include "Move.hpp"
#include "Piece.hpp"
#include "Board.hpp"

#include <iosfwd>
#include <cstddef>
#include <vector>

class PrincipalVariation {
public:

    using MoveIter = const Move*; //std::shared_ptr<Move>;

    PrincipalVariation(std::vector<Move>& pvMoves, const Board& board);
    
    Board board() const;
    
    bool isMate() const;
    void setIsMate(bool isMateVal);
    
    int score() const;

    std::size_t length() const;
    std::vector<Move> getMoves();
    void appendMove(Move move);
    MoveIter begin() const;
    MoveIter end() const;
    
    void enQueueMove(Move move);
    
    std::optional<Move> bestMove;
    
private:
    std::vector<Move> moves_;
    std::shared_ptr<Board> board_;
    bool isMate_;
};

std::ostream& operator<<(std::ostream& os, const PrincipalVariation& pv);

#endif
