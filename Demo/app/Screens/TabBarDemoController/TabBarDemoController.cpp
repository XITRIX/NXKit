#include "TabBarDemoController.hpp"
#include <UIKit.h>
#include <algorithm>

using namespace NXKit;

namespace {
class TabContent final : public UIViewController {
public:
    explicit TabContent(std::string name) { setTitle(std::move(name)); }
    void loadView() override {
        auto root=new_shared<UIView>(); root->setBackgroundColor(UIColor::systemBackground);
        _heading=new_shared<UILabel>(); _heading->setText(title());
        _heading->setFontSize(28); _heading->setFontWeight(700); root->addSubview(_heading);
        _detail=new_shared<UILabel>(); _detail->setText("Content state survives tab changes.");
        _detail->setFontSize(15); root->addSubview(_detail);
        _button=new_shared<UIButton>(UIButtonStyle::filled);
        _button->setText("Increment badge: 0");
        _button->primaryAction=UIAction("Increment badge",[weak=weak_from_base<TabContent>()] {
            if (const auto screen=weak.lock()) {
                const auto count=std::to_string(++screen->_count);
                screen->tabBarItem()->setBadgeValue(count);
                screen->_button->setText("Increment badge: "+count);
            }
        });
        root->addSubview(_button); setView(root);
    }
    void viewDidLayoutSubviews() override {
        const auto safe=view()->safeAreaInsets();
        const auto width=std::max(0.0f,view()->bounds().width()-safe.left-safe.right-40);
        const auto x=safe.left+20,y=safe.top+24;
        _heading->setFrame({x,y,width,40}); _detail->setFrame({x,y+52,width,24});
        _button->setFrame({x,y+100,std::min(width,260.0f),52});
    }
    void viewSafeAreaInsetsDidChange() override { view()->setNeedsLayout(); }
private:
    int _count=0;
    std::shared_ptr<UILabel> _heading,_detail;
    std::shared_ptr<UIButton> _button;
};
}

TabBarDemoController::TabBarDemoController() {
    setTitle("Apple tabs");
    std::vector<std::shared_ptr<UIViewController>> screens;
    for (const auto& name : {"Library","Favorites","Downloads"}) {
        auto screen=new_shared<TabContent>(name);
        // The bundled image also works on Switch, where SF Symbols are absent.
        auto image=UIImage::fromRes("img/moonlight_96.png",3);
        screen->setTabBarItem(std::make_shared<UITabBarItem>(name,image,int(screens.size())));
        screens.push_back(screen);
    }
    setViewControllers(std::move(screens));
}
