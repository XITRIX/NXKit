//
// Created by Даниил Виноградов on 26.01.2025.
//

#include <UIAction.h>

#include <utility>

namespace NXKit {

UIAction::UIAction(std::string title, std::function<void()> handler):
    _title(std::move(title)), _handler(std::move(handler))
{ }

}