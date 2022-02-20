cmake_minimum_required(VERSION 3.16)

project(cplchess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else ()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif ()

add_library(cplchess_lib OBJECT
    Square.cpp
    Move.cpp
    Piece.cpp
    Board.cpp
    CastlingRights.cpp
    MoveGeneration.cpp
    NegaMax.cpp
    Fen.cpp
    PrincipalVariation.cpp
    EngineFactory.cpp
    Uci.cpp
    ChessEngine.cpp
)

target_include_directories(cplchess_lib PUBLIC .)

add_executable(cplchess Main.cpp)
target_link_libraries(cplchess cplchess_lib)

include(CTest)
add_subdirectory(Tests/)
