#ifndef CHESS_ENGINE_TESTS_TESTUTILS_HPP
#define CHESS_ENGINE_TESTS_TESTUTILS_HPP

#include <optional>
#include <memory>
#include <ostream>

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt) {
    if (opt.has_value()) {
        return os << opt.value();
    } else {
        return os << "nullopt";
    }
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::unique_ptr<T>& ptr) {
    if (ptr == nullptr) {
        return os << "nullptr";
    } else {
        return os << ptr.get();
    }
}

#endif
