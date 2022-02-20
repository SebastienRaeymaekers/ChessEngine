#include "PrincipalVariation.hpp"

#include <ostream>
#include <vector>
#include <iostream>

PrincipalVariation::PrincipalVariation(std::vector<Move>& moves, const Board& board){
    moves_ = moves;
    board_ = std::make_shared<Board>(board);
    isMate_ = false;
    bestMove = std::nullopt;
}

Board PrincipalVariation::board() const {
    return *board_;
}

bool PrincipalVariation::isMate() const {
    return isMate_;
}

void PrincipalVariation::setIsMate(bool isMateVal){
    isMate_ = isMateVal;
}

std::vector<Move> PrincipalVariation::getMoves(){
    return moves_;
}

void PrincipalVariation::appendMove(Move move){
    moves_.push_back(move);
}

void PrincipalVariation::enQueueMove(Move move){
    moves_.insert(moves_.begin(), move);
}

int PrincipalVariation::score() const {    
    Board testBoard = board();
    
    if(isMate()) return 0;

    //if(length() == 1) testBoard.makeMove(*begin());
    /*else if(length() != 0){
        for (auto it = begin(); it != end(); ++it){
            Move move = *it;
            testBoard.makeMove(move);
        }
    }*/
    // generate pseudo legal moves for own color
    Board::MoveVec generatedMovesBoardColor = Board::MoveVec();
    testBoard.pseudoLegalMoves(generatedMovesBoardColor);
    
    // generate pseudo legal moves for opposing color
    Board::MoveVec generatedMovesOtherColor = Board::MoveVec();
    testBoard.pseudoLegalMoves(generatedMovesOtherColor);
    
    if(testBoard.isStaleMate(generatedMovesOtherColor)) return 0;

    return testBoard.evaluate(generatedMovesBoardColor, generatedMovesOtherColor);
}

std::size_t PrincipalVariation::length() const {
    return moves_.size();
}

PrincipalVariation::MoveIter PrincipalVariation::begin() const {
    return &moves_.front();
}
              
PrincipalVariation::MoveIter PrincipalVariation::end() const {
    return &moves_.back();
}


std::ostream& operator<<(std::ostream& os, const PrincipalVariation& pv) {
    if(pv.begin() == nullptr) return os;
    os << (pv.begin())->from();
    for (auto it = pv.begin(); it != pv.end(); ++it){
        os << "--->" << (*it).to();
    }
    os << "--->" << (pv.end())->to();
    return os;
}
