//
//  Fatal.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 14.03.2023.
//

#pragma once

#include <string>
#include <stdexcept>

namespace NXKit {

/**
 * Prints the given error message message and throws a std::logic_error.
 */
[[noreturn]] void fatal(const std::string& message);

}
