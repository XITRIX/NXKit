#include <UIBarButtonItem.h>
#include <cmath>
#include <stdexcept>
using namespace NXKit;
UIBarButtonItem::UIBarButtonItem(std::string title,UIBarButtonItemStyle style,std::optional<UIAction> action)
    :_action(std::move(action)) {setTitle(std::move(title));setStyle(style);}
UIBarButtonItem::UIBarButtonItem(std::shared_ptr<UIImage> image,UIBarButtonItemStyle style,std::optional<UIAction> action)
    :_action(std::move(action)) {setImage(std::move(image));setStyle(style);}
UIBarButtonItem::UIBarButtonItem(UIBarButtonSystemItem item,std::optional<UIAction> action)
    :_systemItem(item),_action(std::move(action)) {
    if (int(item)<0 || int(item)>int(UIBarButtonSystemItem::redo)) throw std::invalid_argument("Unknown system bar button");
    if(item==UIBarButtonSystemItem::done)_style=UIBarButtonItemStyle::prominent;
}
UIBarButtonItem::UIBarButtonItem(UIAction action) {setPrimaryAction(std::move(action));}
UIBarButtonItem::UIBarButtonItem(std::shared_ptr<UIView> view):_customView(std::move(view)) {}
void UIBarButtonItem::setStyle(UIBarButtonItemStyle style) {
    if(style!=UIBarButtonItemStyle::plain&&style!=UIBarButtonItemStyle::prominent)throw std::invalid_argument("Unsupported bar button style");
    if(_style==style)return;_style=style;notifyChanged();
}
void UIBarButtonItem::setWidth(NXFloat width) {
    if(!std::isfinite(width)||width<0)throw std::invalid_argument("Bar button width must be finite and nonnegative");
    if(_width==width)return;_width=width;notifyChanged();
}
void UIBarButtonItem::setPrimaryAction(std::optional<UIAction> action) {
    _action=std::move(action);if(_action&&!_systemItem)setTitle(_action->title());notifyChanged();
}
void UIBarButtonItem::setCustomView(std::shared_ptr<UIView> view) {_customView=std::move(view);notifyChanged();}
void UIBarButtonItem::setTintColor(std::optional<UIColor> color) {_tintColor=color;notifyChanged();}
void UIBarButtonItem::setHidesSharedBackground(bool hidden) {if(_hidesSharedBackground==hidden)return;_hidesSharedBackground=hidden;notifyChanged();}
void UIBarButtonItem::setSharesBackground(bool shares) {if(_sharesBackground==shares)return;_sharesBackground=shares;notifyChanged();}
void UIBarButtonItem::setIdentifier(std::optional<std::string> identifier) {if(_identifier==identifier)return;_identifier=std::move(identifier);notifyChanged();}
std::shared_ptr<UIBarButtonItem> UIBarButtonItem::fixedSpaceItemOfWidth(NXFloat width) {
    auto item=std::make_shared<UIBarButtonItem>(UIBarButtonSystemItem::fixedSpace);item->setWidth(width);return item;
}
