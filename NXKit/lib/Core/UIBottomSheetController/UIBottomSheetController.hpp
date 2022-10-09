//
//  UIBottomSheetController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 14.09.2022.
//

#pragma once

#include <Core/UINavigationController/UINavigationController.hpp>

namespace NXKit {

class UIBottomSheetController: public UIViewController {
public:
    UIBottomSheetController(std::shared_ptr<UIViewController> rootViewController);

    void loadView() override;

protected:
    void makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion = [](){}) override;
    void makeViewDisappear(bool animated, std::function<void(bool)> completion) override;

private:
    std::shared_ptr<UIViewController> rootViewController;
};

}
