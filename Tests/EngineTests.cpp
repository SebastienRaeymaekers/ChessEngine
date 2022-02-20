#include "catch2/catch.hpp"

#include "TestUtils.hpp"

#include "EngineFactory.hpp"
#include "Engine.hpp"
#include "Fen.hpp"
#include "Board.hpp"
#include <iostream>

static std::unique_ptr<Engine> createEngine() {
    return EngineFactory::createEngine();
}

static void testGameEnd(const char* fen, bool isMate) {
    auto engine = createEngine();
    REQUIRE(engine != nullptr);

    auto board = Fen::createBoard(fen);
    REQUIRE(board.has_value());

    auto pv = engine->pv(board.value());

    REQUIRE(pv.isMate() == isMate);
    REQUIRE(pv.score() == 0);
    REQUIRE(pv.length() == 0);
}

TEST_CASE("Engine detects checkmate", "[Engine][Checkmate]") {
    auto fen = GENERATE(
        // https://lichess.org/editor/4R2k/6pp/8/8/8/8/8/K7_b_-_-_0_1
        "4R2k/6pp/8/8/8/8/8/K7 b - - 0 1",
        // https://lichess.org/editor/7k/8/8/8/1bb5/8/r7/3BK3_w_-_-_0_1
        "7k/8/8/8/1bb5/8/r7/3BK3 w - - 0 1"
    );

    testGameEnd(fen, true);
}

TEST_CASE("Engine detects stalemate", "[Engine][Stalemate]") {
    auto fen = GENERATE(
        // https://lichess.org/editor/k7/p7/P7/8/8/8/8/KR6_b_-_-_0_1
        "k7/p7/P7/8/8/8/8/KR6 b - - 0 1",
        // https://lichess.org/editor/k7/8/8/6n1/r7/4K3/2q5/8_w_-_-_0_1
        "k7/8/8/6n1/r7/4K3/2q5/8 w - - 0 1"
    );

    testGameEnd(fen, false);
}
