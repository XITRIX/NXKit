#include "GlassDemoViewController.hpp"
#include "GlassDemoScene.hpp"
#include "LensDemoStage.hpp"
#include "GlassScreenViewController.hpp"
#include <algorithm>

using namespace NXKit;
using namespace NXKit::yoga::literals;
namespace {
void updateGlassContent(const std::shared_ptr<UIScrollView>& scroll,
                        const std::shared_ptr<UIView>& content) {
    if (!scroll || !content) return;
    const auto safe=scroll->safeAreaInsets();
    const auto availableWidth=scroll->bounds().width();
    const auto width=availableWidth>0 ? std::min<NXFloat>(720,availableWidth) : 720;
    // Match Navigation Test's measured, centered column and safe-area padding.
    content->configureLayout([safe,width](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(YGValue{width,YGUnitPoint});
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setAllGap(16);
        layout->setPaddingLeft(48_pt);
        layout->setPaddingRight(48_pt);
        layout->setPaddingTop(YGValue{safe.top+36,YGUnitPoint});
        layout->setPaddingBottom(YGValue{safe.bottom+36,YGUnitPoint});
    });
    scroll->setContentSize({std::max<NXFloat>(0,availableWidth),
                           std::max<NXFloat>(0,content->frame().height())});
}

std::shared_ptr<UIButton> button(const std::string& title, std::function<void()> action) {
    auto view=new_shared<UIButton>(UIButtonStyle::tinted); view->setText(title);
    view->layer()->setCornerRadius(8);
    view->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setWidth(100_percent);
        layout->setHeight(64_pt);
        layout->setPaddingHorizontal(20_pt);
    });
    view->primaryAction=UIAction(title,std::move(action)); return view;
}
class LensDemoViewController final : public UIViewController {
public:
    explicit LensDemoViewController(bool tabBarOnly = false):_tabBarOnly(tabBarOnly) {
        setTitle(tabBarOnly?"Interactive glass tab bar":"Lens and tab bar study");
    }
    void loadView() override {
        _scroll=new_shared<UIScrollView>(); _scroll->setBackgroundColor(UIColor::systemBackground);
        _scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
        _scroll->configureLayout([](const auto& layout) { layout->setAlignItems(YGAlignCenter); });
        _content=new_shared<UIView>(); updateGlassContent(_scroll,_content);
        auto detail=new_shared<UILabel>();
        detail->setText(_tabBarOnly
            ?"Tap a tab, or drag and release the selection. Both the bar and selection react to touch. Lift holds the selection raised; Refraction grid reveals the edges."
            :"Tap a tab or drag and release the selection. Press and pull the standalone knobs to explore their liquid response, or use Lift to keep them raised.");
        detail->setFontSize(20); detail->setTextColor(UIColor::secondaryLabel); _content->addSubview(detail);
        _stage=new_shared<LensDemoStage>(_tabBarOnly);
        _stage->layer()->setCornerRadius(20);
        _stage->configureLayout([](const auto& layout) { layout->setWidth(100_percent); layout->setHeight(440_pt); layout->setFlexShrink(0); }); _content->addSubview(_stage);
        const auto weak=weak_from_base<LensDemoViewController>();
        _content->addSubview(button("Lift / Rest",[weak] { if (auto self=weak.lock()) self->_stage->setLifted(!self->_stage->isLifted()); }));
        _content->addSubview(button("Next tab",[weak] { if (auto self=weak.lock()) self->_stage->selectNext(); }));
        _content->addSubview(button("Backdrop",[weak] { if (auto self=weak.lock()) self->_stage->setBackdrop(self->_stage->backdrop()+1); }));
        _content->addSubview(button("Refraction grid",[weak] { if (auto self=weak.lock()) self->_stage->setBackdrop(5); }));
        _content->addSubview(button("Change size",[weak] { if (auto self=weak.lock()) self->_stage->toggleSize(); }));
        _scroll->addSubview(_content); setView(_scroll);
    }
    void viewDidLayoutSubviews() override { UIViewController::viewDidLayoutSubviews(); updateGlassContent(_scroll,_content); }
    void viewSafeAreaInsetsDidChange() override { UIViewController::viewSafeAreaInsetsDidChange(); updateGlassContent(_scroll,_content); }
private:
    bool _tabBarOnly;
    std::shared_ptr<UIScrollView> _scroll; std::shared_ptr<UIView> _content; std::shared_ptr<LensDemoStage> _stage;
};
class GlassSceneViewController final : public UIViewController {
public:
    explicit GlassSceneViewController(GlassDemoID demo):_demo(demo) { setTitle(GlassDemos[demo].title); }
    void loadView() override {
        _scroll=new_shared<UIScrollView>(); _scroll->setBackgroundColor(UIColor::systemBackground);
        _scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
        _scroll->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setAlignItems(YGAlignCenter);
        });
        _contentContainer=new_shared<UIView>();
        updateGlassContent(_scroll,_contentContainer);
        auto detail=new_shared<UILabel>(); detail->setText(GlassDemos[_demo].detail);
        detail->setFontSize(20); detail->setTextColor(UIColor::secondaryLabel);
        detail->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setMarginBottom(12_pt);
        });
        _contentContainer->addSubview(detail);
        _holder=new_shared<UIView>(); _holder->setClipsToBounds(true); _holder->layer()->setCornerRadius(20);
        _holder->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setWidth(100_percent);
            layout->setAspectRatio(720.0f/440.0f);
            layout->setFlexShrink(0);
        });
        _scene=new_shared<GlassDemoScene>(_demo,UIImage::fromRes("img/glass_demo_scene.png",1));
        // Yoga owns the holder. Fixed button examples retain point-sized touch
        // targets; the original optical scenes keep their logical coordinates.
        _scene->setAutolayoutEnabled(false);
        _scene->layer()->setAnchorPoint({0,0}); _holder->addSubview(_scene); _contentContainer->addSubview(_holder);
        const auto weak=weak_from_base<GlassSceneViewController>();
        _buttons.push_back(button("Backdrop",[weak] { if (auto self=weak.lock()) self->_scene->setBackdrop((self->_scene->backdrop()+1)%5); }));
        _buttons.push_back(button("Reset",[weak] { if (auto self=weak.lock()) self->_scene->reset(); }));
        if (GlassDemos[_demo].action[0]) _buttons.push_back(button(GlassDemos[_demo].action,[weak] {
            if (auto self=weak.lock()) {
                auto scene=self->_scene; const auto target=scene->progress()<.5f ? 1.0f : 0.0f;
                UIView::animate(.7,0,static_cast<UIViewAnimationOptions>(UIViewAnimationOptions::allowUserInteraction | UIViewAnimationOptions::beginFromCurrentState),
                    [scene,target] { scene->setProgress(target); });
            }
        }));
        if (_demo==GlassDemoTwoTints) _buttons.push_back(button("Swap order",[weak] {
            if (auto self=weak.lock()) self->_scene->setReversedOrder(!self->_scene->reversedOrder());
        }));
        for (auto& control:_buttons) _contentContainer->addSubview(control);
        _scroll->addSubview(_contentContainer);
        setView(_scroll);
    }
    void viewSafeAreaInsetsDidChange() override {
        UIViewController::viewSafeAreaInsetsDidChange();
        updateGlassContent(_scroll,_contentContainer);
    }
    void viewDidLayoutSubviews() override {
        UIViewController::viewDidLayoutSubviews();
        updateGlassContent(_scroll,_contentContainer);
        const bool fixedButtons=GlassDemoHasButtons(_demo);
        const auto scale=fixedButtons ? 1 : _holder->bounds().width()/720;
        _scene->setBounds(fixedButtons ? _holder->bounds() : NXRect(0,0,720,440));
        _scene->setTransform(NXAffineTransform::scale(scale));
        _scene->layer()->setPosition({0,0});
    }
private:
    GlassDemoID _demo;
    std::shared_ptr<UIScrollView> _scroll;
    std::shared_ptr<UIView> _contentContainer;
    std::shared_ptr<UIView> _holder;
    std::shared_ptr<GlassDemoScene> _scene;
    std::vector<std::shared_ptr<UIButton>> _buttons;
};
}
GlassDemoViewController::GlassDemoViewController() { setTitle("Glass"); }
void GlassDemoViewController::loadView() {
    _scroll=new_shared<UIScrollView>(); _scroll->setBackgroundColor(UIColor::systemBackground);
    _scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
    _scroll->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setAlignItems(YGAlignCenter);
    });
    _contentContainer=new_shared<UIView>();
    updateGlassContent(_scroll,_contentContainer);
    auto heading=new_shared<UILabel>(); heading->setText("Liquid Glass");
    heading->setFontSize(30); heading->setFontWeight(600); heading->setAutolayoutEnabled(true);
    auto detail=new_shared<UILabel>(); detail->setText("Choose a demo. Use Back or B to return to this collection.");
    detail->setFontSize(20); detail->setTextColor(UIColor::secondaryLabel); detail->setAutolayoutEnabled(true);
    detail->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setMarginBottom(12_pt);
    });
    _contentContainer->addSubview(heading); _contentContainer->addSubview(detail);
    const auto weak=weak_from_base<GlassDemoViewController>();
    _contentContainer->addSubview(button("Full-screen iOS layout",[weak] {
        if(auto self=weak.lock()) self->show(new_shared<GlassScreenViewController>(),self);
    }));
    _contentContainer->addSubview(button("Interactive tab bar",[weak] {
        if (auto self=weak.lock()) self->show(new_shared<LensDemoViewController>(true),self);
    }));
    for (int i=0;i<GlassDemoCount;++i) {
        auto item=button(GlassDemos[i].title,[weak,i] {
            if (auto self=weak.lock()) self->show(new_shared<GlassSceneViewController>(static_cast<GlassDemoID>(i)),self);
        });
        _contentContainer->addSubview(item);
    }
    _contentContainer->addSubview(button("Lens · tab bar and knobs",[weak] {
        if (auto self=weak.lock()) self->show(new_shared<LensDemoViewController>(),self);
    }));
    _scroll->addSubview(_contentContainer);
    setView(_scroll);
}
void GlassDemoViewController::viewSafeAreaInsetsDidChange() {
    UIViewController::viewSafeAreaInsetsDidChange();
    updateGlassContent(_scroll,_contentContainer);
}
void GlassDemoViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    updateGlassContent(_scroll,_contentContainer);
}
