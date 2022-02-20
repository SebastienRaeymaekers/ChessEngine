#include "catch2/catch.hpp"

#include "TestUtils.hpp"

#include "Move.hpp"

#include <sstream>

TEST_CASE("Moves store the squares they are constructed with", "[Move][Fundamental]") {
    auto from = Square::E2;
    auto to = Square::G6;
    auto move = Move(from, to);
    REQUIRE(move.from() == from);
    REQUIRE(move.to() == to);
    REQUIRE_FALSE(move.promotion().has_value());
}

TEST_CASE("Moves store the promotion they are constructed with", "[Move][Promotion]") {
    auto from = Square::A7;
    auto to = Square::A8;
    auto promotion = PieceType::Knight;
    auto move = Move(from, to, promotion);
    REQUIRE(move.from() == from);
    REQUIRE(move.to() == to);
    REQUIRE(move.promotion().has_value());
    REQUIRE(move.promotion().value() == promotion);
}

TEST_CASE("Moves support equality checks", "[Move][Fundamental]") {
    auto move1 = Move(Square::D4, Square::H7);
    auto move2 = Move(Square::D4, Square::G3);
    auto move3 = Move(Square::A3, Square::H7);
    auto move1Copy = move1;
    REQUIRE(move1 == move1Copy);
    REQUIRE_FALSE(move1 == move2);
    REQUIRE_FALSE(move1 == move3);
}

TEST_CASE("Moves with promotions support equality checks", "[Move][Promotion]") {
    auto move1 = Move(Square::B2, Square::B1, PieceType::Queen);
    auto move2 = Move(Square::B2, Square::B1);
    auto move3 = Move(Square::C2, Square::C1, PieceType::Queen);
    auto move1Copy = move1;
    REQUIRE(move1 == move1Copy);
    REQUIRE_FALSE(move1 == move2);
    REQUIRE_FALSE(move1 == move3);
}

TEST_CASE("Moves stream their UCI correctly", "[Move][Fundamental]") {
    auto [from, to, uci] = GENERATE(table<Square, Square, const char*>({
        {Square::A1, Square::D7, "a1d7"},
        {Square::C8, Square::F2, "c8f2"},
        {Square::H6, Square::B1, "h6b1"}
    }));

    auto move = Move(from, to);
    auto stream = std::stringstream();
    stream << move;
    REQUIRE(stream.str() == uci);
}

TEST_CASE("Moves with promotions stream their UCI correctly", "[Move][Promotion]") {
    auto [from, to, promotion, uci] = GENERATE(table<Square, Square, PieceType, const char*>({
        {Square::H7, Square::H8, PieceType::Knight, "h7h8n"},
        {Square::C2, Square::C1, PieceType::Rook,   "c2c1r"},
        {Square::E7, Square::E8, PieceType::Queen,  "e7e8q"}
    }));

    auto move = Move(from, to, promotion);
    auto stream = std::stringstream();
    stream << move;
    REQUIRE(stream.str() == uci);
}

TEST_CASE("Moves can be created from valid UCI notation", "[Move][Fundamental]") {
    auto [uci, from, to] = GENERATE(table<const char*, Square, Square>({
        {"a1d7", Square::A1, Square::D7},
        {"c8f2", Square::C8, Square::F2},
        {"h6b1", Square::H6, Square::B1}
    }));

    auto move = Move::fromUci(uci);
    auto expectedMove = Move(from, to);

    CAPTURE(uci, expectedMove, move);
    REQUIRE(move.has_value());
    REQUIRE(move.value() == expectedMove);
}

TEST_CASE("Moves can be created from valid UCI notation with promotion", "[Move][Promotion]") {
    auto [uci, from, to, promotion] = GENERATE(table<const char*, Square, Square, PieceType>({
        {"a7a8n", Square::A7, Square::A8, PieceType::Knight},
        {"c2c1b", Square::C2, Square::C1, PieceType::Bishop},
        {"h7h8r", Square::H7, Square::H8, PieceType::Rook}
    }));

    auto move = Move::fromUci(uci);
    auto expectedMove = Move(from, to, promotion);

    CAPTURE(uci, expectedMove, move);
    REQUIRE(move.has_value());
    REQUIRE(move.value() == expectedMove);
}

TEST_CASE("Moves are not created from invalid UCI notation", "[Move][Fundamental]") {
    auto uci = GENERATE("a1d7x", "a1d", "a1d7123", "a1", "", "a9d1", "a1c0");
    auto move = Move::fromUci(uci);

    CAPTURE(uci, move);
    REQUIRE_FALSE(move.has_value());
}
