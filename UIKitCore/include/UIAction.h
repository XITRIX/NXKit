//
// Created by Даниил Виноградов on 26.01.2025.
//

#pragma once

#include <string>
#include <functional>
#include <utility>

namespace NXKit {

// UIKit exposes this policy through UIMenuLeaf::repeatBehavior. NXKit's
// current UIAction subset does not yet include the menu-element hierarchy, so
// the same policy is surfaced directly on UIAction for responder commands.
enum class UIMenuElementRepeatBehavior {
    automatic,
    repeatable,
    nonRepeatable,
};

class UIAction {
public:
    explicit UIAction(std::string title = "", std::function<void()> handler = []() {});

    [[nodiscard]] const std::string& title() const { return _title; }
    void setTitle(std::string title) { _title = std::move(title); }

    [[nodiscard]] UIMenuElementRepeatBehavior repeatBehavior() const {
        return _repeatBehavior;
    }
    void setRepeatBehavior(UIMenuElementRepeatBehavior repeatBehavior) {
        _repeatBehavior = repeatBehavior;
    }

//    uint identifier() { return _id; }
    void perform() const { _handler(); }
private:
    friend class UIControl;

    std::function<void()> _handler;
    std::string _title;
    UIMenuElementRepeatBehavior _repeatBehavior =
        UIMenuElementRepeatBehavior::automatic;
//    uint _id;
};

}
