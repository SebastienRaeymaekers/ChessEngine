[TOC]

# Introduction

In this project, the goal is to develop a  [*chess engine*][chess engine] in C++.
A chess engine is a computer program that analyzes chess positions and generates a move that it considers best.
It typically consists of three major parts:
- *Board representation*: the data structure that stores all relevant information about a chess position;
- *Move generation*: given a position, generate all valid moves from that position;
- *Search*: use move generation to create a game tree and search it for good moves using an *evaluation heuristic*.

You will be implementing all three parts in this project.

## Goal

The main goal of this project is to create a *correct* implementation.
Many [tests](#testing) are provided to help you validate the correctness of your implementation.
*However*, performance is an important aspect of chess engines.
The search space is simply too large for a brute-force search, even at very small depths.
Therefore, you are expected to put some effort in optimizing your engine.

Some important points about our evaluation:
- Passing all [unit tests](#unit-tests) is a good indication of a correct board representation and move generation implementation.
  However, not all corner cases are tested and we *will* run more tests;
- Passing all [puzzle tests](#puzzles) is a good indication of a correct and adequately performing search/evaluation implementation.
  However, we *will* use more puzzles of increasing difficulty for our evaluation;
- All given puzzles are run with a time limit of 3 minutes.
  Puzzles that take longer to solve will fail.

It is a good idea to add more tests yourself!

## Submitting

This is an **individual assignment**!
A private Git repository will be created for you on the KU Leuven GitLab servers which you can use for development and will be used for submitting your project.
Do not share this repository with anyone.


**Submitting**:
- Deadline: **Friday December 24, 2021 at 23:59**;
- How: push your solution to the `main` branch of your private repository.

> :bulb: At the time of the deadline, your repository will be archived.
> This means it's still available but read-only.

**Important guidelines**:
- Whatever is on the `main` branch of your private repository at the time of the deadline is considered your solution, nothing else.
  Make sure *all* your code is on the *correct branch* in the *correct repository*;
- Your **code must work on our test infrastructure**.
  If it does not, we **cannot grade it**.
  When you push to your private repository, the tests are [automatically run](#automatic-test-runs) on our infrastructure.
  Do this regularly;
- Do not change how tests are run, do not remove any tests, and do not change the compiler options related to warnings in [CMakeLists.txt](CMakeLists.txt).
  If you do, the automatic test runs will not be representative anymore since we will restore everything when running our evaluation tests;
- Do **not share your code** with anyone and do **not include any code not written by you** in your repository (except, of course, the initial code we provided).

## Allowed language features

You are allowed to use all features of [C++20][c++20] including everything in the standard library as long as the compiler on our test infrastructure supports it.
However, we recommend sticking with [C++17][c++17] as this is what was used during the lecture and exercise sessions.
See [this][c++ features] for an overview of all (library) features and in which C++ version they are available.

> :bulb: The compiler used on our test infrastructure is GCC 11.2.0 which [supports][gcc c++ status] (nearly) all C++20 features.

> :bulb: To use C++20, you will have to update the `CMAKE_CXX_STANDARD` variable in [CMakeLists.txt](CMakeLists.txt).

You are **not allowed to use *any* external libraries**.
This includes header-only libraries.

> :warning: Including *any* source files not fully written by you in your repository will be considered plagiarism!

## Questions and issues

All code contains bugs, so I expect that some will be found in the code provided for this assignment.
If you happen to find one, please [report][questions] it and I will try to fix it as soon as possible.
I will then publish the fix on the [assignment repository][assignment repo] from which it can be merged into your private repository.

> :bulb: To make sure merging is as painless as possible, try not to commit any unnecessary changes to the code, especially in the provided implementation files like [Fen.cpp](Fen.cpp) and [Uci.cpp](Uci.cpp).
> Always check what you are about to commit using, for example, `git diff --staged`.

The same issue tracker can be used to **ask questions** about the project.
Just open [open an issue][questions] and I will try to answer as soon as possible.

# Setup

The basic setup for the project is the same as for the [exercise sessions][exercises setup].

From now on, we use the following variables in commands:
- `$REPO_URL`: the Git URL of your private repository;
- `$REPO_DIR`: the directory where you will clone your repository;
- `$BUILD_DIR`: the directory where you will build the project.

[Catch2][catch2] is used for unit testing and its source code is included as a [submodule][git submodules].
Therefore, we have to clone the project repository recursively:

```
$ git clone --recurse-submodules $REPO_URL $REPO_DIR
```

If you did a normal clone, don't worry, this can be fixed:
```
$ cd $REPO_DIR
$ git submodule update --init
```

Now the project can be built in the usual way:

```
$ mkdir $BUILD_DIR
$ cd $BUILD_DIR
$ cmake $REPO_DIR
$ cmake --build .
```

By default, CMake will perform a *debug build*.
This means, among others, that optimizations are disabled.
Since performance is important for a chess engine, you might want perform a *release build*.
This can be done by replacing the third command above by the following:
```
$ cmake -DCMAKE_BUILD_TYPE=Release $REPO_DIR
```

> :bulb: You can have multiple build directories, for example, one with a debug build and one with a release build.
> This is useful since debug builds are preferred if you ever need to use a debugger.
> Debugging release builds is difficult since they do not contain debugging symbols (so you cannot put breakpoints on functions, for example) and are highly optimized.

> :bulb: The above procedure for creating a debug build does not work on Visual Studio since it creates multiple builds in the same directory.
> Therefore, a release build can be created as follows (replaces the fourth command above):
> ```
> $ cmake --build . --config Release
> ```

If all went well, there should now be an executable called `cplchess` for the engine, and one called `Tests/tests` for the unit tests.
Try to execute the latter to verify your build worked.
The tests will obviously fail but it should not crash.

If you want to add extra `.cpp` files to your project (which you will have to), you should add them to the `cplchess_lib` library in [CMakeLists.txt](CMakeLists.txt).
This library is linked into both the engine and the unit tests executables.
If you want to add extra unit test files, add them to the `tests` executable in [Tests/CMakeLists.txt](Tests/CMakeLists.txt)

> :bulb: As explained [earlier](#submitting), you are not allowed to change the compiler options related to warnings in [CMakeLists.txt](CMakeLists.txt).
> We have enabled many [compiler warnings][gcc warnings] in the build and made sure that warnings are treated as errors.
> This forces you to write warning-free code to help you discover many mistakes early.

# Problem description

This section describes the parts you have to implement in your chess engine.
If you are not familiar with chess, it might be worthwhile to learn its [rules][chess rules] first.

Although it is entirely up to you in which order to implement the parts, following the order in this section is probably a good idea.
The unit tests allow you to [select certain tests](#test-tags) to focus on specific parts.
This also allows you to postpone the implementation of more difficult move types (i.e., [castling][castling], [promotion][promotion], and [en passant][en passant]).
It could be interesting, for example, to test the full search algorithm before adding those moves.

> :bulb: Most code you have to implement can be added to existing source files.
> We have provided skeleton code to ensure that all tests compile and that you adhere to the needed API.
> All method bodies are empty but to prevent compiler warnings/errors we
> - return some arbitrary value in non-`void` methods;
> - pretend to use arguments by casting them to `void` (e.g., `(void)arg`).

## Fundamental data structures

> :bulb: Relevant unit test [tag](#test-tags): `[Fundamental]`

We start with describing the fundamental data structures used to represent the board state and moves.
All these data structures need to follow the given API but how they are implemented is up to you.

### Piece

Pieces are the main tools at the disposal of the chess players.
Although the different kind of pieces vary in behavior, we represent them simply by their basic properties: *color* (black or white) and *type* (pawn, knight, etc).
Their behavior (i.e., how they move) is [handled elsewhere](#move-generation).

Piece representation is implemented in [Piece.hpp](Piece.hpp) and [Piece.cpp](Piece.cpp).
Three types are defined here:
- `PieceColor` (`enum`);
- `PieceType` (`enum`);
- `Piece` (`class` that stores a color and a type).

Besides their representation, the `Piece` class also handles conversion to and from standard piece symbols: `Piece::fromSymbol()` takes a `char` and converts it to a `Piece` (if possible) and streaming to `std::ostream` can be used for the opposite.
The following symbols are used for chess pieces: `P` (pawn), `N` (knight), `B` (bishop), `R` (rook), `Q` (queen), and `K` (king). Uppercase letters are used for white, lowercase for black.

> :bulb: Throughout this project, [`std::optional`][std optional] is used to represent optional values.
> For example, creation methods like `Piece::fromSymbol()` return an optional if they may fail.
> Many classes that are often used as optional values define a type alias called `Optional` for brevity.

### Square

Chessboards consist of a grid of 8x8 *squares*.
In [*algebraic notation*][san], each square is identified by a coordinate pair from white's point of view.
Columns (called *files*) are labeled *a* through *h* while rows (called *ranks*) are labeled *1* through *8*.
So, the lower-left square is called *a1* while the upper-right square is called *h8*.

The `Square` class (implemented in [Square.hpp](Square.hpp) and [Square.cpp](Square.cpp)) is used to identify squares.
It offers two ways of identifying squares:
- Coordinates: create using `fromCoordinates()` and get using `file()` and `rank()`.
  Note that both coordinates are numbers in the range `[0, 8)`.
- Index: create using `fromIndex()` and get using `index()`.
  In this representation, all squares have a unique index in the range `[0, 64)` where the index of *a1* is *0*, *b1* is *1*, and that of *h8* is *63*.

Squares also offer conversion from (`fromName`) and to (streaming to `std::ostream`) their name.

Two comparison operator are needed:
- `operator==` for comparing squares in the tests and elsewhere;
- `operator<` for using squares as keys in associative (sorted) containers like [`std::map`][std_map].
  You can use any total ordering.

Constants are provided to conveniently references all squares (e.g., `Square::E4`).
These are implemented using a private constructor that takes an index as argument.
You don't have to keep this constructor, but if you don't, you will have to update the initialization of the constants.

### Move

Moves are how a game of chess progresses from turn to turn.
Although the concept of a move potentially contains a lot of information (e.g., which piece is moved, if the move is valid, etc.), the `Move` class (implemented in [Move.hpp](Move.hpp) and [Move.cpp](Move.cpp)) only uses the bare minimum: the *from* and *to* squares and optionally the [promoted][promotion]-to piece type.
Its constructor takes all three items (although the promotion type is optional and by default a move is not a promotion).

As a textual representation, the `Move` class uses the [UCI][uci] notation (also sometimes called the [*long algebraic notation*][long algebraic notation]).
In this notation, moves are represented by the concatenation of the names of the from and to squares, optionally followed by the lower case symbol of the promotion piece type.
[*Castling*][castling] is considered a king move so is represented by the from and to squares of the king (e.g., white kingside is castling is *e1g1*).
Moves can be created from this representation using `fromUci` and the conversion to UCI is done by streaming to `std::ostream`.

Two comparison operator are needed:
- `operator==` for comparing moves in the tests and elsewhere;
- `operator<` for using moves as keys in associative (sorted) containers like [`std::map`][std_map].
  You can use any total ordering.

Note that this simple representation allows the creation of illegal moves.
Indeed, since the `Move` class has no relation with a `Board`, it's impossible to validate the legality of moves in isolation.
We will see how to handle illegal moves [later](#move-making).

> :bulb: There are some things that *can*  be checked about a move's validity.
> For example, whether a promotion is certainly invalid (e.g., *a7a8k* or *e6e8q* are *never* valid).
> You are free to check this in `fromUci()` but this is not necessary, especially since the constructor has no way of reporting errors so creating such invalid moves is always possible.

### Board

Arguably the most important decision for chess engines is how the [board state is represented][board representation wp] as it will have a large impact on how [*move generation*](#move-generation) will be implemented and thus how efficient this will be.
The most popular among high-rated chess engines is probably a [bitboard][bitboard] representation (e.g., [Stockfish][stockfish] uses this).
Although very efficient for move generation, this representation can be difficult to grasp and work with.
Square-centric, array-based representations (e.g., [8x8 boards][8x8 board]) might be easier to work with at the cost of less efficient move generation.

You are entirely free to choose the way the `Board` class represents its state as long as you can implement its interface:
- Getting and setting pieces: `piece(Square)`, `setPiece(Square, Piece)`;
- Getting and setting the turn: `turn()`, `setTurn(PieceColor)`;
- Getting and setting the [castling rights](#castling-rights): `castlingRights()`, `setCastlingRights(CastlingRights)`;
- Getting and setting the [en passant square](#en-passant-square): `enPassantSquare()`, `setEnPassantSquare(Square)`;
- Default construction (`Board()`): create an _empty_ board.

You should also implement streaming a `Board` to `std::ostream` for debugging purposes.
You are free to choose how a board is printed.
I like the following format:

```
r n b q k b n r
p p p p p p p p
. . . . . . . .
. . . . . . . .
. . . . . . . .
. . . . . . . .
P P P P P P P P
R N B Q K B N R
```

#### Castling rights

[*Castling*][castling] is probably the most complex move type in chess and requires extra state to be stored in `Board` (i.e., knowing which castling moves are valid does not depend only on the state of the pieces).
For example, once a rook has moved, its king permanently loses the right to castle on that side of the board, even when the rook returns to its original square.

To represent *castling rights*, the `CastlingRights` enumeration is provided (see [CastlingRights.hpp](CastlingRights.hpp) and [CastlingRights.cpp](CastlingRights.cpp)).
Overloads are provided for many bitwise operations so that this type can be used as [bit flags][bit flags].
For example:

```c++
CastlingRights rights = ...;

if ((rights & CastlingRights::BlackQueenside) != CastlingRights::None) {
    // Queenside castling is available for black
}

// Add kingside castling rights for white
rights |= CastlingRights::WhiteKingside;

// Remove all castling rights for black
rights &= ~CastlingRights::Black;
```

#### En passant square

[*En passant*][en passant], probably the least-known move type in chess, also requires extra state to be stored since it is only allowed for one move after a pawn made a two-square move.
Therefore, to implement pawn captures correctly, `Board` should store the square to which an en passant capture can be made.

## Move generation

> :bulb: Relevant unit test [tag](#test-tags): `[MoveGen]`

[*Move generation*][move generation] is the process of generating valid moves from a board state.
These moves will be used for [searching](#searching).

There are generally two ways of generating moves: legal or pseudo-legal.

### Legal move generation

Only moves that are [legal][legal move] according to the [rules of chess][chess rules] are generated.
This is convenient for the search algorithm but might not be that easy to implement efficiently.
The main issue is verifying that the king is not left in [check][chess rules check] after a move (which is illegal).
The naive way of doing this might be to generate the opponent's moves and verifying that none of them captures the king.
While this might work, it would result in doubling the amount of generated moves.

### Pseudo-legal move generation

A [*pseudo-legal move*][pseudo legal move] is one that adheres to all the rules of chess *except* that it might leave the king in check.
From the description above it should be clear that this is easier to generate but might complicate the search algorithm.
Indeed, we now have to make sure that we somehow reject illegal moves while searching.
One way to tackle this could be to search until a move that captures the king and then reject the previous move.

> :bulb: Due to the complex rules of [castling][castling], a pseudo-legal castling move is usually also legal.

### Interface

Move generation should be implemented by the `Board` class through the following method:

```c++
using MoveVec = std::vector<Move>;

void Board::pseudoLegalMoves(MoveVec& moves) const;
```

This method should add all (pseudo) legal moves for the current player to the `moves` vector.

For [testing](#unit-tests) purposes, the following method should also be implemented to generate all moves from a specific square:

```c++
void Board::pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const;
```

> :bulb: You are free to use an entirely different interface to communicate between the search algorithm and `Board` as long as you can still implement this one for testing.

> :bulb: Even though "pseudo-legal" is part of the method names, you are free to generate legal moves here.
> The tests will only verify the correct generation of pseudo-legal moves, though.

## Move making

> :bulb: Relevant unit test [tag](#test-tags): `[MoveMaking]`

[*Move making*][move making] is the process of updating the board state to reflect a move.
Obviously, this should include moving the moved piece from the source square to the destination square (possibly replacing a captured piece) but it may also include updating [castling rights](#castling-rights) and the [en passant square](#en-passant-square).

Move making is used by the search algorithm to generate child nodes after move generation.

It should be implemented in the `Board` class by the following method:

```c++
void Board::makeMove(const Move& move);
```

> :bulb: Since this method is only used by your search algorithm, it is probably not necessary to verify that the given move is legal in this method.
> You just have to make sure that *if* a legal move is given, it is correctly executed.

## Searching

> :bulb: Relevant tests: [puzzles](#puzzles).

The basic question a chess engine tries to answer is what the best move to play is at a particular position.
To answer this, a [search tree][game tree] is typically constructed with the initial position in the root node and the children at every level being positions resulting from valid moves from their parents.

Given this tree, a search algorithm tries to find a path that *guarantees* the player will end-up in *the most favorable* position.
There are two words to discuss a bit here:
- *Guaranteed* most favorable position: what we mean here is that we must assume the opponent always makes the best possible move for them.
  That is, we assume the *worst case scenario* and try to minimize our loss (or maximize our win);
- *Favorable*: we try to win the chess game but most of the time it will not feasible to search the whole tree to find a [checkmate][checkmate].
  Therefore, we have to limit the search depth and use a [heuristic][eval] to evaluate positions that are not the [end of the game][chess rules game end].

[*Minimax*][minimax] is a well-known search algorithm that guarantees finding the move that ends-up in the most favorable position.
You are free to use any correct algorithm but minimax is definitely a good choice.

> :bulb: Here are some tips for using minimax:
> - Since chess is a [zero-sum game][zero-sum game], the slightly simpler [negamax][negamax] variant can be used;
> - It is highly recommended to implement [*alpha-beta pruning*][alpha-beta pruning] to improve search performance.
>   Without it, it is probably impossible to solve most puzzles within the time limit;
> - When using alpha-beta pruning, [*move ordering*][move ordering] becomes important: if potentially good moves are searched first, it may lead to earlier pruning reducing the size the search tree;
> - If you want to take time into account (see below), you might want to use [*iterative deepening*][iterative deepening] to make sure you have reasonable move available quickly while continuing the search at higher depths to find better moves.

[Evaluating][eval] chess positions is very complex.
The most naive way is to simply calculate the [value][piece value] of all remaining pieces.
However, this is clearly not optimal so more advanced strategies will take other aspects into account (e.g., [center control][center control], [king safety][king safety], etc).

> :bulb: It is up to you to decide how far you want to go with improving the heuristic.
> However, it should at least be good enough to solve the provided [puzzles](#puzzles).

Remember that if you use [pseudo-legal move generation](#pseudo-legal-move-generation), the search algorithm needs to somehow reject illegal moves.
And unless you evaluation function can detect [checkmate][checkmate] and [stalemate][stalemate], it also needs to be able to handle [end of the game][chess rules game end] conditions.

### Principal variation

The result of a search is a [*principal variation*][pv] (PV) which is the sequence of moves the engine considers best.
A PV is represented by the `PrincipalVariation` class (in [PrincipalVariation.hpp](PrincipalVariation.hpp) and [PrincipalVariation.cpp](PrincipalVariation.cpp)).
The main interface that has to be implemented is the following:

```c++
using MoveIter = /* your iterator type here */;

std::size_t PrincipalVariation::length() const;
MoveIter PrincipalVariation::begin() const;
MoveIter PrincipalVariation::end() const;
```

Where `length()` returns the number of [plies][ply] in the PV and `begin()` and `end()` allows for iterating over the `Move`s.

> :bulb: You are free to choose any container to store the `Move`s.
> Adjust `MoveIter` accordingly.

The `PrincipalVariation` class also stores information about the evaluation [score][eval score]:

```c++
bool PrincipalVariation::isMate() const;
int PrincipalVariation::score() const;
```

`isMate()` should return `true` if the PV ends in checkmate.
In this case, `score()` should return the number of plies that leads to the checkmate.
Otherwise, `score()` returns the evaluation of the position the PV leads to.
In both cases, the score is from the point of view of the engine: positive values are advantageous for the engine, negative ones for its opponent.

> :bulb: Your are free to choose the unit of the evaluation score as long as larger scores mean better evaluations.
> Typically, [*centipawns*][centipawns] are used where 100 centipawns corresponds to the value of one pawn.

There is also an overload declared to stream `PrincipalVariation` to a `std::ostream`.
You can choose how PVs are printed.

### Engine

The main interface to the search algorithm is provided by the abstract class `Engine` (see [Engine.hpp](Engine.hpp)).
This class functions as an interface so you have to add you own derived class to implement this interface.
You should then adapt `EngineFactory::createEngine()` (in [EngineFactory.cpp](EngineFactory.cpp)) to return an instance of your class.

The most important method in the `Engine` interface is the following:

```c++
virtual PrincipalVariation Engine::pv(
    const Board& board,
    const TimeInfo::Optional& timeInfo = std::nullopt
) = 0;
```

This should calculate and return the PV starting from the position represented by `board`.

> :bulb: The second argument, `timeInfo`, provides timing information (see [TimeInfo.hpp](TimeInfo.hpp)) if it is available.
> Most chess games are played with a [time control][time control] and for those games, `timeInfo` contains the time left on the clock of both players as well as their increment.
> It is *completely optional* to use this information but will be especially useful if you choose to use [iterative deepening][iterative deepening].

The following method is called whenever a new game starts (not guaranteed to be called for the first game played by an `Engine` instance):

```c++
virtual void Engine::newGame() = 0;
```

> :bulb: You probably don't need this method but it could be useful if you store state inside your engine.

Lastly, the following methods are used to identify your engine over the [UCI interface](#uci):

```c++
virtual std::string Engine::name() const = 0;
virtual std::string Engine::version() const = 0;
virtual std::string Engine::author() const = 0;
```

# Testing

A number of tests are provided in the [Tests](Tests/) directory that you can run locally and are also run automatically when pushing commits to GitLab.
You are free (even encouraged) to add more tests but **do not modify any of the existing test files**.
They will be overwritten when we run our automatic tests after the deadline.
Also remember that the tests do not try to cover all edge cases and we will run more tests when grading the projects.

Unit tests are provided to verify the correctness of the [board representation](#fundamental-data-structures) and [pseudo-legal move generation](#move-generation).
To test the search algorithm, chess puzzles are used.

> :bulb: It is possible to pass all the tests with a relatively shallow search depth (about 5 [plies](ply)) and a relatively simple evaluation function.
> The most naive search, however, will probably not suffice to finish most puzzles within the time limit.

## Automatic test runs

Whenever you push commits to your private repository, a [CI/CD pipeline][gitlab pipelines] will start automatically that builds and tests your code.
The pipelines can be viewed by going to "CI/CD -> Pipelines" in the web interface of your repository.

When clicking on a pipeline, you can see it consist of two stages and three jobs:
- Build stage: runs the "build" job that compiles your code and produces the engine and unit tests executables for the next stage;
- Test stage: runs the "unit_tests" and "puzzle_tests" jobs that perform all automatic tests.

> :bulb: If the build stage fails, the test stage will not run.
> Both jobs in the test stage are independent and run in parallel, though.

You can click on a specific job to view its output which can be useful when debugging failures (e.g., to see compiler errors).

The "Tests" tab contains detailed information about the result of the test runs.
If you want to know why a test failed, click on the job name in the "Tests" tab first (e.g., "unit_tests") and then on "View details" next to the failed test.

> :bulb: For those who are interested, the pipeline is defined in [.gitlab-ci.yml](.gitlab-ci.yml).
> **Do not modify this file, though!**

## Unit tests

Unit tests are implemented using the [Catch2][catch2] framework.
After building the project, a test executable is available at `$BUILD_DIR/Tests/tests`.
Running this executable without any arguments runs all tests and looks something like this when everything passes:

```
$ $BUILD_DIR/Tests/tests
===============================================================================
All tests passed (1698 assertions in 95 test cases)
```

### Test tags

Since initially (almost) all tests will fail, test cases are tagged to be able to only run a subset of tests.
The following tags are provided:
- `[Fundamental]`: tests for functionality that is fundamental.
  For example, for classes or methods used by the tests themselves.
  This includes the `Piece`, `Square`, and `Move` classes as well as the state of the `Board` class.
  **This functionality should be the first thing to implement!**
- `[ClassName`]: each class with unit tests has a corresponding tag (e.g., `[Board]`) that runs all tests of that class;
- `[MoveGen]`: all tests for pseudo-legal move generation;
- `[MoveMaking]`: all tests for move making;
- `[Castling]`, `[Promotion]`, `[EnPassant]`: all tests for these "special" moves are tagged accordingly.
  This allows you to implement and test "normal" moves first;
- `[PieceType]`: tests for a specific piece type (e.g., `[Queen]`) are tagged accordingly.
  This is mostly related to `[MoveGen]` tests.

Tags can be selected by providing them as an argument to the test executable.
If one is preceded by a `~`, it is deselected.
For example, to test the move generation for pawns except en passant:

```
$ $BUILD_DIR/Tests/tests '[MoveGen][Pawn]~[EnPassant]'
Filters: [MoveGen][Pawn]~[EnPassant]
===============================================================================
All tests passed (65 assertions in 11 test cases)
```

See [this][catch2 filter tags] for more information.

## Puzzles

Puzzles are chess problems that have a clear, short, and unique solution.
They are often used to train a chess player's [tactical][chess tactic] awareness.
Simple puzzles typically present a position where a checkmate can be forced or a large material advantage can be gained.
For more advanced puzzles, one might need to find moves that gain a [positional advantage][positional advantage].

We have selected a number of easy puzzles from the freely available [puzzle database][lichess puzzle db] of [Lichess][lichess].
They can be found in [Tests/Puzzles/](Tests/Puzzles/) and contain three major categories:
- `mateIn1`: checkmate can be forced in one move;
- `mateIn2`: checkmate can be forced in two moves;
- `crushing`: a significant material advantage can be gained.

For each category, three variants are provided:
- `simple`: puzzles do not involve castling or en passant moves;
- `castling`: all puzzles involve castling moves;
- `enPassant`: all puzzles involve en passant moves;

The puzzle files are named according to the category and variant.
For example, the simple mate-in-one puzzles can be found in [Tests/Puzzles/mateIn1_simple.csv](Tests/Puzzles/mateIn1_simple.csv).

### Running puzzles

> :bulb: To run the puzzles locally, you need Python 3 (at least 3.7) and the `chess` and `junit-xml` packages which can be installed through `pip`:
> ```
> pip3 install chess junit-xml
> ```

A Python script is provided ([Tests/puzzlerunner.py](Tests/puzzlerunner.py)) to run puzzles through an engine and verify its solution. It uses the [UCI interface](#uci) to communicate with the engine.
The script can be used as follows:
```
$ ./puzzlerunner.py --engine /path/to/engine/executable [csv files...]
```

For example, to run the simple mate-in-one puzzles using your engine:

```
$ ./puzzlerunner.py --engine $BUILD_DIR/cplchess Puzzles/mateIn1_simple.csv
=== Running puzzles from /.../Tests/Puzzles/mateIn1_simple.csv ===
Running puzzle jrJls ... OK (1.645s)
Running puzzle rZoKr ... OK (0.834s)
[snip]
Running puzzle BEUkI ... OK (0.435s)
Running puzzle EAnMf ... OK (0.027s)
Total time: 25.294s
All tests passed
```

> :bulb: The timing information shown is not very accurate since it's measured in wall-clock time.
> Because running a puzzle involves spawning a new process (the engine) and inter-process communication between the Python script and this process, the measured time will depend on the current load of the system.

When a puzzle fails because the engine returned a wrong move, some information is shown to help debug the issue.
For example:

```
Running puzzle 93ERa ... FAIL (5.034s)
===
Failure reason: unexpected move
URL: https://lichess.org/training/93ERa
position=4rqk1/5rp1/7R/4B3/4P1Q1/6PK/PP5P/2n5 w - - 1 39
move=e5d6
expected move=g4g6
===
```

It shows a link to the puzzle on Lichess, the position from which the wrong move was generated (in [FEN](#fen) notation), and the generated and expected moves.

### Generating puzzles

The full [Lichess puzzle database][lichess puzzle db] currently contains about two million puzzles.
We have selected only a few to be included in the automatic tests but you are encouraged to use more puzzles to test your engine.
To help you with this, we provide a script ([Tests/puzzledb.py](Tests/puzzledb.py)) that can parse and filter the database.

Puzzles can be filtered on a number of criteria, including their [tags][lichess puzzle themes], rating, whether they include castling or en passant moves, etc.
For a full list of options, run `./puzzledb.py --help`.

As an example, the simple mate-in-one puzzles were generated like this:

```
$ ./puzzledb.py --tag=mateIn1 --castling=no --en-passant=no lichess_db_puzzle.csv | sort -R | head -n 20
```

> :bulb: `sort -R` randomly shuffles all matches and `head -n 20` selects the first 20.
> If you run this locally, you will (most likely) get different puzzles than the ones in [Tests/Puzzles/mateIn1_simple.csv](Tests/Puzzles/mateIn1_simple.csv).
> Also note that this is Bash syntax, the full command will not work in other (incompatible) shells but the invocation of `puzzledb.py` itself will.

> :bulb: Note that the options `--tag` and `--not-tag` can be given multiple times and it will filter on puzzles that (don't) have *all* given tags.

> :bulb: [Tests/puzzledb.py](Tests/puzzledb.py) is not used by our testing infrastructure so feel free to modify it to add more filter options.

# Tools

Here we list some freely available tools that you can use while developing your chess engine.

## FEN

[*Forsyth–Edwards Notation*][fen] (FEN) is a standard notation for describing chess positions.
It contains information about piece placement, current turn, castling rights, en passant square, and number of moves made.

A FEN parser is provided (see [Fen.hpp](Fen.hpp) and [Fen.cpp](Fen.cpp)) that converts a FEN string to a `Board`.

> :warning: The FEN parser will not work properly until all `[Fundamental]` [unit tests](#unit-tests) pass.

> :bulb: The FEN parser currently does not support parsing the move number information (nor does `Board` store it) because you don't need it for a simple engine.
> [Some extensions](#draw-conditions) might need it, though, so if you want to implement those, you'll have to extend the FEN parser.

When running the engine executable with a FEN string as argument, the position is evaluated and the PV printed.
For example:

```
$ $BUILD_DIR/cplchess "2kr4/ppp2Q2/3Pp3/1q2P3/n1r4p/5P2/6PB/R1R3K1 b - - 0 29"
PV: +600 [b5c5 g1h1 c4c1 a1c1 c5c1]
```

> :bulb: The FEN string should be passed *as a single argument* so you have to put it in quotes.

> :bulb: Since you are free to choose [how a PV is printed](#principal-variation), the output may look different.

While developing your engine, you will most likely want to test it on specific positions.
To manually create positions and convert them to FEN, the Lichess [board editor][lichess board editor] is a very handy tool.
All unit tests that start from a position (which is most of them) contain a link to the board editor with that specific position in a comment.

> :bulb: The en passant square cannot be set from the editor so you'll have to fill that in by hand if you need it.

## UCI

The [*Universal Chess Protocol*][uci] is a protocol that is mainly used to communicate between chess engines and user interfaces.
It allows user interfaces to send positions (and other information like timing) to an engine and for the engine to send its best move (and optionally a PV and evaluation) back.
See [this][uci protocol] for a description of the protocol.

A UCI implementation is provided (see [Uci.hpp](Uci.hpp) and [Uci.cpp](Uci.cpp)) that allows you to use your engine with a GUI or to let it play against another engine (or itself).
Running the engine executable without any arguments starts it in UCI mode.
It will listen on stdin for commands and write replies to stdout.
It will also log some information to a file called `uci-log.txt` in its current working directory:
- All incoming and outgoing UCI commands;
- After receiving a new position and after getting a move from the engine, the board is printed;
- The PV received from the engine.


There are many [UCI GUIs][uci gui] and all of them should work but the one I use is [Cute Chess][cutechess].

To use your engine in Cute Chess, you first have to add it to its engine list.
Go to "Tools -> Settings" and then to the "Engines" tab to add your engine by clicking on the "+" symbol at the bottom.
Once you added your engine, you can use it by going to "Game -> New" and then selecting "CPU" and your engine for one or both of the players.

> :bulb: Cute Chess also includes a command line tool (`cutechess-cli`) that you can use to make two engines play each other.
> See [this][cutechess cli] and `cutechess-cli -help` for more info.

# Extensions

This section describes some (optional) extensions that could give your engine an edge during the [tournament](#tournament).

## Time control

As described [before](#engine), timing information is passed to the engine.
This can be used to prevent the engine from timing out during games.

## Draw conditions

Besides [stalemate][stalemate], there are some other conditions that can draw a game.
Most notably the [*fifty-move rule*][fifty-move rule] and [*threefold repetition*][threefold repetition].
These rules can be used to turn a losing position into a draw, especially against an engine that does not understand these rules.
They will be applied in the tournament.

# Tournament

We plan to organize a tournament between the engines that pass all tests.
More details will follow.

[san]: https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
[long algebraic notation]: https://en.wikipedia.org/wiki/Algebraic_notation_(chess)#Long_algebraic_notation
[std_map]: https://en.cppreference.com/w/cpp/container/map
[promotion]: https://en.wikipedia.org/wiki/Promotion_(chess)
[uci]: https://en.wikipedia.org/wiki/Universal_Chess_Interface
[board representation wp]: https://en.wikipedia.org/wiki/Board_representation_(computer_chess)
[board representation cpw]: https://www.chessprogramming.org/Board_Representation
[bitboard]: https://en.wikipedia.org/wiki/Bitboard
[stockfish]: https://stockfishchess.org/
[8x8 board]: https://www.chessprogramming.org/8x8_Board
[castling]: https://en.wikipedia.org/wiki/Castling
[bit flags]: https://blog.podkalicki.com/bit-level-operations-bit-flags-and-bit-masks/
[en passant]: https://en.wikipedia.org/wiki/En_passant
[move generation]: https://www.chessprogramming.org/Move_Generation
[pseudo legal move]: https://www.chessprogramming.org/Pseudo-Legal_Move
[legal move]: https://www.chessprogramming.org/Legal_Move
[chess rules]: https://en.wikipedia.org/wiki/Rules_of_chess
[chess rules check]: https://en.wikipedia.org/wiki/Rules_of_chess#Check
[chess rules game end]: https://en.wikipedia.org/wiki/Rules_of_chess#End_of_the_game
[move making]: https://www.chessprogramming.org/Make_Move
[game tree]: https://en.wikipedia.org/wiki/Game_tree
[checkmate]: https://en.wikipedia.org/wiki/Checkmate
[stalemate]: https://en.wikipedia.org/wiki/Stalemate
[minimax]: https://en.wikipedia.org/wiki/Minimax
[zero-sum game]: https://en.wikipedia.org/wiki/Zero-sum_game
[negamax]: https://en.wikipedia.org/wiki/Negamax
[alpha-beta pruning]: https://en.wikipedia.org/wiki/Alpha-beta_pruning
[move ordering]: https://www.chessprogramming.org/Move_Ordering
[iterative deepening]: https://www.chessprogramming.org/Iterative_Deepening
[pv]: https://www.chessprogramming.org/Principal_Variation
[eval]: https://www.chessprogramming.org/Evaluation
[piece value]: https://en.wikipedia.org/wiki/Chess_piece_relative_value
[center control]: https://www.chessprogramming.org/Center_Control
[king safety]: https://www.chessprogramming.org/King_Safety
[ply]: https://en.wikipedia.org/wiki/Ply_(game_theory)
[eval score]: https://www.chessprogramming.org/Score
[centipawns]: https://www.chessprogramming.org/Centipawns
[time control]: https://en.wikipedia.org/wiki/Time_control#Chess
[catch2]: https://github.com/catchorg/Catch2
[catch2 filter tags]: https://github.com/catchorg/Catch2/blob/v2.x/docs/command-line.md#specifying-which-tests-to-run
[chess tactic]: https://en.wikipedia.org/wiki/Chess_tactic
[positional advantage]: https://www.chessstrategyonline.com/content/tutorials/introduction-to-chess-strategy-positional-advantage
[lichess]: https://lichess.org/
[lichess puzzle db]: https://database.lichess.org/#puzzles
[lichess puzzle themes]: https://lichess.org/training/themes
[fen]: https://en.wikipedia.org/wiki/Forsyth-Edwards_Notation
[lichess board editor]: https://lichess.org/editor
[uci]: https://en.wikipedia.org/wiki/Universal_Chess_Interface
[uci protocol]: https://backscattering.de/chess/uci/
[uci gui]: https://www.chessprogramming.org/UCI#GUIs
[cutechess]: https://github.com/cutechess/cutechess
[cutechess cli]: https://github.com/cutechess/cutechess#running
[chess engine]: https://en.wikipedia.org/wiki/Chess_engine
[questions]: https://gitlab.kuleuven.be/distrinet/education/cpl/cplusplus-project-assignment/-/issues
[assignment repo]: https://gitlab.kuleuven.be/distrinet/education/cpl/cplusplus-project-assignment
[exercises setup]: https://gitlab.kuleuven.be/distrinet/education/cpl/cplusplus-exercises-assignment#setup
[git submodules]: https://git-scm.com/book/en/v2/Git-Tools-Submodules
[c++17]: https://en.cppreference.com/w/cpp/17
[c++20]: https://en.cppreference.com/w/cpp/20
[c++ features]: https://en.cppreference.com/w/cpp
[fifty-move rule]: https://en.wikipedia.org/wiki/Fifty-move_rule
[threefold repetition]: https://en.wikipedia.org/wiki/Threefold_repetition
[gcc c++ status]: https://gcc.gnu.org/projects/cxx-status.html
[gitlab pipelines]: https://docs.gitlab.com/ee/ci/pipelines/
[gcc warnings]: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
[std optional]: https://en.cppreference.com/w/cpp/utility/optional
