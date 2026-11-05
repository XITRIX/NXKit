#include <UINavigationItem.h>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
using namespace NXKit;
namespace {
void validate(const auto& items,const auto& other) {
    std::unordered_set<const UIBarButtonItem*> unique;
    for(const auto& item:items)if(!item||!unique.insert(item.get()).second
        ||std::find(other.begin(),other.end(),item)!=other.end())throw std::invalid_argument("Navigation buttons must be non-null and distinct on each bar");
}
}
void UINavigationItem::changed(bool animated) {
    std::erase_if(_observers,[](const auto& observer){return observer.owner.expired();});
    const auto observers=_observers;for(const auto& observer:observers)if(const auto owner=observer.owner.lock())observer.changed(animated);
}
void UINavigationItem::setTitle(std::string title) {_explicitTitle=true;if(_title==title)return;_title=std::move(title);changed();}
void UINavigationItem::setTitleView(std::shared_ptr<UIView> view) {_titleView=std::move(view);changed();}
void UINavigationItem::setLeftBarButtonItems(std::vector<std::shared_ptr<UIBarButtonItem>> items,bool animated) {validate(items,_right);_left=std::move(items);changed(animated);}
void UINavigationItem::setRightBarButtonItems(std::vector<std::shared_ptr<UIBarButtonItem>> items,bool animated) {validate(items,_left);_right=std::move(items);changed(animated);}
void UINavigationItem::setLeftBarButtonItem(std::shared_ptr<UIBarButtonItem> item,bool animated) {setLeftBarButtonItems(item?std::vector{item}:std::vector<std::shared_ptr<UIBarButtonItem>>{},animated);}
void UINavigationItem::setRightBarButtonItem(std::shared_ptr<UIBarButtonItem> item,bool animated) {setRightBarButtonItems(item?std::vector{item}:std::vector<std::shared_ptr<UIBarButtonItem>>{},animated);}
void UINavigationItem::setHidesBackButton(bool hidden,bool animated) {_hidesBack=hidden;changed(animated);}
void UINavigationItem::setLeftItemsSupplementBackButton(bool supplement) {_supplementsBack=supplement;changed();}

void UINavigationItem::setLargeTitleDisplayMode(UINavigationItemLargeTitleDisplayMode mode) {
    switch(mode) {
    case UINavigationItemLargeTitleDisplayMode::automatic:
    case UINavigationItemLargeTitleDisplayMode::always:
    case UINavigationItemLargeTitleDisplayMode::never:break;
    default:throw std::invalid_argument("Unknown large-title display mode");
    }
    if(_largeTitleDisplayMode==mode)return;
    _largeTitleDisplayMode=mode;changed();
}
