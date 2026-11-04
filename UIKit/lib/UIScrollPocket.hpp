#pragma once
#include <UIView.h>
#include <UIViewController.h>
#include <UIScrollView.h>
#include <array>
namespace NXKit::Internal {
struct ScrollPocketSnapshot {
    std::shared_ptr<UIScrollView> scroll;
    NXRect viewport;
    NXFloat safeExtent=0,effectExtent=0,strength=0;
    UIScrollEdgeEffectStyle style=UIScrollEdgeEffectStyle::automatic;
    bool visible=false;
};
ScrollPocketSnapshot scrollPocketSnapshot(UIViewController&,const std::shared_ptr<UIView>& host,int edge);
std::shared_ptr<UIView> makeScrollPocketHost(const std::shared_ptr<UIViewController>&);
void updateScrollPocketHost(const std::shared_ptr<UIView>&);
}
