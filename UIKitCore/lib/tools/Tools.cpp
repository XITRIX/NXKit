//
//  Utils.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 09.02.2023.
//

#include <cstdio>
#include <string>
#include <cassert>
#include <tools/Tools.hpp>

namespace NXKit {

bool startsWith(const std::string& data, const std::string& prefix)
{
    return data.rfind(prefix, 0) == 0;
}

bool endsWith(const std::string& data, const std::string& suffix)
{
    return data.find(suffix, data.size() - suffix.size()) != std::string::npos;
}

}
