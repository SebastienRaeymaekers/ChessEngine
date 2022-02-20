#include "Board.hpp"
#include "MoveGeneration.hpp"

#include <ostream>
#include <cassert>
#include <cmath>
#include <memory>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include "NegaMax.hpp"
#include "Board.hpp"
#include "Board.hpp"
//#include "ValueVisitor.hpp"

Board::Board()
{
    turn_ = PieceColor::White;
    castlingright_ = CastlingRights::None;
    captures = std::stack<std::pair<Piece,Move>>();
    promotions = std::stack<std::pair<PieceType,Move>>();
    enPassantSquares = std::stack<std::pair<Square,Move>>();
    castlings = std::stack<std::pair<Move,Move>>();
}

void Board::setPiece(const Square& square, const Piece::Optional& piece) {
    if(!piece.has_value()) return;
    Piece pieceNoOpt = (Piece) *piece;
    std::shared_ptr<Piece> piecePtr = std::make_shared<Piece>(pieceNoOpt);
    pieceMap_[square.index()] = piecePtr;
}

Piece::Optional Board::piece(const Square& square) const {
    if(!pieceMap_.count(square.index())) return std::nullopt;
    std::shared_ptr<Piece> res = pieceMap_.at(square.index());
    return *res;
}

void Board::setTurn(PieceColor turn) {
    turn_ = turn;
}

PieceColor Board::turn() const {
    return turn_;
}

std::map< int, std::shared_ptr< Piece > > Board::pieceMap() const
{
    return pieceMap_;
}

void Board::setCastlingRights(CastlingRights cr) {
    castlingright_ = cr;
}

void Board::addCastlingRights(CastlingRights cr){
    castlingright_ |= cr;
}

void Board::removeCastlingRights(CastlingRights cr){
    castlingright_ &= ~cr;
}

CastlingRights Board::castlingRights() const {
    return castlingright_;
}

void Board::setEnPassantSquare(Square::Optional square)
{
    enPassantSquare_ = square;
}

Square::Optional Board::enPassantSquare() const {
    return enPassantSquare_;
}

void Board::setGeneratedMovesBoardColor(std::shared_ptr<MoveVec> ptr){
    generatedMovesBoardColor = ptr;
}

std::shared_ptr<Board::MoveVec> Board::getGeneratedMovesBoardColor() const{
    return generatedMovesBoardColor;
}

void Board::setGeneratedMovesOtherColor(std::shared_ptr<MoveVec> ptr){
    generatedMovesOtherColor = ptr;
}

std::shared_ptr<Board::MoveVec> Board::getGeneratedMovesOtherColor() const{
    return generatedMovesOtherColor;
}

void Board::makeMove(const Move& move) {    
    // piece to move
    Piece pieceToMove = (Piece)* pieceMap_.at(move.from().index());
    PieceType pieceToMoveType = pieceToMove.type(); // optional promotion piece type
    
    /* promotion */
    // if promotion move => set promotionPieceType
    if(move.promotion() != std::nullopt){ 
        //push promotion on stack
        promotions.push(std::make_pair(pieceToMove.type(), move));
        //change type of piece to move
        pieceToMoveType = (PieceType)* move.promotion();
    }
    
    /* en passant */
    //if move is to enPassantSquare and pieceToMove is pawn => capture opposing pawn
    if(pieceToMove.type() == PieceType::Pawn && move.to() == (Square)* enPassantSquare() 
        /*&& move.from().rank() == ((Square)* enPassantSquare()).rank()*/){ //TODO: more check needed
        //std::cout << "in enPassant";
        //push capture on stack
        //captures.push(std::make_pair(*pieceMap().at(move.from().rank() * 8 + move.to().file()), move)); //TODO: ERROR HERE
        //capture piece on index with same rank as from and same file as to
        pieceMap_.erase(move.from().rank() * 8 + move.to().file());
    }
    // enPassantSquare was set => remove after this move
    if(enPassantSquare() != std::nullopt) setEnPassantSquare(std::nullopt);

    // set en passant square for next move: check if moved piece is pawn and has moved to placed from startrank
    if(pieceToMove.type() == PieceType::Pawn && ( (move.from().rank() == 1 && move.to().rank() == 3) || (move.from().rank() == 6 && move.to().rank() == 4) )){
        if(pieceToMove.color() == PieceColor::White) setEnPassantSquare(Square::fromCoordinates(move.to().file(),move.to().rank()-1));
        if(pieceToMove.color() == PieceColor::Black) setEnPassantSquare(Square::fromCoordinates(move.to().file(),move.to().rank()+1));
        //push en passant square on stack
        //std::cout << "enpassent on move: " << move;
        //std::cout << "enPassantSquare: " << (Square)* enPassantSquare();
        enPassantSquares.push(std::make_pair((Square)* enPassantSquare(), move));
    }
    
    /* regular checks */
    //check regular capture
    if(pieceMap().count(move.to().index())) { // if piece on move.to()
        Piece pieceCaptured = (Piece)* pieceMap_.at(move.to().index());
        if(pieceCaptured.color() != pieceToMove.color()){
            captures.push(std::make_pair(pieceCaptured, move)); // push to captures stack
            pieceMap_.erase(move.from().index()); // remove captured piece from piecemap
        }
    }
    // set moving piece to
    setPiece(move.to(), Piece(pieceToMove.color(), pieceToMoveType)); // ,(PieceType)* move.promotion()));
    // erase moving piece from
    pieceMap_.erase(move.from().index());
    
    /* castling */
    // perform castling
    // if move moves king two vertical spaces => castling happens, thus move rook
    if(pieceToMove.type() == PieceType::King && abs((int) move.from().file() - (int) move.to().file()) == 2){
        int oldRookOffsetWRTKing = 1; int newRookOffsetWRTking = -1; // moved right
        if((int)(move.from().file() - move.to().file()) > 0){ oldRookOffsetWRTKing = -2; newRookOffsetWRTking = 1;} // moved left 
        Piece rook = *pieceMap_.at(move.to().index()+oldRookOffsetWRTKing);
        setPiece((Square)* Square::fromIndex(move.to().index()+newRookOffsetWRTking), rook); // move rook
        pieceMap_.erase(move.to().index()+oldRookOffsetWRTKing); // remove old rook
        //push rook move alongside this move on castling stack
        castlings.push(std::make_pair(Move((Square)* Square::fromIndex(move.to().index()+oldRookOffsetWRTKing),
                                           (Square)* Square::fromIndex(move.to().index()+newRookOffsetWRTking)), move));
    }
    // update castling rights
    updateCastlingRights(pieceToMove, move);
    
    
    //switch turn
    setTurn(!turn());
}

void Board::reverseMove(const Move& move){
    //std::cout << "reverse move----";
    // piece to unmove
    Piece pieceToMove = (Piece)* pieceMap_.at(move.to().index());

    /* promotion */
    // if move had promotion => unperform promotion: set piece with type from top of stack and remove from stack
    if(move.promotion() != std::nullopt && !promotions.empty() && std::get<1>(promotions.top()) == move){
        setPiece(move.from(), Piece(pieceToMove.color(),std::get<0>(promotions.top()))); 
        pieceMap_.erase(move.to().index()); // erase moving piece
        promotions.pop();
    }
    /* en passant */
    /*else if(!enPassantSquares.empty() && std::get<1>(enPassantSquares.top()) == move){
        setPiece(move.from(),pieceToMove);    
        pieceMap_.erase(move.to().index()); // erase piece that was captured en passant
    }*/
    /* castling */
    // if move had castling =>
    else if(!castlings.empty() && std::get<1>(castlings.top()) == move){
        setPiece(move.from(), pieceToMove); // reset king
        pieceMap_.erase(move.to().index()); // erase moving piece
        Move castlingMove = std::get<0>(castlings.top());
        if(pieceMap_.count(castlingMove.to().index())){
            Piece otherPieceToMove = (Piece)* pieceMap_.at(castlingMove.to().index());
            setPiece(castlingMove.from(), otherPieceToMove); //reset other piece
            pieceMap_.erase(castlingMove.to().index()); // erase moving piece
            castlings.pop();
        }
        // TODO reset castlingrights
    }
    else{ // unmove piece normally
        setPiece(move.from(),pieceToMove);    
        pieceMap_.erase(move.to().index()); // erase moving piece
    }
        
        
    //unperform capture
    if(!captures.empty() && std::get<1>(captures.top()) == move && pieceMap().count(move.from().index())){ //check top of captures stack for move
        // unperform en passant capture (if just reset piece is pawn and move.to was enPassantSquare)
        std::optional<Piece> piece = *pieceMap().at(move.from().index());
        if( ((Piece)* piece).type() == PieceType::Pawn && !enPassantSquares.empty() && 
            std::get<0>(enPassantSquares.top()) == move.to()){
        //if(false){
            //std::cout << "in reset en passant capture";
            //std::cout << "piece: " << move.from().index();
            //reset capture piece
            if(turn() == PieceColor::Black){
                setPiece((Square)* Square::fromCoordinates(move.to().file(), move.to().rank()-1), std::get<0>(captures.top()));
                //std::cout << "move.to x,y: " << move.to().file() << " " << move.to().rank()-1;
            }
            else setPiece((Square)* Square::fromCoordinates(move.to().file(), move.to().rank()+1), std::get<0>(captures.top()));
        }
        // unperform normal capture    
        else //reset capture piece */
            setPiece(move.to(), std::get<0>(captures.top())); //reset capture piece
        // remove captured piece from captures
        captures.pop();
    }
    
    /* en passant */
    // if move set an en passant square => unset it and 
    if(!enPassantSquares.empty() && std::get<1>(enPassantSquares.top()) == move){
        enPassantSquares.pop();
        if(!enPassantSquares.empty()) setEnPassantSquare(std::get<0>(enPassantSquares.top()));
        else setEnPassantSquare(std::nullopt);
    }
    if(enPassantSquare() != std::nullopt) setEnPassantSquare(std::nullopt);
    
    //switch turn
    setTurn(!turn());
}

void Board::updateCastlingRights(Piece pieceToMove, const Move& move){
    // Rule 1: king and rook can not have moved already OK
    // Rule 2: king moves two spaces vertically: if 2 to the left, rook to the right of that and vise versa OK
    // Rule 3: king can't castle when in check OR if king goes 'through' a check (meaning if one square to the
    //         left is in check => no leftside castling and vise versa) OR if king lands on check after castling. NOK
    // Rule 4: no pieces in between king and rook. OK
    
    /* Black */
    // king and rook can not have moved already
    // if black king moves = remove black castling rights
    if(pieceToMove.type() == PieceType::King && pieceToMove.color() == PieceColor::Black && !blackKingHasMoved){
        removeCastlingRights(CastlingRights::Black); 
        blackKingHasMoved = true;
    }
    // if black rook moves
    if(pieceToMove.type() == PieceType::Rook && pieceToMove.color() == PieceColor::Black){
        if(move.from().file() < 5 && !blackLeftRookHasMoved){
            removeCastlingRights(CastlingRights::BlackQueenside); // left rook removes queenside rights
            blackLeftRookHasMoved = true;
        }
        else{ 
            removeCastlingRights(CastlingRights::BlackKingside); // right rook removes kingside rights
            blackRightRookHasMoved = true;
        }
    }
    
    bool pieceInBetweenWhiteKingAndRightRook = pieceInBetween((Square)* Square::fromIndex(4), (Square)* Square::fromIndex(7));
    // if pieces in between king and right rook
    if(pieceInBetweenWhiteKingAndRightRook) removeCastlingRights(CastlingRights::WhiteKingside);
    // if NO pieces in between king and right rook and has queen side rigths 
    if(!pieceInBetweenWhiteKingAndRightRook && !blackKingHasMoved && !blackRightRookHasMoved) addCastlingRights(CastlingRights::BlackKingside);
    
    bool pieceInBetweenWhiteKingAndLeftRook = pieceInBetween((Square)* Square::fromIndex(4), (Square)* Square::fromIndex(0));
    // if pieces in between king and right rook
    if(pieceInBetweenWhiteKingAndLeftRook) removeCastlingRights(CastlingRights::WhiteQueenside);
    // if NO pieces in between king and right rook and has queen side rigths 
    if(!pieceInBetweenWhiteKingAndLeftRook && !blackKingHasMoved && !blackLeftRookHasMoved) addCastlingRights(CastlingRights::BlackKingside);
    

    /* White */
    // king and rook can not have moved already
    // if white king moves = remove white castling rights
    if(pieceToMove.type() == PieceType::King && pieceToMove.color() == PieceColor::White && !whiteKingHasMoved){
        removeCastlingRights(CastlingRights::White); 
        whiteKingHasMoved = true;
    }
    // if white rook moves
    if(pieceToMove.type() == PieceType::Rook && pieceToMove.color() == PieceColor::White){
        if(move.from().file() < 5 && !whiteLeftRookHasMoved){
            removeCastlingRights(CastlingRights::WhiteQueenside); // left rook removes queenside rights
            whiteLeftRookHasMoved = true;
        }
        else{ 
            removeCastlingRights(CastlingRights::WhiteKingside); // right rook removes kingside rights
            whiteRightRookHasMoved = true;
        }
    }
    
    bool pieceInBetweenBlackKingAndRightRook = pieceInBetween((Square)* Square::fromIndex(60), (Square)* Square::fromIndex(63));
    // if pieces in between king and right rook
    if(pieceInBetweenBlackKingAndRightRook) removeCastlingRights(CastlingRights::WhiteKingside);
    // if NO pieces in between king and right rook and has queen side rigths 
    if(!pieceInBetweenBlackKingAndRightRook && !whiteKingHasMoved && !whiteRightRookHasMoved) addCastlingRights(CastlingRights::WhiteKingside);
    
    bool pieceInBetweenBlackKingAndLeftRook = pieceInBetween((Square)* Square::fromIndex(60), (Square)* Square::fromIndex(56));
    // if pieces in between king and right rook
    if(pieceInBetweenBlackKingAndLeftRook) removeCastlingRights(CastlingRights::WhiteQueenside);
    // if NO pieces in between king and right rook and has queen side rigths 
    if(!pieceInBetweenBlackKingAndLeftRook && !blackKingHasMoved && !whiteLeftRookHasMoved) addCastlingRights(CastlingRights::WhiteQueenside);
    
    
    /* check rules */
    // check if king of next move in check
    /*if(isCheck()){
        if(turn() == PieceColor::White) removeCastlingRights(CastlingRights::White);
        else removeCastlingRights(CastlingRights::White);
    }*/
    // check if rook in check
    /*if(pieceToMove.type() == PieceType::King && abs((int) move.from().file() - (int) move.to().file()) == 2){
        if(Board::pieceInCapturingZone(*this, )){
    
    }*/
}

bool Board::castlingRightsHave(CastlingRights cr) const {
    return (castlingRights() & cr) != CastlingRights::None;
}

bool Board::pieceInBetween(Square square1, Square square2){
    int factor = 1;
    if(square1.index() < square2.index()) factor = -1;
    for(int i = 0; i < std::abs( (int) (square1.index() - square2.index())); i++){
        if(pieceMap().count(square1.index()+(factor*i))) return true;
    }
    return false;
}

void Board::pseudoLegalMoves(MoveVec& moves) const {
    MoveGeneration::generatePseudoLegalMoves(*this, moves);
}

void Board::pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const{
    MoveGeneration::generatePseudoLegalMoves(*this, moves, from);
}

void Board::printPossibleMoves(const Board& board, std::set<Move> generatedMoves) const{
    std::vector<Square> moves;
    
    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            bool movePrinted = false;
            for(auto move : generatedMoves){
                if((int)move.to().rank() == i && (int)move.to().file() == j){
                std::cout << " o "; movePrinted = true; break;
                }
            }
            if(pieceMap_.count(i * 8 + j)){
                if(!movePrinted){
                std::shared_ptr<Piece> res = board.pieceMap().at(i * 8 + j);
                std::cout << " " << *res << " ";}
            }
            else if(!movePrinted) std::cout << " . ";
        }
        std::cout << "\n";
    }
}

int Board::pieceValue(PieceType type) const
{
    switch(type){
        case PieceType::Pawn:   return 10;
        case PieceType::Knight: return 30;
        case PieceType::Bishop: return 30;
        case PieceType::Rook:   return 50;
        case PieceType::Queen:  return 90;
        default: return 0;
    }
}

int Board::evaluate(MoveVec& generatedMovesBoardColor, MoveVec& generatedMovesOtherColor)
{
    // Currently counting all piece scores. Implement better one later ex: Center control etc.
    int myPieceValues = 0;
    int otherPieceValues = 0;
    int* evalValue = nullptr;
    for (auto const& [key, val] : pieceMap()){
        Piece piece = *val;
        evalValue = piece.color() == turn() ? &myPieceValues : &otherPieceValues;
        *evalValue = *evalValue + pieceValue(piece.type());
    }
    
    //if(turn() == PieceColor::White) std::cout << " turn: White";
    //else std::cout << " turn: Black";
    
    // penalize board for being in check and return worst possible score for being checkmate
    if(isCheck(generatedMovesOtherColor)){
        *evalValue = *evalValue - 1000;
        if(isCheckMate(generatedMovesBoardColor, generatedMovesOtherColor)){ /*std::cout << "in checkmate";*/ return - std::numeric_limits<int>::max();}
    }

    int finalEval = myPieceValues - otherPieceValues;
    //int factor = turn() == PieceColor::White ? 1 : -1;
    return finalEval;
}

/*bool Board::sortByScore(Move& move1, Move& move2) {
    return (move1.score(*this) < move2.score(*this));
}*/

struct CustomMove
{
    Move move; int score;
    CustomMove(Move move, int score) : move(move), score(score) {}
    bool operator < (CustomMove move2) const{ return (score < move2.score); }
};

Board::MoveVec Board::orderMoves(MoveVec& generatedMoves)
{
    for(Move move: generatedMoves){ 
        // save piece that moves & potential captured piece
        if(!pieceMap().count((int) move.from().index())) continue;
        Piece movePiece = *(pieceMap().at((int) move.from().index()));
        std::optional<Piece> capturedPiece;
        if(pieceMap().count((int)move.to().index())) capturedPiece = *(pieceMap().at((int) move.to().index()));
        makeMove(move);
        int score = move.score(*this, movePiece, capturedPiece);
        move.setScore(score);
        reverseMove(move);
    }
    //std::cout << "unordered moves: \n"; 
    //for(Move move : generatedMoves) std::cout << move;
    // fill custom sorter vector
    std::vector<CustomMove> vec;
    for(Move move : generatedMoves){
        if(move.getScore() > 100000000 || move.getScore() < -100000000) vec.push_back(CustomMove(move, 0));
        else vec.push_back(CustomMove(move, move.getScore()));
    }
    // sort custom sorter vector
    std::sort(vec.begin(), vec.end());
    // refill generatedMoves with sorted generatedMoves
    generatedMoves.clear();
    for(CustomMove move : vec) generatedMoves.push_back(move.move);
    //std::reverse(generatedMoves.begin(), generatedMoves.end());
    //std::cout << "\n ordered moves: \n"; 
    //for(Move move : generatedMoves) std::cout << move << " score: " << move.getScore() << " ";
    return generatedMoves;
}

bool Board::pieceInCapturingZone(Board board, int index){    
    // check possible moves of opposing color
    board.setTurn(!board.turn());
    Board::MoveVec moves;
    board.pseudoLegalMoves(moves);
    board.setTurn(!board.turn());
    // check if given index is in any moves.to
    for(Move move: moves){
        if((int) move.to().index() == index) return true;
    }
    return false;
}

std::vector<int> Board::findPieceIndices(PieceType pieceType, PieceColor color) const{
    std::vector<int> indices = std::vector<int>();
    for (auto const& [key, val] : pieceMap()){
        std::shared_ptr<Piece> piecePtr = val;
        if(*piecePtr == Piece(color, pieceType))
            indices.push_back(key);
    }
    return indices;
}

std::optional<int> Board::findKingIndex(PieceColor color) const{
    std::optional<int> index;
    for (auto const& [key, val] : pieceMap()){
        std::shared_ptr<Piece> piecePtr = val;
        if(*piecePtr == Piece(color, PieceType::King))
            index = key;
    }
    return index;
}

std::vector<int> Board::getMoveToIndices(MoveVec& moves) const{
    std::vector<int> possibleMovesindices = std::vector<int>();
    for(auto move: moves) 
        possibleMovesindices.push_back(move.to().index());
    return possibleMovesindices;
}

std::vector<int> Board::calculateIntersection(std::vector<int>& vector1, std::vector<int>& vector2){
    std::vector<int> result;
    std::sort(vector1.begin(), vector1.end());
    std::sort(vector2.begin(), vector2.end());
    std::set_intersection(vector1.begin(),vector1.end(), vector2.begin(),vector2.end(), back_inserter(result));
    return result;
}

bool Board::isCheck(MoveVec& generatedMovesOtherColor, std::optional< int > kingIndex, std::optional< PieceColor > color)
{    
    // set optional turn of board
    PieceColor turnOfBoard = turn();
    if(color != std::nullopt) turnOfBoard = (PieceColor)* color;
    
    // find king on board if no index given
    if(kingIndex != std::nullopt) kingIndex = (int)* kingIndex;
    else {
        std::optional<int> index = findKingIndex(turnOfBoard);
        if(index != std::nullopt) kingIndex = (int)* index;
        else return false;
    }
    //if(kingIndex == std::nullopt) return false;
    // get possible moves of all opposing pieces and store the move.to indices
    std::vector<int> possibleOtherPiecesMovesIndices = std::vector<int>();
    MoveVec moves = generatedMovesOtherColor;
    //if(getGeneratedMovesOtherColor() == nullptr) return false;
    //else moves = *getGeneratedMovesOtherColor();
    for(Move move: moves) possibleOtherPiecesMovesIndices.push_back((int) move.to().index());
    
    // if king index in attackedindices => return true
    if(std::find(possibleOtherPiecesMovesIndices.begin(), possibleOtherPiecesMovesIndices.end(), (int)* kingIndex) != possibleOtherPiecesMovesIndices.end())
        return true;
    return false;
}

bool Board::isStaleMate(MoveVec& generatedMovesOtherColor, std::optional< PieceColor > color){
    (void) generatedMovesOtherColor;
    //(void) color;
    // set optional turn of board
    PieceColor turnOfBoard = turn();
    if(color != std::nullopt) turnOfBoard = (PieceColor)* color;
    (void) turnOfBoard;

    /* check if king is in check in current position */
    /*if(isCheck(generatedMovesOtherColor)) return false;

    // find king on board
    int kingIndex = (int)* findKingIndex(turnOfBoard);
    Board::MoveVec kingMoves;
    pseudoLegalMovesFrom(*Square::fromIndex(kingIndex), kingMoves);
    if(kingMoves.size() == 0) return false;
    for(Move move: kingMoves){
        makeMove(move);
        Board::MoveVec otherPiecesMoves;
        pseudoLegalMoves(otherPiecesMoves);
        if(!isCheck(otherPiecesMoves, std::nullopt, !turn())) return false;
        reverseMove(move);
    }
    //return true;*/
    return false;
}

bool Board::isCheckMate(MoveVec& generatedMovesBoardColor, MoveVec& generatedMovesOtherColor)
{
    //(void) color;
    
    /* check if king is in check in current position */
    if(!isCheck(generatedMovesOtherColor)) return false;
        
    /* check if piece of same color can prevent the king being in check */
    bool nonCheckMovePossible = false;
    MoveVec originalOpposingColorMoves = generatedMovesOtherColor;
    MoveVec movesOpposingColor = MoveVec();
    // try every move you can make and check if any move is in check
    for(Move move: generatedMovesBoardColor){
        //std::cout << "my moves: ";
        // make the move
        makeMove(move);
        //for(Move move: generatedMovesBoardColor) std::cout << move << ", ";
        //std::cout << "\n ";
        //NegaMax::printBoardWithPossibleMoves(*this, *getGeneratedMovesBoardColor());

        // generate moves other can make as a response
        pseudoLegalMoves(movesOpposingColor);
        //std::cout << "other moves: " << move;
        //for(Move move: movesOpposingColor) std::cout << move << ", ";
        //std::cout << "\n ";
        //NegaMax::printBoardWithPossibleMoves(*this, movesOpposingColor);
        
        //setGeneratedMovesOtherColor(std::make_shared<MoveVec>(movesOpposingColor));
        generatedMovesOtherColor = movesOpposingColor;
        //std::cout << "is check?: " << isCheck(generatedMovesOtherColor, std::nullopt, !turn());
        if(!isCheck(generatedMovesOtherColor, std::nullopt, !turn())) nonCheckMovePossible = true;
        reverseMove(move);
        if(nonCheckMovePossible){
            //setGeneratedMovesOtherColor(std::make_shared<MoveVec>(originalOpposingColorMoves));
            generatedMovesOtherColor = originalOpposingColorMoves;
            return false;
        }
        movesOpposingColor.clear();
    }
    generatedMovesOtherColor = originalOpposingColorMoves;
    //setGeneratedMovesOtherColor(std::make_shared<MoveVec>(originalOpposingColorMoves));
    return true;
}

std::ostream& operator<<(std::ostream& os, const Board& board) {

    //std::cout << "\n";
    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            //std::cout << "x: " << j << " y: " << i; 
            if(board.pieceMap().count(i * 8 + j)){
                std::shared_ptr<Piece> res = board.pieceMap().at(i * 8 + j);
                os << " " << *res << " ";
            }
            else os << " . ";
        }
        os << "\n";
    }
    /*std::cout << "piecemap: \n" ;
    for (auto const& [key, val] : board.pieceMap()){
        std::cout << "key: " << key << " val: " << *val << " ";
    }*/
    //std::cout << "\n \n \n" ;
    //std::cout << "\n \n \n" ;
    return os;
}



