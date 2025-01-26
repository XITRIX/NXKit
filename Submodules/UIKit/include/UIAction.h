//
// Created by Даниил Виноградов on 26.01.2025.
//

#pragma once

#include <string>
#include <functional>
#include <utility>

namespace NXKit {

class UIAction {
public:
    explicit UIAction(std::string title = "", std::function<void()> handler = []() {});

    std::string title() { return _title; }
    void setTitle(std::string title) { _title = std::move(title); }

//    uint identifier() { return _id; }
private:
    friend class UIControl;

    std::function<void()> _handler;
    std::string _title;
//    uint _id;
};

}