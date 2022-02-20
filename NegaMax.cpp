#include "NegaMax.hpp"
#include "Move.hpp"

#include <ostream>
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <sys/time.h>
#include <random>

enum class FlagType {
    LOWERBOUND,
    UPPERBOUND,
    EXACT
};

struct BoardStruct {
    int depth;
    int eval;
    int alpha;
    int beta;
    std::optional<Move> bestMove;
    FlagType flag; 
    BoardStruct(int depth, int eval, int alpha, int beta, std::optional<Move> bestMove, FlagType flag) 
    : depth(depth), eval(eval), alpha(alpha), beta(beta), bestMove(bestMove), flag(flag) {}
};

int NegaMax::negaMax(Board board, int depth, int alpha, int beta, time_t endTime, PrincipalVariation& pv, std::optional<Square> from){
    std::cout << "---------------"; 
    std::cout << "\n in Negamax \n"; 
    std::cout << "---------------\n"; 
    /*
    int alphaOrig = alpha;
    // transposition table lookup
    unsigned long long int zobristHash = NegaMax::computeZobristHash(zobristTable, board);
    if(boardStructMap.count(zobristHash)){
        BoardStruct bs = *boardStructMap.at(zobristHash);
        if(bs.flag == FlagType::EXACT) return bs.eval;
        else if(bs.flag == FlagType::LOWERBOUND)
            alpha = std::max(alpha, bs.eval);
        else if(bs.flag == FlagType::UPPERBOUND)
            beta = std::min(beta, bs.eval);
        if(alpha >= beta) return bs.eval;
    }*/
    
    // generate pseudo legal moves for own color
    Board::MoveVec generatedMovesBoardColor = Board::MoveVec();
    NegaMax::generatePseudoLegalMoves(board, generatedMovesBoardColor, false, from);
    
    // generate pseudo legal moves for opposing color
    Board::MoveVec generatedMovesOtherColor = Board::MoveVec();
    NegaMax::generatePseudoLegalMoves(board, generatedMovesOtherColor, true, from);

    // if no move generated => return lowest possible value
    if(board.isCheckMate(generatedMovesBoardColor, generatedMovesOtherColor)){ /*std::cout << "IS CHECKMATE--------\n";*/
        pv.setIsMate(true);
        return - std::numeric_limits<int>::max();
    }
    if(board.isStaleMate(generatedMovesOtherColor)){ //TODO: is true when starting from startpos
        std::cout << "IS STALEMATE--------\n";
        return 0; 
    }; 

    // order generated legal moves from best to worst to speed up alpha beta pruning
    board.orderMoves(generatedMovesBoardColor); // OK but slower
    
    // perform negamax algorithm
    int value = - std::numeric_limits<int>::max();
    int bestValue = - std::numeric_limits<int>::max();
    if(generatedMovesBoardColor.size() == 0) return 0; 
    Move bestMove = generatedMovesBoardColor.at(0);
    //printBoardWithPossibleMoves(board, generatedMovesBoardColor);
    //for(Move move: generatedMovesBoardColor) std::cout << "move: " << move;
    for(Move move: generatedMovesBoardColor){
        if(time(nullptr) > endTime) break;
        //std::cout << "\n move: " << move << '\n';
        // make move
        board.makeMove(move);
        // eval move
        int eval = - negamaxSearch(board, depth-1, - beta, -alpha, endTime, pv);
        //std::cout << "|-score-|: " << eval;
        // take best eval
        value = std::max(value, eval);
        // reverse move
        board.reverseMove(move);
        // perform alpha beta pruning
        alpha = std::max(alpha, value);  
        if(alpha > beta) break;
        //std::cout << " bestValue: " << bestValue << " value: " << value;
        // keep best move
        if(value > bestValue){
            bestValue = value;
            bestMove = move;
        }
    }
    /*
    // add board to boardStructMap
    BoardStruct bs = BoardStruct(depth, bestValue, alpha, beta, pv.bestMove, FlagType::EXACT);    
    if(bestValue <= alphaOrig) bs = BoardStruct(depth, bestValue, alpha, beta, pv.bestMove, FlagType::UPPERBOUND);
    else if (bestValue >= beta) bs = BoardStruct(depth, bestValue, alpha, beta, pv.bestMove, FlagType::LOWERBOUND);
    boardStructMap[zobristHash] = std::make_shared<BoardStruct>(bs);*/
    
    std::cout << "\n printing best move: " << bestMove << '\n'; 
    board.makeMove(bestMove);
    std::cout << "board: \n" << board;
    board.reverseMove(bestMove);
    std::cout << "after reverse move";
    // add best move to pv
    //pv.insert(v.begin(), 6);
    pv.enQueueMove(bestMove);
    return bestValue;
}

int NegaMax::negamaxSearch(Board board, int depth, int alpha, int beta, time_t endTime, PrincipalVariation& pv, const std::optional< Square > from)
{
    /*
    int alphaOrig = alpha;
    // transposition table lookup
    unsigned long long int zobristHash = NegaMax::computeZobristHash(zobristTable, board);
    if(boardStructMap.count(zobristHash)){
        BoardStruct bs = *boardStructMap.at(zobristHash);
        if(bs.flag == FlagType::EXACT) return bs.eval;
        else if(bs.flag == FlagType::LOWERBOUND)
            alpha = std::max(alpha, bs.eval);
        else if(bs.flag == FlagType::UPPERBOUND)
            beta = std::min(beta, bs.eval);
        if(alpha >= beta) return bs.eval;
    }*/
    
    // generate pseudo legal moves for own color
    Board::MoveVec generatedMovesBoardColor = Board::MoveVec();
    NegaMax::generatePseudoLegalMoves(board, generatedMovesBoardColor, false, from);
    
    // reject the previous move if the other color was in check by giving the board the highest scores
    //std::cout << "previous board in check: " << board.isCheck(generatedMovesBoardColor, std::nullopt, !board.turn());
    if(board.isCheck(generatedMovesBoardColor, std::nullopt, !board.turn())){
        return std::numeric_limits<int>::max();
    }
    
    // generate pseudo legal moves for opposing color
    Board::MoveVec generatedMovesOtherColor = Board::MoveVec();
    NegaMax::generatePseudoLegalMoves(board, generatedMovesOtherColor, true, from);
    
    // if no move generated => return lowest possible value
    if(board.isCheckMate(generatedMovesBoardColor, generatedMovesOtherColor)){ std::cout << "IS CHECKMATE--------\n";
        pv.setIsMate(true);
        return - std::numeric_limits<int>::max();
    }
    if(board.isStaleMate(generatedMovesOtherColor)){ //TODO: is true when starting from startp
        std::cout << "IS STALEMATE--------\n";
        return 0;
    }; 
    
    //std::cout << "in negamaxSearch \n";
    if(depth == 0){ // || board.hasNoChildren())
        //std::cout << "Eval Board: \n" << board;
        //std::cout << " Board Score: " << board.evaluate() << '\n';
        //std::cout << "-----------------\n";
        return board.evaluate(generatedMovesBoardColor, generatedMovesOtherColor);
    }

    // order generated legal moves from best to worst to speed up alpha beta pruning
    board.orderMoves(generatedMovesBoardColor); // OK but slower
    
    //for(Move move: generatedLegalMoves) std::cout << "move: " << move << ",";
    // perform negamax algorithm
    //int value = - std::numeric_limits<int>::max();
    if(generatedMovesBoardColor.size() == 0) return 0;
    Move bestMove = generatedMovesBoardColor.at(0);
    int eval = - std::numeric_limits<int>::max();
    for(Move move: generatedMovesBoardColor){
        if(time(nullptr) > endTime) return alpha;
        //std::cout << "\n move: " << move << '\n';
        // make move
        board.makeMove(move);
        // eval move
        eval = - negamaxSearch(board, depth-1, - beta, -alpha, endTime, pv);
        // reverse move
        board.reverseMove(move);
        // perform alpha beta pruning
        if(eval >= beta) return beta;
        alpha = std::max(alpha, eval);  
    }
    /*
    // add board to boardStructMap
    BoardStruct bs = BoardStruct(depth, eval, alpha, beta, pv.bestMove, FlagType::EXACT);    
    if(eval <= alphaOrig) bs = BoardStruct(depth, eval, alpha, beta, pv.bestMove, FlagType::UPPERBOUND);
    else if (eval >= beta) bs = BoardStruct(depth, eval, alpha, beta, pv.bestMove, FlagType::LOWERBOUND);
    boardStructMap[zobristHash] = std::make_shared<BoardStruct>(bs);
    */
    return alpha;
}

int NegaMax::iterativeDeepening(Board board, int alpha, int beta, time_t endTime, PrincipalVariation& pv, const std::optional<Square> from)
{   
    (void) from;
    int value = 0;
    
    //if(timeInfo != std::nullopt){
    PlayerTimeInfo pti;
    (void) pti;
    //if(board.turn() == PieceColor::White) pti = timeInfo.white;
    //else pti = timeInfo.black;
    /*unsigned long long int zobristTable[64][12];
    NegaMax::initZobristTable(zobristTable);
    std::map<unsigned long long int, std::shared_ptr<BoardStruct>> boardStructMap = std::map<unsigned long long int, std::shared_ptr<BoardStruct>>();*/
    
    int depth = 1;
    while(depth < 50){
        std::cout << "\n DEPTH: " << depth << '\n';
        value = negaMax(board, depth, alpha, beta, endTime, pv);
        if(time(nullptr) > endTime) break;
        if(pv.isMate()) break;
        depth++;
    }
    return value;
}


void NegaMax::generatePseudoLegalMoves(Board& board, Board::MoveVec& generatedMoves, bool changeColor, std::optional<Square> from){
    // set color for which moves will be generated on the given board
    if(changeColor) board.setTurn(!board.turn());
    // call pseudoLegalMoves depending on optional from
    if(from != std::nullopt) board.pseudoLegalMovesFrom((Square)* from, generatedMoves);
    else board.pseudoLegalMoves(generatedMoves);
    // reset color for which moves had to be generated on the given board
    if(changeColor) board.setTurn(!board.turn());
}

void NegaMax::printBoardWithPossibleMoves(Board& board, Board::MoveVec& generatedMoves){
    using MoveSet = std::set<Move>;
    auto generatedMovesSet = MoveSet(generatedMoves.begin(), generatedMoves.end());
    std::cout << "board: \n" << board;
    std::cout << "Possible moves: \n"; board.printPossibleMoves(board, generatedMovesSet);
    std::cout << "-------------------------------" << '\n';
}

void NegaMax::initZobristTable(unsigned long long int (&zobristTable)[64][12]){
    std::mt19937 mt(01234567);
    for (int index = 0; index<64; index++){
        for (int k = 0; k<12; k++){
        std::uniform_int_distribution<unsigned long long int> dist(0, UINT64_MAX);
          zobristTable[index][k] = dist(mt);;
      }
    }
}

long long unsigned int NegaMax::computeZobristHash(long long unsigned int (&zobristTable)[64][12], Board& board)
{
    unsigned long long int hash = 0;
    for (auto const& [key, val] : board.pieceMap()){
        Piece piece = *val;
        int pieceIndex = piece.zobristIndexOf();
        hash ^= zobristTable[key][pieceIndex];
    }
    return hash;
}













