#include "GlassStressView.hpp"
#include <algorithm>
#include <string>

using namespace NXKit;
namespace {
constexpr NXFloat cardWidth = 176, cardHeight = 164, stride = 194, railHeight = 204, sectionHeight = 248;
std::shared_ptr<UILabel> label(const std::string& text, NXFloat size, UIColor color = UIColor::white) {
    auto result = new_shared<UILabel>();
    result->setText(text); result->setFontSize(size); result->setTextColor(color);
    result->setUserInteractionEnabled(false);
    return result;
}
UIColor color(int index) {
    constexpr unsigned char palette[][3] = {{35,86,180},{172,47,109},{0,128,137},{176,88,29},{92,57,178},{33,123,87}};
    const auto& rgb = palette[index % 6]; return UIColor(rgb[0],rgb[1],rgb[2]);
}
}

GlassStressView::GlassStressView() {
    setBackgroundColor(color(0));
    _vertical = new_shared<UIScrollView>();
    _vertical->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
    _vertical->setBounceVertically(true);
    addSubview(_vertical);
    _content = new_shared<UIView>(); _vertical->addSubview(_content);
    const auto weak = weak_from_base<GlassStressView>();
    for (int row = 0; row < railCount; ++row) {
        auto heading = label("RAIL " + std::to_string(row+1) + "  /  DRAG SIDEWAYS",16);
        _content->addSubview(heading); _headings.push_back(heading);
        auto rail = new_shared<UIScrollView>();
        rail->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);
        rail->setBounceHorizontally(true); rail->setShowsVerticalScrollIndicator(false);
        _content->addSubview(rail); _rails.push_back(rail);
        auto strip = new_shared<UIView>(NXRect(0,0,cardsPerRail*stride+18,railHeight));
        rail->addSubview(strip); rail->setContentSize(strip->bounds().size);
        for (int column = 0; column < cardsPerRail; ++column) {
            auto card = new_shared<UIView>(NXRect(18+column*stride,16,cardWidth,cardHeight));
            // Overflow is deliberately clipped at the card boundary. The
            // inner glass remains live over these moving, high-contrast tiles.
            card->setClipsToBounds(true); card->layer()->setCornerRadius(24);
            card->setBackgroundColor(color(row+column)); strip->addSubview(card);
            for (int tile = 0; tile < 12; ++tile) {
                auto block = new_shared<UIView>(NXRect((tile%4)*48-8,(tile/4)*58-10,34,40));
                block->setBackgroundColor(color(row+column+tile+2));
                block->layer()->setCornerRadius(9); card->addSubview(block);
            }
            UIGlassEffect effect(column%3 == 1 ? UIGlassEffectStyle::clear : UIGlassEffectStyle::regular);
            effect.setInteractive(true);
            auto glass = new_shared<BackdropEffectView>(effect);
            glass->setFrame({10,38,cardWidth-20,112}); glass->layer()->setCornerRadius(22); card->addSubview(glass);
            auto title = label("Glass " + std::to_string(row*cardsPerRail+column+1),22);
            title->setFrame({14,9,132,32}); glass->addSubview(title);
            auto detail = label(column%3 == 1 ? "Clear material" : "Regular material",13);
            detail->setFrame({14,41,132,24}); glass->addSubview(detail);
            auto action = new_shared<UIControl>(); action->setFrame({0,0,cardWidth-20,112}); glass->addSubview(action);
            action->primaryAction = UIAction("Select glass",[weak,title] {
                if (auto self = weak.lock()) {
                    ++self->_activations; title->setText("Selected " + std::to_string(self->_activations));
                    self->_status->setText(std::to_string(self->_activations)+" selections / 96 live glass cards");
                }
            });
        }
    }
    UIGlassEffect toolbarEffect; toolbarEffect.setInteractive(true);
    _toolbar = new_shared<BackdropEffectView>(toolbarEffect); _toolbar->layer()->setCornerRadius(24); addSubview(_toolbar);
    _toolbar->addSubview(label("LIQUID GLASS / STRESS",19));
    _status = label("96 live glass cards / drag in both axes",13); _toolbar->addSubview(_status);
    _backdropButton = new_shared<UIControl>(); _toolbar->addSubview(_backdropButton);
    _backdropButton->primaryAction = UIAction("Change backdrop",[weak] { if (auto self=weak.lock()) self->changeBackdrop(); });
    _backdropButton->addSubview(label("Backdrop ↻",15));
}

void GlassStressView::changeBackdrop() {
    setBackgroundColor(color(++_backdrop));
}
void GlassStressView::resetOffsets() {
    _vertical->setContentOffset({},false);
    for (const auto& rail : _rails) rail->setContentOffset({},false);
}
void GlassStressView::layoutSubviews() {
    UIView::layoutSubviews();
    const auto size = bounds().size; const auto safe = safeAreaInsets();
    _vertical->setFrame({0,0,size.width,size.height});
    const NXFloat top = safe.top+112;
    _content->setFrame({0,0,size.width,top+railCount*sectionHeight+safe.bottom+24});
    _vertical->setContentSize(_content->bounds().size);
    _vertical->setVerticalScrollIndicatorInsets({top,0,safe.bottom,0});
    for (int row = 0; row < railCount; ++row) {
        _headings[row]->setFrame({20,top+row*sectionHeight,size.width-40,32});
        _rails[row]->setFrame({0,top+row*sectionHeight+34,size.width,railHeight});
    }
    _toolbar->setFrame({12+safe.left,safe.top+8,std::max(0.0f,size.width-safe.left-safe.right-24),88});
    _toolbar->subviews()[0]->setFrame({16,8,_toolbar->bounds().width()-24,28});
    _status->setFrame({16,38,std::max(0.0f,_toolbar->bounds().width()-24),20});
    _backdropButton->setFrame({16,58,std::max(0.0f,_toolbar->bounds().width()-32),28});
    _backdropButton->subviews().front()->setFrame(_backdropButton->bounds());
}
