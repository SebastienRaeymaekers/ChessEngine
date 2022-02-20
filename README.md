
<!-- PROJECT LOGO -->
<br />
<p align="center">
  <h1 align="center">Chess Engine</h1>

  <p align="center">
    Given a chess board, find the best move using the negamax algorithm.
    <br />
    <br />
  </p>
</p>


## Fundamental data structures

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


### Square

Chessboards consist of a grid of 8x8 *squares*.
In [*algebraic notation*][san], each square is identified by a coordinate pair from white's point of view.
Columns (called *files*) are labeled *a* through *h* while rows (called *ranks*) are labeled *1* through *8*.
So, the lower-left square is called *a1* while the upper-right square is called *h8*.

The `Square` class (implemented in [Square.hpp](Square.hpp) and [Square.cpp](Square.cpp)) is used to identify squares.
It offers two ways of identifying squares:
- Coordinates: create using `fromCoordinates()` and get using `file()` and `rank()`.
- Index: create using `fromIndex()` and get using `index()`.
  In this representation, all squares have a unique index in the range `[0, 64)` where the index of *a1* is *0*, *b1* is *1*, and that of *h8* is *63*.

Squares also offer conversion from (`fromName`) and to (streaming to `std::ostream`) their name.

Two comparison operator are used:
- `operator==` for comparing squares in the tests and elsewhere;
- `operator<` for using squares as keys in associative (sorted) containers like [`std::map`][std_map].
  You can use any total ordering.

Constants are provided to conveniently references all squares (e.g., `Square::E4`).
These are implemented using a private constructor that takes an index as argument.

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

The following format is used:

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


[*Move generation*][move generation] is the process of generating valid moves from a board state.
These moves will be used for [searching](#searching).

There are generally two ways of generating moves: legal or pseudo-legal.

### Legal move generation

Only moves that are [legal][legal move] according to the [rules of chess][chess rules] are generated.

### Pseudo-legal move generation

A [*pseudo-legal move*][pseudo legal move] is one that adheres to all the rules of chess *except* that it might leave the king in check.
We now have to make sure that we somehow reject illegal moves while searching.
One way to tackle this could be to search until a move that captures the king and then reject the previous move.


### Interface

Move generation should be implemented by the `Board` class through the following method:

```c++
using MoveVec = std::vector<Move>;

void Board::pseudoLegalMoves(MoveVec& moves) const;
```

This method adds all (pseudo) legal moves for the current player to the `moves` vector.

For [testing](#unit-tests) purposes, the following method should also be implemented to generate all moves from a specific square:

```c++
void Board::pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const;
```


## Move making

[*Move making*][move making] is the process of updating the board state to reflect a move.
Obviously, this should include moving the moved piece from the source square to the destination square (possibly replacing a captured piece) but it may also include updating [castling rights](#castling-rights) and the [en passant square](#en-passant-square).

Move making is used by the search algorithm to generate child nodes after move generation.

It is implemented in the `Board` class by the following method:

```c++
void Board::makeMove(const Move& move);
```

## Searching

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

[Evaluating][eval] chess positions is very complex.
The most naive way is to simply calculate the [value][piece value] of all remaining pieces.
However, this is clearly not optimal so more advanced strategies will take other aspects into account (e.g., [center control][center control], [king safety][king safety], etc).


Remember that if you use [pseudo-legal move generation](#pseudo-legal-move-generation), the search algorithm needs to somehow reject illegal moves.
And unless you evaluation function can detect [checkmate][checkmate] and [stalemate][stalemate], it also needs to be able to handle [end of the game][chess rules game end] conditions.

### Principal variation

The result of a search is a [*principal variation*][pv] (PV) which is the sequence of moves the engine considers best.
A PV is represented by the `PrincipalVariation` class (in [PrincipalVariation.hpp](PrincipalVariation.hpp) and [PrincipalVariation.cpp](PrincipalVariation.cpp)).
The main interface is the following:

```c++
using MoveIter = ... ;

std::size_t PrincipalVariation::length() const;
MoveIter PrincipalVariation::begin() const;
MoveIter PrincipalVariation::end() const;
```

Where `length()` returns the number of [plies][ply] in the PV and `begin()` and `end()` allows for iterating over the `Move`s.

The `PrincipalVariation` class also stores information about the evaluation [score][eval score]:

```c++
bool PrincipalVariation::isMate() const;
int PrincipalVariation::score() const;
```

`isMate()` returns `true` if the PV ends in checkmate.
In this case, `score()` returns the number of plies that leads to the checkmate.
Otherwise, `score()` returns the evaluation of the position the PV leads to.
In both cases, the score is from the point of view of the engine: positive values are advantageous for the engine, negative ones for its opponent.


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
    const TimeInfo::Optional& timeInfo
) = 0;
```

This calculates and returns the PV starting from the position represented by `board`.

The following method is called whenever a new game starts (not guaranteed to be called for the first game played by an `Engine` instance):

```c++
virtual void Engine::newGame()
```

Lastly, the following methods are used to identify the engine over the [UCI interface](#uci):

```c++
virtual std::string Engine::name()
virtual std::string Engine::version()
virtual std::string Engine::author()
```

## Puzzles

Puzzles are chess problems that have a clear, short, and unique solution.
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


# Tools

Here we list some freely available tools that were used while developing your chess engine.

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

While developing your engine, you will most likely want to test it on specific positions.
To manually create positions and convert them to FEN, the Lichess [board editor][lichess board editor] is a very handy tool.
All unit tests that start from a position (which is most of them) contain a link to the board editor with that specific position in a comment.

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


