add_subdirectory(Catch2/)

add_executable(tests
    Main.cpp
    SquareTests.cpp
    MoveTests.cpp
    PieceTests.cpp
    BoardTests.cpp
    FenTests.cpp
    EngineTests.cpp
)

target_link_libraries(tests cplchess_lib Catch2::Catch2)

include(Catch2/contrib/Catch.cmake)
catch_discover_tests(tests)
