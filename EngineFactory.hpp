#ifndef CHESS_ENGINE_ENGINEFACTORY_HPP
#define CHESS_ENGINE_ENGINEFACTORY_HPP

#include "Engine.hpp"

#include <iosfwd>
#include <optional>
#include <string>
#include <memory>   

class EngineFactory {
public:
    static std::unique_ptr<Engine> createEngine();
};

#endif
