#pragma once

#include <any>
#include <set>
#include <vector>
#include <memory>
#include <cstdio>
#include <string>
#include <cassert>
#include <optional>
#include <functional>

#include "OptionSet.hpp"
#include "SharedBase.hpp"

namespace NXKit {

template< typename T >
std::optional<T> any_optional_cast(std::optional<std::any> obj) {
    if (!obj.has_value()) { return std::nullopt; }

    try {
        return std::any_cast<T>(obj.value()); // throws
    }
    catch(const std::bad_any_cast& e) {
        return std::nullopt;
    }
}

}
