//
//  UIScrollView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>

namespace NXKit {

class UIScrollView: public UIView {
public:
    UIScrollView();
    Point getContentOffset();
    void setContentOffset(Point offset);

    void layoutSubviews() override;

    void setFixWidth(bool fix);
    void setFixHeight(bool fix);
    
private:
    UIEdgeInsets lastSafeAreaInset;
    bool fixWidth = false, fixHeight = false;
};

}
