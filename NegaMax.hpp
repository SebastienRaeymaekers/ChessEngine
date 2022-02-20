#ifndef CHESS_ENGINE_NEGAMAX_HPP
#define CHESS_ENGINE_NEGAMAX_HPP

#include "Board.hpp"
#include "TimeInfo.hpp"

#include <optional>
#include <iosfwd>
#include <string>
#include "PrincipalVariation.hpp"
#include <optional>

struct BoardStruct;

class NegaMax {
public:
    static int negaMax(Board board, int depth, int alpha, int beta, time_t endTime, PrincipalVariation& pv, std::optional<Square> from = std::nullopt);
    static int negamaxSearch(Board board, int depth, int alpha, int beta, time_t endTime, PrincipalVariation& pv, const std::optional<Square> from = std::nullopt);
    static int iterativeDeepening(Board board, int alpha, int beta, time_t endTime, PrincipalVariation& pv, const std::optional<Square> from = std::nullopt);
    
    static void generatePseudoLegalMoves(Board& board, Board::MoveVec& generatedMoves, bool changeColor, std::optional<Square> from = std::nullopt);
    static void filterLegalMovesFromPseudoLegalMoves(Board& board, Board::MoveVec& generatedMoves, Board::MoveVec& generatedLegalMoves);
    static void printBoardWithPossibleMoves(Board& board, Board::MoveVec& generatedMoves);
    
    static void initZobristTable(unsigned long long int (&zobristTable)[64][12]);
    static unsigned long long int computeZobristHash(unsigned long long int (&zobristTable)[64][12], Board& board);
    
    //static time_t currentTime;
    //static time_t endTime;
};

#endif
