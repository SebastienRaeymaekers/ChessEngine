#include "EngineFactory.hpp"
#include "Engine.hpp"
#include "ChessEngine.hpp"

#include <ostream>
#include <cassert>
#include <cmath>
#include <memory>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

std::unique_ptr<Engine> EngineFactory::createEngine() {
    return std::unique_ptr<Engine>(new ChessEngine());
}
