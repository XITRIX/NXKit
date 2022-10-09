//
//  UITableViewRadioCell.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#include <Core/UITableViewRadioCell/UITableViewRadioCell.hpp>

namespace NXKit {

UICheckBoxView::UICheckBoxView() {
    setSize({ 30, 30 });
}

void UICheckBoxView::draw(NVGcontext* vgContext) {
    float radius  = getBounds().width() / 2;
    float centerX = getBounds().midX();
    float centerY = getBounds().midY();

    int thickness = roundf(radius * 0.10f);

    // Background
    nvgFillColor(vgContext, getTintColor().raw());
    nvgBeginPath(vgContext);
    nvgCircle(vgContext, centerX, centerY, radius);
    nvgFill(vgContext);

    // Check mark
    nvgFillColor(vgContext, UIColor::systemBackground.raw());

    // Long stroke
    nvgSave(vgContext);
    nvgTranslate(vgContext, centerX, centerY);
    nvgRotate(vgContext, -NVG_PI / 4.0f);

    nvgBeginPath(vgContext);
    nvgRect(vgContext, -(radius * 0.55f), 0, radius * 1.3f, thickness);
    nvgFill(vgContext);
    nvgRestore(vgContext);

    // Short stroke
    nvgSave(vgContext);
    nvgTranslate(vgContext, centerX - (radius * 0.65f), centerY);
    nvgRotate(vgContext, NVG_PI / 4.0f);

    nvgBeginPath(vgContext);
    nvgRect(vgContext, 0, -(thickness / 2), radius * 0.53f, thickness);
    nvgFill(vgContext);

    nvgRestore(vgContext);
}

UITableViewRadioCell::UITableViewRadioCell() {
    setHeight(70);
    setPaddingTop(12.5f);
    setPaddingBottom(12.5f);

    setPadding(12.5f, 20, 12.5f, 20);

    setAxis(Axis::HORIZONTAL);
    setAlignItems(AlignItems::CENTER);

    imageView = NXKit::make_shared<UIImageView>();
    imageView->setSize({ 44, 44 });
    imageView->setMarginRight(20);

    label = NXKit::make_shared<UILabel>("Test text");
    label->setGrow(1);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;

    checkbox = NXKit::make_shared<UICheckBoxView>();

    addSubview(imageView);
    addSubview(label);
    addSubview(checkbox);

    onEvent = [this](UIControlTouchEvent event) {
        if (event == NXKit::UIControlTouchEvent::touchUpInside) {
            getFirstAvailableAction(BUTTON_A).action();
        }
    };
}

void UITableViewRadioCell::setText(std::string text) {
    label->setText(text);
}

void UITableViewRadioCell::setOn(bool isOn) {
    checkbox->setHidden(!isOn);
}

void UITableViewRadioCell::setImage(std::shared_ptr<UIImage> image) {
    imageView->setHidden(!image);
    imageView->setImage(image);
}

}
