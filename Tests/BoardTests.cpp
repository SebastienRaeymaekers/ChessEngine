#include "catch2/catch.hpp"

#include "TestUtils.hpp"

#include "Board.hpp"
#include "Square.hpp"
#include "Fen.hpp"

#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <iostream>
#include <NegaMax.hpp>

TEST_CASE("A default-constructed board is empty", "[Board][Fundamental]") {
    auto board = Board();

    for (auto i = 0; i < 64; ++i) {
        auto optSquare = Square::fromIndex(i);
        REQUIRE(optSquare.has_value());

        auto square = optSquare.value();
        auto optPiece = board.piece(square);
        REQUIRE_FALSE(optPiece.has_value());
    }
}

TEST_CASE("Pieces can be set on a board", "[Board][Fundamental]") {
    auto board = Board();

    auto optSquare = Square::fromIndex(41);
    REQUIRE(optSquare.has_value());

    auto square = optSquare.value();
    auto piece = Piece(PieceColor::White, PieceType::Rook);
    board.setPiece(square, piece);

    auto optSetPiece = board.piece(square);
    REQUIRE(optSetPiece.has_value());

    auto setPiece = optSetPiece.value();
    REQUIRE(setPiece == piece);

    SECTION("Setting a piece on an occupied square overrides") {
        auto newPiece = Piece(PieceColor::Black, PieceType::King);
        board.setPiece(square, newPiece);

        auto optNewSetPiece = board.piece(square);
        REQUIRE(optNewSetPiece.has_value());

        auto newSetPiece = optNewSetPiece.value();
        REQUIRE(newSetPiece == newPiece);
    }
}

TEST_CASE("The turn can be set on a board", "[Board][Fundamental]") {
    auto color = GENERATE(PieceColor::Black, PieceColor::White);

    auto board = Board();
    board.setTurn(color);
    REQUIRE(board.turn() == color);
}

static void testPseudoLegalMoves(
    const char* fen,
    const std::string& fromName,
    const std::vector<std::string>& expectedTargets)
{
    auto optBoard = Fen::createBoard(fen);
    REQUIRE(optBoard.has_value());

    auto from = Square::Optional();

    if (!fromName.empty()) {
        from = Square::fromName(fromName);
        REQUIRE(from.has_value());
    }

    auto board = optBoard.value();

    using MoveSet = std::set<Move>;
    auto expectedMoves = MoveSet();

    for (auto targetName : expectedTargets) {
        if (targetName.length() == 2) {
            // targetName is a square
            auto optTarget = Square::fromName(targetName);
            REQUIRE(optTarget.has_value());
            REQUIRE(from.has_value());

            auto target = optTarget.value();
            auto move = Move(from.value(), target);
            expectedMoves.insert(move);
        } else {
            // targetName is a move
            auto optMove = Move::fromUci(targetName);
            REQUIRE(optMove.has_value());

            expectedMoves.insert(optMove.value());
        }
    }

    auto generatedMovesVec = Board::MoveVec();

    if (from.has_value()) {
        board.pseudoLegalMovesFrom(from.value(), generatedMovesVec);
    } else {
        auto t1 = std::chrono::high_resolution_clock::now();
        board.pseudoLegalMoves(generatedMovesVec);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        std::cout << "Time pseudoLegalMoves in ms: " << ms_double.count() << " " << '\n';
    }

    auto generatedMoves = MoveSet(generatedMovesVec.begin(),
                                  generatedMovesVec.end());
    
    auto unexpectedMoves = MoveSet();
    auto notGeneratedMoves = MoveSet();

    std::set_difference(
        generatedMoves.begin(), generatedMoves.end(),
        expectedMoves.begin(), expectedMoves.end(),
        std::inserter(unexpectedMoves, unexpectedMoves.begin())
    );

    std::set_difference(
        expectedMoves.begin(), expectedMoves.end(),
        generatedMoves.begin(), generatedMoves.end(),
        std::inserter(notGeneratedMoves, notGeneratedMoves.begin())
    );
    
    std::cout << "-------------------------------" << '\n';
    std::cout << "-------------------------------" << '\n';
    std::cout << "-------------------------------" << '\n';
    std::cout << "Board: " << '\n' << board;
    std::cout << "-------------------------------" << '\n';
    //std::cout << "Board Evaluation: " << board.evaluate()  << '\n';
    std::cout << "-------------------------------" << '\n';
    std::cout << "Possible moves: "; board.printPossibleMoves(board, generatedMoves);
    std::cout << "-------------------------------" << '\n';
    
    /*
    if(!generatedMoves.empty() && from.has_value()){
        std::vector<Move> pv = std::vector<Move>();
        //Move bestNextMove = Move((*generatedMoves.begin()).from(),(*generatedMoves.begin()).from(),(*generatedMoves.begin()).promotion());
        auto t1 = std::chrono::high_resolution_clock::now();
        int bestValue = NegaMax::negamaxSearch(board, 1, - std::numeric_limits<int>::infinity(), std::numeric_limits<int>::infinity(), pv, from.value());
        if(!pv.empty()){
            std::cout << "Best move: " << *pv.begin() << " Best value: " << bestValue << '\n'; 
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << "Time negamaxSearch in ms: " << ms_double.count() << " " << '\n';
        }
    }*/
    
    std::cout << "-------------------------------" << '\n';
    
    CAPTURE(fen, from, unexpectedMoves, notGeneratedMoves);
    REQUIRE(generatedMoves == expectedMoves);
}

#define TEST_CASE_PSEUDO_MOVES(name, tag) \
    TEST_CASE(name, "[Board][MoveGen]" tag)

TEST_CASE_PSEUDO_MOVES("Pseudo-legal knight moves, empty board", "[Knight]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3N4/8/8/8_w_-_-_0_1
        "8/8/8/8/3N4/8/8/8 w - - 0 1",
        "d4",
        {
            "c6", "e6",
            "f5", "f3",
            "c2", "e2",
            "b3", "b5"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal knight moves, side of board", "[Knight]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/n7/8_b_-_-_0_1
        "8/8/8/8/8/8/n7/8 b - - 0 1",
        "a2",
        {
            "b4",
            "c3", "c1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal knight moves, captures", "[Knight]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/2p1P3/8/2PN4/2Pp4/8/8_w_-_-_0_1
        "8/8/2p1P3/8/2PN4/2Pp4/8/8 w - - 0 1",
        "d4",
        {
            "c6",
            "f5", "f3",
            "c2", "e2",
            "b3", "b5"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal king moves, empty board", "[King]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3K4/8/8/8_w_-_-_0_1
        "8/8/8/8/3K4/8/8/8 w - - 0 1",
        "d4",
        {
            "c5", "d5", "e5",
            "c4", "e4",
            "c3", "d3", "e3",
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal king moves, side of board", "[King]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/8/K7_w_-_-_0_1
        "8/8/8/8/8/8/8/K7 w - - 0 1",
        "a1",
        {
            "a2", "b2",
            "b1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal king moves, captures", "[King]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/2PK4/3p4/8/8_w_-_-_0_1
        "8/8/8/8/2PK4/3p4/8/8 w - - 0 1",
        "d4",
        {
            "c5", "d5", "e5",
            "e4",
            "c3", "d3", "e3",
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal bishop moves, empty board", "[Bishop]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3B4/8/8/8_w_-_-_0_1
        "8/8/8/8/3B4/8/8/8 w - - 0 1",
        "d4",
        {
            "c5", "b6", "a7",
            "e5", "f6", "g7", "h8",
            "c3", "b2", "a1",
            "e3", "f2", "g1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal bishop moves, captures", "[Bishop]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/1p3P2/8/3B4/8/8/8_w_-_-_0_1
        "8/8/1p3P2/8/3B4/8/8/8 w - - 0 1",
        "d4",
        {
            "c5", "b6",
            "e5",
            "c3", "b2", "a1",
            "e3", "f2", "g1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal rook moves, empty board", "[Rook]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3R4/8/8/8_w_-_-_0_1
        "8/8/8/8/3R4/8/8/8 w - - 0 1",
        "d4",
        {
            "d5", "d6", "d7", "d8",
            "d3", "d2", "d1",
            "c4", "b4", "a4",
            "e4", "f4", "g4", "h4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal rook moves, captures", "[Rook]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/3p4/8/3R1P2/8/8/8_w_-_-_0_1
        "8/8/3p4/8/3R1P2/8/8/8 w - - 0 1",
        "d4",
        {
            "d5", "d6",
            "d3", "d2", "d1",
            "c4", "b4", "a4",
            "e4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal queen moves, empty board", "[Queen]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3Q4/8/8/8_w_-_-_0_1
        "8/8/8/8/3Q4/8/8/8 w - - 0 1",
        "d4",
        {
            "c5", "b6", "a7",
            "e5", "f6", "g7", "h8",
            "c3", "b2", "a1",
            "e3", "f2", "g1",
            "d5", "d6", "d7", "d8",
            "d3", "d2", "d1",
            "c4", "b4", "a4",
            "e4", "f4", "g4", "h4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal queen moves, captures", "[Queen]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/1p3P2/8/1P1Q4/8/3p4/8_w_-_-_0_1
        "8/8/1p3P2/8/1P1Q4/8/3p4/8 w - - 0 1",
        "d4",
        {
            "c5", "b6",
            "e5",
            "c3", "b2", "a1",
            "e3", "f2", "g1",
            "d5", "d6", "d7", "d8",
            "d3", "d2",
            "c4",
            "e4", "f4", "g4", "h4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, step, empty board", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3P4/8/8/8_w_-_-_0_1
        "8/8/8/8/3P4/8/8/8 w - - 0 1",
        "d4",
        {
            "d5"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, step, opponent blocked", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/3p4/3P4/8/8/8_w_-_-_0_1
        "8/8/8/3p4/3P4/8/8/8 w - - 0 1",
        "d4",
        {}
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, step, self blocked", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/3p4/3b4/8/8/8_b_-_-_0_1
        "8/8/8/3p4/3b4/8/8/8 b - - 0 1",
        "d5",
        {}
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, capture left", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/2ppP3/3P4/8/8/8_w_-_-_0_1
        "8/8/8/2ppP3/3P4/8/8/8 w - - 0 1",
        "d4",
        {
            "c5"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, capture both", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/3p4/2PRQ3/8/8/8_b_-_-_0_1
        "8/8/8/3p4/2PRQ3/8/8/8 b - - 0 1",
        "d5",
        {
            "c4", "e4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, base step, empty board", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/3p4/8/8/8/8/8/8_b_-_-_0_1
        "8/3p4/8/8/8/8/8/8 b - - 0 1",
        "d7",
        {
            "d6", "d5"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, base step, blocked 1", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/3p4/3P4/8/8/8/8/8_b_-_-_0_1
        "8/3p4/3P4/8/8/8/8/8 b - - 0 1",
        "d7",
        {}
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, base step, blocked 2", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/7p/8/7P/8_w_-_-_0_1
        "8/8/8/8/7p/8/7P/8 w - - 0 1",
        "h2",
        {
            "h3"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, black left", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3Pp3/8/8/8_b_-_d3_0_1
        "8/8/8/8/3Pp3/8/8/8 b - d3 0 1",
        "e4",
        {
            "e3", "d3"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, black right", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/2pP4/2P5/8/8_b_-_d3_0_1
        "8/8/8/8/2pP4/2P5/8/8 b - d3 0 1",
        "c4",
        {
            "d3"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, white left", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/2n5/pP6/8/8/8/8_w_-_a6_0_1
        "8/8/2n5/pP6/8/8/8/8 w - a6 0 1",
        "b5",
        {
            "a6", "b6", "c6"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, white right", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/n7/Pp6/8/8/8/8_w_-_b6_0_1
        "8/8/n7/Pp6/8/8/8/8 w - b6 0 1",
        "a5",
        {
            "b6"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, black both", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/5pPp/7P/8/8_b_-_g3_0_1
        "8/8/8/8/5pPp/7P/8/8 b - g3 0 1",
        "",
        {
            "f4f3", "f4g3",
            "h4g3"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, white both", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/2r2n2/2PpP3/8/8/8/8_w_-_d6_0_1
        "8/8/2r2n2/2PpP3/8/8/8/8 w - d6 0 1",
        "",
        {
            "c5d6",
            "e5d6", "e5e6", "e5f6"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, en passant, wrong square", "[Pawn][EnPassant]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/3p1P2/8/8/8_b_-_f3_0_1
        "8/8/8/8/3p1P2/8/8/8 b - f3 0 1",
        "d4",
        {
            "d3"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, visual en passant, no square", "[Pawn]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/1pPp4/8/8/8/8_w_-_-_0_1
        "8/8/8/1pPp4/8/8/8/8 w - - 0 1",
        "c5",
        {
            "c6"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, white promotions", "[Pawn][Promotion]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/5r2/4P3/8/8/8/8/8/8_w_-_-_0_1
        "5r2/4P3/8/8/8/8/8/8 w - - 0 1",
        "e7",
        {
            "e7e8q", "e7e8r", "e7e8b", "e7e8n",
            "e7f8q", "e7f8r", "e7f8b", "e7f8n"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal pawn moves, black promotions", "[Pawn][Promotion]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/1p6/2Q5_b_-_-_0_1
        "8/8/8/8/8/8/1p6/2Q5 b - - 0 1",
        "b2",
        {
            "b2b1q", "b2b1r", "b2b1b", "b2b1n",
            "b2c1q", "b2c1r", "b2c1b", "b2c1n"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, white kingside", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/3PPP2/3QK2R_w_K_-_0_1
        "8/8/8/8/8/8/3PPP2/3QK2R w K - 0 1",
        "e1",
        {
            "f1", "g1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, white queenside", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/3PPP2/R3KB2_w_Q_-_0_1
        "8/8/8/8/8/8/3PPP2/R3KB2 w Q - 0 1",
        "e1",
        {
            "d1", "c1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, black kingside", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/3qk2r/3ppp2/8/8/8/8/8/8_b_k_-_0_1
        "3qk2r/3ppp2/8/8/8/8/8/8 b k - 0 1",
        "e8",
        {
            "f8", "g8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, black queenside", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/r3kb2/3ppp2/8/8/8/8/8/8_b_q_-_0_1
        "r3kb2/3ppp2/8/8/8/8/8/8 b q - 0 1",
        "e8",
        {
            "d8", "c8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, kingside blocked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/3PPP2/3QK1NR_w_K_-_0_1
        "8/8/8/8/8/8/3PPP2/3QK1NR w K - 0 1",
        "e1",
        {
            "f1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, queenside blocked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/rn2kb2/3ppp2/8/8/8/8/8/8_b_q_-_0_1
        "rn2kb2/3ppp2/8/8/8/8/8/8 b q - 0 1",
        "e8",
        {
            "d8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, kingside attacked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/3qk2r/3ppp2/7N/8/8/8/8/8_b_-_-_0_1
        "3qk2r/3ppp2/7N/8/8/8/8/8 b - - 0 1",
        "e8",
        {
            "f8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, queenside attacked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/5n2/3PPP2/R3KB2_w_Q_-_0_1
        "8/8/8/8/8/5n2/3PPP2/R3KB2 w Q - 0 1",
        "e1",
        {
            "d1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, queenside rook attacked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/r3kb2/3ppp2/8/8/8/5B2/8/8_b_q_-_0_1
        "r3kb2/3ppp2/8/8/8/5B2/8/8 b q - 0 1",
        "e8",
        {
            "d8", "c8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, kingside rook attacked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/2q5/8/8/8/3PPP2/3QK2R_w_K_-_0_1
        "8/8/2q5/8/8/8/3PPP2/3QK2R w K - 0 1",
        "e1",
        {
            "f1", "g1"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, queenside b-square attacked", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/r3kb2/3ppp2/8/8/8/8/8/1R6_b_q_-_0_1
        "r3kb2/3ppp2/8/8/8/8/8/1R6 b q - 0 1",
        "e8",
        {
            "d8", "c8"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal castling moves, no rights", "[Castling]") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/r3k2r/8/8/8/8/8/3PPP2/R3K2R_w_Kkq_-_0_1
        "r3k2r/8/8/8/8/8/3PPP2/R3K2R w Kkq - 0 1",
        "e1",
        {
            "f1", "g1",
            "d1"
        }
    );
}

static void testMakeMove(const char* fen, const Move& move) {
    auto board = Fen::createBoard(fen);
    REQUIRE(board.has_value());

    auto piece = board->piece(move.from());
    REQUIRE(piece.has_value());

    auto turn = board->turn();
    board->makeMove(move);
    REQUIRE(board->turn() == !turn);

    REQUIRE_FALSE(board->piece(move.from()).has_value());

    auto toPiece = board->piece(move.to());
    REQUIRE(toPiece.has_value());
    REQUIRE(piece == toPiece.value());
}

static void testMakeMove(const char* fen, const Move& move,
                         const std::map<Square, Piece>& expectedBoard) {
    CAPTURE(fen, move);

    auto board = Fen::createBoard(fen);
    REQUIRE(board.has_value());
    
    //if(board->turn() == PieceColor:: Black) std::cout << "turn: black "<< '\n';
    //else std::cout << "turn: white "<< '\n';
    //std::cout << board << '\n';
    
    auto turn = board->turn();
    board->makeMove(move);
    REQUIRE(board->turn() == !turn);
    
    //std::cout << board << '\n';
    
    using MoveSet = std::set<Move>;
    auto expectedMoves = MoveSet();

    /*
    std::cout << "Possible moves: ";
    Board::MoveVec generatedMovess = {move};
    auto generatedMoves = MoveSet(generatedMovess.begin(),
                                  generatedMovess.end());
    board->printPossibleMoves((Board)* board, generatedMoves);*/


    for (auto [square, expectedPiece] : expectedBoard) {
        auto piece = board->piece(square);
        INFO("Expected piece not on board");
        CAPTURE(square, piece, expectedPiece);
        REQUIRE(piece.has_value());
        REQUIRE(piece.value() == expectedPiece);
    }

    for (auto i = 0; i < 64; ++i) {
        auto square = Square::fromIndex(i);
        REQUIRE(square.has_value());

        auto piece = board->piece(square.value());
        CAPTURE(square.value(), piece);
        auto expectedPieceIt = expectedBoard.find(square.value());

        if (piece.has_value()) {
            INFO("Unexpected piece on board");
            REQUIRE(expectedPieceIt != expectedBoard.end());
            CAPTURE(expectedPieceIt->second);
            REQUIRE(piece.value() == expectedPieceIt->second);
        }
    }
}

static void testMakeMove(const char* fen, const Move& move, CastlingRights cr) {
    auto board = Fen::createBoard(fen);
    REQUIRE(board.has_value());

    auto turn = board->turn();
    //std::cout << board << '\n';
    board->makeMove(move);
    //std::cout << board << '\n';
    REQUIRE(board->turn() == !turn);

    REQUIRE(board->castlingRights() == cr);
}

static void testMakeMove(const char* fen, const Move& move,
                         const Square::Optional& epSquare) {
    auto board = Fen::createBoard(fen);
    REQUIRE(board.has_value());
    //std::cout << board << '\n';
    auto turn = board->turn();
    board->makeMove(move);
    REQUIRE(board->turn() == !turn);
    //std::cout << board << '\n';
    REQUIRE(board->enPassantSquare() == epSquare);
}

#define TEST_CASE_MAKE_MOVE(name, tag) \
    TEST_CASE(name, "[Board][MoveMaking]" tag)

TEST_CASE_MAKE_MOVE("Move making moves a piece on the board", "") {
    // https://lichess.org/editor/8/8/8/8/8/8/8/2Q5_w_-_-_0_1
    testMakeMove(
        "8/8/8/8/8/8/8/2Q5 w - - 0 1",
        Move(Square::C1, Square::F4)
    );
}

TEST_CASE_MAKE_MOVE("Move making supports captures", "") {
    // https://lichess.org/editor/8/8/8/2n5/8/3P4/8/8_b_-_-_0_1
    testMakeMove(
        "8/8/8/2n5/8/3P4/8/8 b - - 0 1",
        Move(Square::C5, Square::D3)
    );
}

TEST_CASE_MAKE_MOVE("Move making, white promotion", "[Promotion]") {
    // https://lichess.org/editor/8/4P3/8/8/8/8/8/8_w_-_-_0_1
    testMakeMove(
        "8/4P3/8/8/8/8/8/8 w - - 0 1",
        Move(Square::E7, Square::E8, PieceType::Queen),
        {
            {Square::E8, Piece(PieceColor::White, PieceType::Queen)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, black promotion", "[Promotion]") {
    // https://lichess.org/editor/8/4P3/8/8/8/8/8/8_w_-_-_0_1
    testMakeMove(
        "8/8/8/8/8/8/p7/8 b - - 0 1",
        Move(Square::A2, Square::A1, PieceType::Knight),
        {
            {Square::A1, Piece(PieceColor::Black, PieceType::Knight)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling white kingside", "[Castling]") {
    // https://lichess.org/editor/8/8/8/8/8/8/8/4K2R_w_K_-_0_1
    testMakeMove(
        "8/8/8/8/8/8/8/4K2R w K - 0 1",
        Move(Square::E1, Square::G1),
        {
            {Square::G1, Piece(PieceColor::White, PieceType::King)},
            {Square::F1, Piece(PieceColor::White, PieceType::Rook)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling white queenside", "[Castling]") {
    // https://lichess.org/editor/8/8/8/8/8/8/8/R3K3_w_Q_-_0_1
    testMakeMove(
        "8/8/8/8/8/8/8/R3K3 w Q - 0 1",
        Move(Square::E1, Square::C1),
        {
            {Square::C1, Piece(PieceColor::White, PieceType::King)},
            {Square::D1, Piece(PieceColor::White, PieceType::Rook)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling black kingside", "[Castling]") {
    // https://lichess.org/editor/4k2r/8/8/8/8/8/8/8_w_k_-_0_1
    testMakeMove(
        "4k2r/8/8/8/8/8/8/8 w k - 0 1",
        Move(Square::E8, Square::G8),
        {
            {Square::G8, Piece(PieceColor::Black, PieceType::King)},
            {Square::F8, Piece(PieceColor::Black, PieceType::Rook)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling black queenside", "[Castling]") {
    // https://lichess.org/editor/r3k3/8/8/8/8/8/8/8_w_q_-_0_1
    testMakeMove(
        "r3k3/8/8/8/8/8/8/8 w q - 0 1",
        Move(Square::E8, Square::C8),
        {
            {Square::C8, Piece(PieceColor::Black, PieceType::King)},
            {Square::D8, Piece(PieceColor::Black, PieceType::Rook)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling rights king move", "[Castling]") {
    // https://lichess.org/editor/r3k2r/8/8/8/8/8/8/R3K2R_w_KQkq_-_0_1
    testMakeMove(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move(Square::E1, Square::D1),
        CastlingRights::Black
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling rights rook move", "[Castling]") {
    // https://lichess.org/editor/r3k2r/8/8/8/8/8/8/R3K2R_b_KQkq_-_0_1
    testMakeMove(
        "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
        Move(Square::H8, Square::G8),
        CastlingRights::White | CastlingRights::BlackQueenside
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling rights rook capture", "[Castling]") {
    // https://lichess.org/editor/r3k2r/8/8/8/8/8/8/R3K2R_w_KQkq_-_0_1
    testMakeMove(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move(Square::A1, Square::A8),
        CastlingRights::WhiteKingside | CastlingRights::BlackKingside
    );
}

TEST_CASE_MAKE_MOVE("Move making, castling rights king back to base", "[Castling]") {
    // https://lichess.org/editor/r3k2r/8/8/8/8/8/8/R2K3R_w_kq_-_0_1
    testMakeMove(
        "r3k2r/8/8/8/8/8/8/R2K3R w kq - 0 1",
        Move(Square::D1, Square::E1),
        CastlingRights::Black
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant, black left", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/8/5Pp1/8/8/8_b_-_f3_0_1
    testMakeMove(
        "8/8/8/8/5Pp1/8/8/8 b - f3 0 1",
        Move(Square::G4, Square::F3),
        {
            {Square::F3, Piece(PieceColor::Black, PieceType::Pawn)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant, black right", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/3P4/1PpPP3/2P5/8/8_b_-_d3_0_1
    testMakeMove(
        "8/8/8/3P4/1PpPP3/2P5/8/8 b - d3 0 1",
        Move(Square::C4, Square::D3),
        {
            {Square::D3, Piece(PieceColor::Black, PieceType::Pawn)},
            {Square::B4, Piece(PieceColor::White, PieceType::Pawn)},
            {Square::C3, Piece(PieceColor::White, PieceType::Pawn)},
            {Square::D5, Piece(PieceColor::White, PieceType::Pawn)},
            {Square::E4, Piece(PieceColor::White, PieceType::Pawn)},
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant, white left", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/3pPp2/8/8/8/8_w_-_d6_0_1
    testMakeMove(
        "8/8/8/3pPp2/8/8/8/8 w - d6 0 1",
        Move(Square::E5, Square::D6),
        {
            {Square::D6, Piece(PieceColor::White, PieceType::Pawn)},
            {Square::F5, Piece(PieceColor::Black, PieceType::Pawn)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant, white right", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/1Pp5/8/8/8/8_w_-_c6_0_1
    testMakeMove(
        "8/8/8/1Pp5/8/8/8/8 w - c6 0 1",
        Move(Square::B5, Square::C6),
        {
            {Square::C6, Piece(PieceColor::White, PieceType::Pawn)}
        }
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant square, white double step update", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/8/3p4/8/4P3/8_w_-_-_0_1
    testMakeMove(
        "8/8/8/8/3p4/8/4P3/8 w - - 0 1",
        Move(Square::E2, Square::E4),
        Square::E3
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant square, black double step update", "[EnPassant]") {
    // https://lichess.org/editor/8/p7/8/1P6/8/8/8/8_b_-_-_0_1
    testMakeMove(
        "8/p7/8/1P6/8/8/8/8 b - - 0 1",
        Move(Square::A7, Square::A5),
        Square::A6
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant square, remove after capture", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/8/Pp6/8/8/8_b_-_a3_0_1
    testMakeMove(
        "8/8/8/8/Pp6/8/8/8 b - a3 0 1",
        Move(Square::B4, Square::A3),
        std::nullopt
    );
}

TEST_CASE_MAKE_MOVE("Move making, en passant square, remove after move", "[EnPassant]") {
    // https://lichess.org/editor/8/8/8/4pP2/8/8/1N6/8_w_-_e6_0_1
    testMakeMove(
        "8/8/8/4pP2/8/8/1N6/8 w - e6 0 1",
        Move(Square::B2, Square::C4),
        std::nullopt
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal moves, multiple pieces, white", "") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/8/8/8/8/8/8/2P5/NB6_w_-_-_0_1
        "8/8/8/8/8/8/2P5/NB6 w - - 0 1",
        "",
        {
            "a1b3",
            "b1a2",
            "c2c3", "c2c4"
        }
    );
}

TEST_CASE_PSEUDO_MOVES("Pseudo-legal moves, multiple pieces, black", "") {
    testPseudoLegalMoves(
        // https://lichess.org/editor/7n/7b/6p1/8/8/8/8/8_b_-_-_0_1
        "7n/7b/6p1/8/8/8/8/8 b - - 0 1",
        "",
        {
            "h8f7",
            "h7g8",
            "g6g5"
        }
    );
}
