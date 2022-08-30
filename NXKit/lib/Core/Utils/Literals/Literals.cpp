//
//  Literals.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#include <Core/Utils/Literals/Literals.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

std::string operator"" _res(const char* str, size_t len) {
    return Application::shared()->getResourcesPath() + std::string(str, len);
}

}

