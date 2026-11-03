#pragma once
#include <UINavigationBar.h>
#include <BackdropEffectView.h>
#include <GlassContainerView.h>
#include <UIControl.h>
#include <UILabel.h>
#include <UIImageView.h>
namespace NXKit::Internal {
struct NavigationButton {
    std::shared_ptr<UIBarButtonItem> item;
    std::shared_ptr<UIControl> control;
    std::shared_ptr<UILabel> label;
    std::shared_ptr<UIImageView> image;
    std::shared_ptr<UIView> ink;
    std::shared_ptr<UIView> custom;
    bool back=false,spacer=false;
    NXFloat width=44;
};
struct NavigationSide {
    struct Group {
        std::shared_ptr<UIView> view;
        std::vector<std::shared_ptr<NavigationButton>> buttons;
        std::optional<UIColor> tint;
        bool hiddenBackground=false;
    };
    std::vector<std::shared_ptr<NavigationButton>> buttons;
    std::vector<std::shared_ptr<BackdropEffectView>> backgrounds;
    std::vector<Group> groups;
};
struct NavigationBarState {
    std::vector<std::shared_ptr<UINavigationItem>> items;
    std::weak_ptr<UINavigationBarDelegate> delegate;
    std::weak_ptr<UIViewController> owner;
    std::function<void()> backAction;
    std::shared_ptr<GlassContainerView> glass;
    std::shared_ptr<UILabel> title;
    std::shared_ptr<UIView> titleInk;
    std::weak_ptr<UINavigationItem> renderedItem;
    std::shared_ptr<UIView> customTitle;
    std::shared_ptr<UIBarButtonItem> backButton;
    NavigationSide left,right;
    std::vector<std::weak_ptr<UIView>> retiring;
    bool transitioning=false;
    bool refreshing=false;
    size_t revision=0,generation=0;
    std::optional<std::vector<std::shared_ptr<UINavigationItem>>> interactiveSource;
    std::shared_ptr<UIView> interactiveTitle;
    double interactiveHandoff=-1;
};
class NavigationBarAccess {
public:
    static auto& state(UINavigationBar& bar) {return *bar._state;}
    static void setItems(UINavigationBar& bar,std::vector<std::shared_ptr<UINavigationItem>> items,bool animated,int direction) {bar.install(std::move(items),animated,direction);}
    static void beginInteractivePop(UINavigationBar&,std::vector<std::shared_ptr<UINavigationItem>>,double span);
    static void endInteractivePop(UINavigationBar&,bool commit,double progress);
};
}
