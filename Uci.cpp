#include "Uci.hpp"

#include "Fen.hpp"

#include <utility>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

Uci::Uci(std::unique_ptr<Engine> engine,
         std::istream& cmdIn,
         std::ostream& cmdOut,
         std::ostream& log
) : engine_(std::move(engine)), cmdIn_(cmdIn), cmdOut_(cmdOut), log_(log) {
}

void Uci::run() {
    log_ << "UCI engine started" << std::endl;

    while (!cmdIn_.eof()) {
        std::string line;
        std::getline(cmdIn_, line);
        runCommand(line);
    }
}

void Uci::runCommand(const std::string& line) {
    log_ << "> " << line << std::endl;

    auto stream = std::stringstream(line);
    auto command = std::string();
    stream >> command;

    if (command == "uci") {
        uciCommand(stream);
    } else if (command == "isready") {
        isreadyCommand(stream);
    } else if (command == "ucinewgame") {
        ucinewgameCommand(stream);
    } else if (command == "position") {
        positionCommand(stream);
    } else if (command == "go") {
        goCommand(stream);
    } else if (command == "quit") {
        quitCommand(stream);
    }
}

void Uci::uciCommand(std::istream&) {
    std::stringstream nameCommand;
    nameCommand << "id name " << engine_->name() << " " << engine_->version();
    sendCommand(nameCommand.str());

    std::stringstream authorCommand;
    authorCommand << "id author " << engine_->author();
    sendCommand(authorCommand.str());

    sendCommand("uciok");
}

void Uci::isreadyCommand(std::istream&) {
    sendCommand("readyok");
}

void Uci::ucinewgameCommand(std::istream&) {
    engine_->newGame();
}

void Uci::positionCommand(std::istream& stream) {
    auto type = std::string();
    stream >> type;

    auto newBoard = Board::Optional();

    if (type == "startpos") {
        newBoard = Fen::createBoard(Fen::StartingPos);
    } else if (type == "fen") {
        newBoard = Fen::createBoard(stream);
    } else {
        error("Illegal position type " + type);
        return;
    }

    if (!newBoard.has_value()) {
        error("Illegal FEN");
        return;
    }

    board_ = newBoard.value();

    auto moves = std::string();
    stream >> moves;

    if (moves == "moves") {
        while (!stream.eof()) {
            auto uciMove = std::string();
            stream >> uciMove;

            if (uciMove.empty()) {
                continue;
            }

            auto optMove = Move::fromUci(uciMove);

            if (!optMove.has_value()) {
                error("Illegal move " + uciMove);
                return;
            }

            board_.makeMove(optMove.value());
        }
    }

    log_ << board_ << std::endl;
}

template<typename T>
static std::optional<T> readValue(std::istream& stream) {
    if (T value; stream >> value) {
        return value;
    } else {
        return std::nullopt;
    }
}

TimeInfo::Optional Uci::readTimeInfo(std::istream& stream) {
    std::optional<unsigned> wtime, winc, btime, binc, movestogo;

    for (std::string command; stream >> command;) {
        auto value = readValue<unsigned>(stream);

        if (command == "wtime") {
            wtime = value;
        } else if (command == "winc") {
            winc = value;
        } else if (command == "btime") {
            btime = value;
        } else if (command == "binc") {
            binc = value;
        } else if (command == "movestogo") {
            movestogo = value;
        } else if (command == "infinite") {
            error("go infinite not supported");
            return std::nullopt;
        }
    }

    if (wtime.has_value() && btime.has_value()) {
        PlayerTimeInfo whiteTime, blackTime;
        whiteTime.timeLeft = std::chrono::milliseconds(wtime.value());
        whiteTime.increment = std::chrono::milliseconds(winc.value_or(0));
        blackTime.timeLeft = std::chrono::milliseconds(btime.value());
        blackTime.increment = std::chrono::milliseconds(binc.value_or(0));

        TimeInfo timeInfo;
        timeInfo.white = whiteTime;
        timeInfo.black = blackTime;
        timeInfo.movesToGo = movestogo;
        return timeInfo;
    } else {
        return std::nullopt;
    }
}

void Uci::goCommand(std::istream& stream) {
    auto timeInfo = readTimeInfo(stream);
    auto pv = engine_->pv(board_, timeInfo);

    if (pv.length() == 0) {
        error("Engine returned no PV");
        return;
    }

    log_ << "PV: " << pv << std::endl;
    sendPvInfo(pv);

    auto bestMove = *pv.begin();
    board_.makeMove(bestMove);
    log_ << board_ << std::endl;

    auto bestMoveCmd = std::stringstream();
    bestMoveCmd << "bestmove " << bestMove;
    sendCommand(bestMoveCmd.str());
}

void Uci::quitCommand(std::istream&) {
    std::exit(EXIT_SUCCESS);
}

void Uci::sendPvInfo(const PrincipalVariation& pv) {
    auto stream = std::stringstream();
    stream << "info score ";

    auto score = pv.score();

    if (pv.isMate()) {
        auto numMoves =
            score < 0 ? std::floor(score / 2.0) : std::ceil(score / 2.0);
        stream << "mate " << numMoves;
    } else {
        stream << "cp " << score;
    }

    stream << " pv";

    for (auto move : pv) {
        stream << ' ' << move;
    }

    sendCommand(stream.str());
}

void Uci::sendCommand(const std::string& command) {
    log_ << "< " << command << std::endl;
    cmdOut_ << command << std::endl;
}

void Uci::error(const std::string& msg) {
    log_ << "UCI error: " << msg << std::endl;
    std::exit(EXIT_FAILURE);
}
