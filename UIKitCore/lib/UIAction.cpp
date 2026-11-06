//
// Created by Даниил Виноградов on 26.01.2025.
//

#include <UIAction.h>

#include <utility>
#include <atomic>

namespace NXKit {

UIAction::UIAction(std::string title, std::function<void()> handler)
    : UIAction(std::move(title), {}, [handler=std::move(handler)](const UIAction&) {
        if (handler) handler();
    }) {}

UIAction::UIAction(std::string title, UIActionIdentifier identifier,
                   std::function<void(const UIAction&)> handler)
    : _handler(std::move(handler)), _identifier(std::move(identifier)), _title(std::move(title)) {
    static std::atomic<unsigned long long> nextIdentifier{0};
    if (_identifier.empty()) _identifier = "NXKit.UIAction." + std::to_string(++nextIdentifier);
}

}
