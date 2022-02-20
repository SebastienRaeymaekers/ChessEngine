#include "catch2/catch.hpp"

#include "TestUtils.hpp"

#include "Square.hpp"

#include <sstream>

TEST_CASE("Squares can be created from valid coordinates", "[Square][Fundamental]") {
    auto [file, rank] = GENERATE(table<Square::Coordinate, Square::Coordinate>({
        {4, 2}, {0, 0}, {7, 7}, {1, 7}
    }));

    auto optSquare = Square::fromCoordinates(file, rank);
    REQUIRE(optSquare.has_value());

    auto square = optSquare.value();
    REQUIRE(square.file() == file);
    REQUIRE(square.rank() == rank);
    REQUIRE(square.index() == rank * 8 + file);
}

TEST_CASE("Squares are not created from invalid coordinates", "[Square][Fundamental]") {
    auto [file, rank] = GENERATE(table<Square::Coordinate, Square::Coordinate>({
        {4, 8}, {8, 3}, {12, 45}
    }));

    auto optSquare = Square::fromCoordinates(file, rank);
    REQUIRE_FALSE(optSquare.has_value());
}

TEST_CASE("Squares can be created from valid indices", "[Square][Fundamental]") {
    auto index = GENERATE(as<Square::Index>{}, 0, 63, 12, 41);

    auto optSquare = Square::fromIndex(index);
    REQUIRE(optSquare.has_value());

    auto square = optSquare.value();
    REQUIRE(square.file() == index % 8);
    REQUIRE(square.rank() == index / 8);
    REQUIRE(square.index() == index);
}

TEST_CASE("Squares are not created from invalid indices", "[Square][Fundamental]") {
    auto index = GENERATE(as<Square::Index>{}, 64, 1024);

    auto optSquare = Square::fromIndex(index);
    REQUIRE_FALSE(optSquare.has_value());
}

TEST_CASE("Squares can be created from valid names", "[Square][Fundamental]") {
    auto [square, name] = GENERATE(table<Square, const char*>({
        {Square::A1, "a1"}, {Square::H8, "h8"}, {Square::D5, "d5"}
    }));

    auto optCreatedSquare = Square::fromName(name);
    CAPTURE(square, name, optCreatedSquare);
    REQUIRE(optCreatedSquare.has_value());

    auto createdSquare = optCreatedSquare.value();
    REQUIRE(createdSquare == square);
}

TEST_CASE("Squares are not created from invalid names", "[Square][Fundamental]") {
    auto name = GENERATE("", "a", "a12", "1a", "xyz");

    auto optSquare = Square::fromName(name);
    CAPTURE(name, optSquare);
    REQUIRE_FALSE(optSquare.has_value());
}

TEST_CASE("Squares stream their name correctly", "[Square][Fundamental]") {
    auto [square, name] = GENERATE(table<Square, const char*>({
        {Square::A1, "a1"}, {Square::H8, "h8"}, {Square::E4, "e4"}
    }));

    auto stream = std::stringstream();
    stream << square;
    REQUIRE(stream.str() == name);
}
