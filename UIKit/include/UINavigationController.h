#pragma once
#include <UIViewController.h>
#include <UINavigationBar.h>
#include <deque>
namespace NXKit {
class UINavigationController;
class UINavigationControllerDelegate {
public:
    virtual ~UINavigationControllerDelegate()=default;
    virtual void navigationControllerWillShow(const std::shared_ptr<UINavigationController>&,const std::shared_ptr<UIViewController>&,bool) {}
    virtual void navigationControllerDidShow(const std::shared_ptr<UINavigationController>&,const std::shared_ptr<UIViewController>&,bool) {}
};
/** Apple navigation container with collapsible large titles and a reversible leading-edge pop gesture.
    Toolbar presentation is not yet implemented. Stack changes
    are serialized, including changes requested during an interactive pop. */
class UINavigationController : public UIViewController, public UINavigationControllerInterface, public UINavigationBarDelegate {
public:
    UINavigationController();
    explicit UINavigationController(const std::shared_ptr<UIViewController>& rootViewController);
    ~UINavigationController() override;
    [[nodiscard]] std::vector<std::shared_ptr<UIViewController>> viewControllers() const override {return _controllers;}
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers,bool animated=false) override;
    void pushViewController(const std::shared_ptr<UIViewController>& controller,bool animated) override;
    std::shared_ptr<UIViewController> popViewController(bool animated) override;
    std::vector<std::shared_ptr<UIViewController>> popToViewController(const std::shared_ptr<UIViewController>& controller,bool animated) override;
    std::vector<std::shared_ptr<UIViewController>> popToRootViewController(bool animated) override;
    [[nodiscard]] std::shared_ptr<UIViewController> topViewController() const override;
    [[nodiscard]] std::shared_ptr<UIViewController> visibleViewController() const override;
    [[nodiscard]] std::shared_ptr<UIGestureRecognizer> interactivePopGestureRecognizer() override;
    [[nodiscard]] std::shared_ptr<UINavigationBar> navigationBar();
    [[nodiscard]] bool isNavigationBarHidden() const {return _barHidden;}
    void setNavigationBarHidden(bool hidden,bool animated=false);
    [[nodiscard]] std::shared_ptr<UINavigationControllerDelegate> delegate() const {return _delegate.lock();}
    void setDelegate(std::shared_ptr<UINavigationControllerDelegate> delegate) {_delegate=delegate;}
    void show(const std::shared_ptr<UIViewController>& controller,const std::shared_ptr<UIResponder>& sender=nullptr) override;
    void loadView() override;
    void viewDidLoad() override;
    void viewWillAppear(bool animated) override;
    void viewDidAppear(bool animated) override;
    void viewWillDisappear(bool animated) override;
    void viewDidDisappear(bool animated) override;
    void viewDidLayoutSubviews() override;
    void viewSafeAreaInsetsDidChange() override;
    bool allowsActionDispatch() const override {return !_updating&&!_appearanceTransition&&UIViewController::allowsActionDispatch();}
    std::vector<std::shared_ptr<UIFocusEnvironment>> preferredFocusEnvironments() override;
private:
    struct ScrollLayout;
    std::unique_ptr<ScrollLayout> _scrollLayout;
    struct Update {std::vector<std::shared_ptr<UIViewController>> controllers;bool animated;};
    std::vector<std::shared_ptr<UIViewController>> _controllers;
    std::deque<Update> _pending;
    std::weak_ptr<UINavigationControllerDelegate> _delegate;
    std::shared_ptr<UINavigationBar> _bar;
    std::shared_ptr<UIView> _content,_displayedHost,_outgoingHost;
    std::shared_ptr<UIViewController> _displayed;
    struct InteractivePop;
    std::shared_ptr<InteractivePop> _interactive;
    std::shared_ptr<UIGestureRecognizer> _popGesture;
    bool _updating=false,_visible=false,_appearanceTransition=false,_barHidden=false;
    const std::vector<std::shared_ptr<UIViewController>>& requested() const;
    void validate(const std::vector<std::shared_ptr<UIViewController>>& controllers);
    void apply(std::vector<std::shared_ptr<UIViewController>> controllers,bool animated);
    void mount(int direction,bool animated,std::vector<std::shared_ptr<UIViewController>> removed);
    void finish();
    void layout(bool scrolling=false);
    bool canBeginInteractivePop();
    void handlePopGesture(const std::shared_ptr<UIGestureRecognizer>& gesture);
    void beginInteractivePop();
    void endInteractivePop(bool commit,NXFloat velocity,bool animated=true);
    void finishInteractivePop(bool commit);
};
}
