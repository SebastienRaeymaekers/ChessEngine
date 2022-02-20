#include "ChessEngine.hpp"
#include "PrincipalVariation.hpp"
#include "NegaMax.hpp"
#include "TimeInfo.hpp"
#include <iostream>


std::string ChessEngine::name() const {
    return "ChessEngine";
}

std::string ChessEngine::version() const {
    return "1.0";
}

std::string ChessEngine::author() const {
    return "Sébastien Raeymaekers";
}

void ChessEngine::newGame() {
    return;
}

PrincipalVariation ChessEngine::pv(const Board& board, const TimeInfo::Optional& timeInfo) {
    (void) timeInfo;
    std::vector<Move> pvMoves = std::vector<Move>();
    PrincipalVariation pv = PrincipalVariation(pvMoves, board);
    std::cout << "board: \n" << board;
    //Board b = const_cast<Board&>(board);
    //if(timeInfo != std::nullopt) NegaMax::iterativeDeepening(board, - std::numeric_limits<int>::infinity(), std::numeric_limits<int>::infinity(), pv, timeInfo);
    //else NegaMax::negaMax(board, 3, - std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), pv);
    
    time_t endTime = time(nullptr) + 180;
    NegaMax::iterativeDeepening(board, - std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), endTime, pv);
    
    std::cout << "-----------" << '\n'; 
    return pv;
}


