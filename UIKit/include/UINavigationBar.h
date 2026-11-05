#pragma once
#include <UIView.h>
#include <UINavigationItem.h>
namespace NXKit {
class UINavigationBar;
namespace Internal { struct NavigationBarState; class NavigationBarAccess; }
class UINavigationBarDelegate {
public:
    virtual ~UINavigationBarDelegate() = default;
    virtual bool navigationBarShouldPushItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&) {return true;}
    virtual void navigationBarDidPushItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&) {}
    virtual bool navigationBarShouldPopItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&) {return true;}
    virtual void navigationBarDidPopItem(const std::shared_ptr<UINavigationBar>&,const std::shared_ptr<UINavigationItem>&) {}
};
/** Apple-style navigation bar with shared glass items and collapsible large titles.
    Appearance objects and search integration remain unimplemented. */
class UINavigationBar : public UIView {
public:
    static std::shared_ptr<UIView> init() {return new_shared<UINavigationBar>();}
    explicit UINavigationBar(NXRect frame = {});
    ~UINavigationBar() override;
    [[nodiscard]] std::vector<std::shared_ptr<UINavigationItem>> items() const;
    void setItems(std::vector<std::shared_ptr<UINavigationItem>> items,bool animated=false);
    [[nodiscard]] std::shared_ptr<UINavigationItem> topItem() const;
    [[nodiscard]] std::shared_ptr<UINavigationItem> backItem() const;
    void pushNavigationItem(const std::shared_ptr<UINavigationItem>& item,bool animated);
    std::shared_ptr<UINavigationItem> popNavigationItem(bool animated);
    [[nodiscard]] std::shared_ptr<UINavigationBarDelegate> delegate() const;
    void setDelegate(std::shared_ptr<UINavigationBarDelegate> delegate);
    [[nodiscard]] bool prefersLargeTitles() const;
    void setPrefersLargeTitles(bool prefers);
    bool applyXMLAttribute(const std::string& name,const std::string& value) override;
    void layoutSubviews() override;
    void tintColorDidChange() override;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) override;
    NXSize sizeThatFits(NXSize size) override;
    bool point(NXPoint point,UIEvent* event) override;
    std::vector<std::shared_ptr<UIFocusEnvironment>> preferredFocusEnvironments() override;
private:
    friend class Internal::NavigationBarAccess;
    std::unique_ptr<Internal::NavigationBarState> _state;
    void install(std::vector<std::shared_ptr<UINavigationItem>> items,bool animated,int direction);
    void refresh(bool animated=false,int direction=0);
    void arrange();
    void activate(const std::shared_ptr<UIBarButtonItem>& item,bool back);
};
}
