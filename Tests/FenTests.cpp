#include "catch2/catch.hpp"

#include "TestUtils.hpp"

#include "Fen.hpp"

#include <map>

TEST_CASE("Legal placements are correctly parsed", "[Fen]") {
    // https://lichess.org/editor/k7/2B5/7p/1q3P2/8/3N4/8/5r2_w_-_-_0_1
    auto fen = "k7/2B5/7p/1q3P2/8/3N4/8/5r2 w - - 0 1";

    auto optBoard = Fen::createBoard(fen);
    REQUIRE(optBoard.has_value());

    auto board = optBoard.value();

    auto expectedPlacement = std::map<Square, Piece>{
        {Square::F1, Piece(PieceColor::Black, PieceType::Rook)},
        {Square::D3, Piece(PieceColor::White, PieceType::Knight)},
        {Square::B5, Piece(PieceColor::Black, PieceType::Queen)},
        {Square::F5, Piece(PieceColor::White, PieceType::Pawn)},
        {Square::H6, Piece(PieceColor::Black, PieceType::Pawn)},
        {Square::C7, Piece(PieceColor::White, PieceType::Bishop)},
        {Square::A8, Piece(PieceColor::Black, PieceType::King)}
    };

    for (auto index = 0; index < 64; ++index) {
        auto optSquare = Square::fromIndex(index);
        REQUIRE(optSquare.has_value());

        auto square = optSquare.value();
        INFO("Checking square " << square);

        auto expectedPieceIt = expectedPlacement.find(square);
        auto optActualPiece = board.piece(square);

        if (expectedPieceIt == expectedPlacement.end()) {
            INFO("Expecting empty")
            REQUIRE_FALSE(optActualPiece.has_value());
        } else {
            auto expectedPiece = expectedPieceIt->second;
            INFO("Expecting " << expectedPiece);

            REQUIRE(optActualPiece.has_value());

            auto actualPiece = optActualPiece.value();
            REQUIRE(actualPiece == expectedPiece);
        }
    }
}

TEST_CASE("Legal turns are correctly parsed", "[Fen]") {
    auto [fen, turn] = GENERATE(table<const char*, PieceColor>({
        // https://lichess.org/editor/8/8/8/8/8/8/8/8_w_-_-_0_1
        {"8/8/8/8/8/8/8/8 w - - 0 1", PieceColor::White},
        // https://lichess.org/editor/8/8/8/8/8/8/8/8_b_-_-_0_1
        {"8/8/8/8/8/8/8/8 b - - 0 1", PieceColor::Black}
    }));

    auto optBoard = Fen::createBoard(fen);
    REQUIRE(optBoard.has_value());

    auto board = optBoard.value();
    REQUIRE(board.turn() == turn);
}

TEST_CASE("Castling rights are correctly parsed", "[Fen]") {
    auto [fen, cr] = GENERATE(table<const char*, CastlingRights>({
        // https://lichess.org/editor/rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR_w_KQkq_-_0_1
        {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            CastlingRights::All
        },
        // https://lichess.org/editor/8/8/8/8/8/8/8/8_b_-_-_0_1
        {
            "8/8/8/8/8/8/8/8 b - - 0 1",
            CastlingRights::None
        },
        // https://lichess.org/editor/rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR_w_Qk_-_0_1
        {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Qk - 0 1",
            CastlingRights::WhiteQueenside | CastlingRights::BlackKingside
        }
    }));

    CAPTURE(fen, cr);

    auto optBoard = Fen::createBoard(fen);
    REQUIRE(optBoard.has_value());

    auto board = optBoard.value();
    REQUIRE(board.castlingRights() == cr);
}

TEST_CASE("En passant square is correctly parsed", "[Fen][EnPassant]") {
    auto [fen, ep] = GENERATE(table<const char*, Square::Optional>({
        // https://lichess.org/editor/rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR_w_KQkq_-_0_1
        {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            std::nullopt
        },
        // https://lichess.org/editor/rnbqkbnr/ppppp1pp/8/8/2PPPp2/8/PP3PPP/RNBQKBNR_b_KQkq_e3_0_3
        {
            "rnbqkbnr/ppppp1pp/8/8/2PPPp2/8/PP3PPP/RNBQKBNR b KQkq e3 0 3",
            Square::E3
        }
    }));

    CAPTURE(fen, ep);

    auto optBoard = Fen::createBoard(fen);
    REQUIRE(optBoard.has_value());

    auto board = optBoard.value();
    REQUIRE(board.enPassantSquare() == ep);
}
