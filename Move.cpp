#include "Move.hpp"
#include "Board.hpp"

#include <ostream>
#include <iostream>
#include <memory>
#include <algorithm>
#include "NegaMax.hpp"

Move::Move(const Square& from, const Square& to,
           const std::optional<PieceType>& promotion)
{
    from_ = std::make_shared<Square>(from);
    to_ = std::make_shared<Square>(to);
    promotion_ = promotion;
}

Move::Optional Move::fromUci(const std::string& uci) {
    if(uci.length() > 5 || uci.length()< 4) return std::nullopt;
    Square::Optional from = Square::fromName(std::string(1,uci[0])+uci[1]);
    Square::Optional to = Square::fromName(std::string(1,uci[2])+uci[3]);
    if(!from.has_value() || !to.has_value()) return std::nullopt;
    if(uci.length() == 5){
        std::optional<PieceType> promotion = Piece::charToPieceType(uci[4]);
        if(!promotion.has_value()) return std::nullopt;
        return Move((Square) *from, (Square) *to, promotion);
    }
    return Move((Square) *from, (Square) *to);
}

Square Move::from() const {
    return *from_.get();
}

Square Move::to() const {
    return *to_.get();
}

std::optional<PieceType> Move::promotion() const {
    return promotion_;
}

int Move::getScore(){
    return score_;
}

void Move::setScore(int score){
    score_ = score;
}

int Move::score(Board board, Piece movePiece, std::optional<Piece> capturedPiece){
    int score = 0;

    // Capturing valuable pieces with less valuable ones gives a higher score.
    if(capturedPiece != std::nullopt && board.pieceMap().count(to().index())){
        Piece capturedPiece = *(board.pieceMap().at(to().index()));
        score = score + board.pieceValue(capturedPiece.type()) + (board.pieceValue(capturedPiece.type()) - board.pieceValue(movePiece.type()));
    }
    
    // Causing a check gives a higher score
    Board::MoveVec generatedMovesOtherColor = Board::MoveVec();
    NegaMax::generatePseudoLegalMoves(board, generatedMovesOtherColor, true, std::nullopt);
    if(board.isCheck(generatedMovesOtherColor)) score = score + 1000;
    
    /*
    // Moving to square which is in range of opposing piece gives lower score.
    for(auto const& [key, val] : board.pieceMap()){ // For all pieces
        Piece piece = (*val);
        if(piece.color() != movePiece.color()){ // If piece has other color
            Board::MoveVec generatedMoves;
            board.pseudoLegalMovesFrom(to(), generatedMoves); // Generate all its moves
            std::vector<Square> attackingSquares = std::vector<Square>(); // Collect squares they land on
            for(Move move: generatedMoves){
                attackingSquares.push_back(move.to());
            } // Check if move.to is in squares they land on
            if (std::find(attackingSquares.begin(), attackingSquares.end(), to()) != attackingSquares.end()) {
                score = score - board.pieceValue(movePiece.type());
            }
        }
    }
    //score_ = score;*/
    return score;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    if (!move.promotion().has_value()) 
        return os << move.from() << move.to();
    else if(move.promotion() == PieceType::Pawn) 
        return os << move.from() << move.to() << 'p';
    else if(move.promotion() == PieceType::Knight) 
        return os << move.from() << move.to() << 'n';
    else if(move.promotion() == PieceType::Bishop) 
        return os << move.from() << move.to() << 'b';
    else if(move.promotion() == PieceType::Rook) 
        return os << move.from() << move.to() << 'r';
    else if(move.promotion() == PieceType::Queen) 
        return os << move.from() << move.to() << 'q';
    else 
        return os << move.from() << move.to() << 'k';
}


bool operator<(const Move& lhs, const Move& rhs) {
    return lhs.from().index() + lhs.to().index() < rhs.from().index() + rhs.to().index();
    //return lhs.getScore() < rhs.getScore(); (results in failed promotion tests)
}

bool operator==(const Move& lhs, const Move& rhs) {
    return lhs.to() == rhs.to() && lhs.from() == rhs.from() && lhs.promotion() == rhs.promotion();
    //return lhs.getScore() == rhs.getScore(); (results in failed promotion tests)
}
