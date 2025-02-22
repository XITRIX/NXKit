//
// Created by Даниил Виноградов on 22.02.2025.
//

#include "NXTestScreen.h"

class NXNavigationBar: public UIView {
public:
    NXNavigationBar() {
        auto headerContent = new_shared<UIView>();
        headerContent->setAutolayoutEnabled(true);

        _imageView = new_shared<UIImageView>();
        _titleLabel = new_shared<UILabel>();

        _imageView->setContentMode(UIViewContentMode::scaleAspectFill);
        _imageView->setImage(UIImage::fromRes("img/moonlight_96.png", 3));

        _titleLabel->setFontSize(28);

        _imageView->setAutolayoutEnabled(true);
        _titleLabel->setAutolayoutEnabled(true);

        headerContent->addSubview(_imageView);
        headerContent->addSubview(_titleLabel);

        _imageView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setSize({ 58, 58 });
        });

        headerContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setAllGap(12);
            layout->setMarginTop(14_pt);
            layout->setMarginHorizontal(30_pt);
            layout->setFlexGrow(1);
//        layout->setSize({ 250, 50 });
        });

        addSubview(headerContent);

        auto separator = new_shared<UIView>();
        separator->setAutolayoutEnabled(true);
        separator->setBackgroundColor(UIColor::label);
        separator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setHeight(1_pt);
        });
        addSubview(separator);

        configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionColumn);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignStretch);
//            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setMarginHorizontal(30_pt);
            layout->setHeight(88_pt);
        });
    }

    void setTitle(const std::string& text) {
        _titleLabel->setText(text);
    }
private:
    std::shared_ptr<UIImageView> _imageView;
    std::shared_ptr<UILabel> _titleLabel;
};

class NXFooterBar: public UIView {
public:
    NXFooterBar() {
        auto headerContent = new_shared<UIView>();
        headerContent->setAutolayoutEnabled(true);

        _imageView = new_shared<UIImageView>();
        _titleLabel = new_shared<UILabel>();

        _imageView->setContentMode(UIViewContentMode::center);
        _imageView->setImage(UIImage::fromRes("img/sys/battery_back_light.png", 1));

        _titleLabel->setFontSize(21.5);

        _imageView->setAutolayoutEnabled(true);
        _titleLabel->setAutolayoutEnabled(true);

        headerContent->addSubview(_imageView);
        headerContent->addSubview(_titleLabel);

        _imageView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setSize({ 40, 40 });
        });

        headerContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setAllGap(30);
            layout->setPaddingHorizontal(30_pt);
            layout->setFlexGrow(1);
//        layout->setSize({ 250, 50 });
        });


        auto separator = new_shared<UIView>();
        separator->setAutolayoutEnabled(true);
        separator->setBackgroundColor(UIColor::label);
        separator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setHeight(1_pt);
        });
        addSubview(separator);
        addSubview(headerContent);

        configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionColumn);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignStretch);
//            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setMarginHorizontal(30_pt);
            layout->setHeight(73_pt);
        });
    }

    void setTitle(const std::string& text) {
        _titleLabel->setText(text);
    }
private:
    std::shared_ptr<UIImageView> _imageView;
    std::shared_ptr<UILabel> _titleLabel;
};

void NXNavigationController::loadView() {
    auto view = new_shared<UIView>();
    view->setBackgroundColor(UIColor::systemBackground);

    auto header = new_shared<NXNavigationBar>();
    header->setAutolayoutEnabled(true);
    header->setTitle("Системные настройки");

    auto footer = new_shared<NXFooterBar>();
    footer->setAutolayoutEnabled(true);
    footer->setTitle("16:21:53");

    view->addSubview(header);
    view->addSubview(footer);

    view->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setJustifyContent(YGJustifySpaceBetween);
        layout->setAlignItems(YGAlignStretch);
    });

    setView(view);
}

void NXNavigationController::viewDidLoad() {

}