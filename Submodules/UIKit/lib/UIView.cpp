#include <UIView.h>

using namespace NXKit;

std::shared_ptr<CALayer> UIView::initLayer() {
    return new_shared<CALayer>();
}

UIView::UIView(NXRect frame) {
    _layer = initLayer();
    _layer->setAnchorPoint(NXPoint::zero);
    setFrame(frame);
}

void UIView::setFrame(NXRect frame) {
    if (this->frame().size != frame.size) {
//        setNeedsLayout();
    }
    _layer->setFrame(frame);
//    setNeedsUpdateSafeAreaInsets();
}

void UIView::setBounds(NXRect bounds) {
    if (this->bounds().size != bounds.size) {
//        setNeedsLayout();
//        setNeedsUpdateSafeAreaInsets();
    }
    _layer->setBounds(bounds);
}

void UIView::setCenter(NXPoint position) {
    auto frame = this->frame();
    frame.setMidX(position.x);
    frame.setMidY(position.y);
    setFrame(frame);
}

void UIView::addSubview(std::shared_ptr<UIView> view) {
    bool needToNotifyViewController = false;
//    if (!view->_parentController.expired()) {
//        auto window = this->window();
//        if (window) {
//            needToNotifyViewController = true;
//        }
//    }

//    setNeedsLayout();
    view->removeFromSuperview();

//    if (needToNotifyViewController)
//        view->_parentController.lock()->viewWillAppear(true);

    _layer->addSublayer(view->_layer);
    _subviews.push_back(view);
    view->setSuperview(this->shared_from_this());
//    view->setNeedsUpdateSafeAreaInsets();
}

void UIView::setSuperview(std::shared_ptr<UIView> superview) {
    _superview = superview;
}

void UIView::insertSubviewAt(std::shared_ptr<UIView> view, int index) {
    // TODO: Need to implement
}

void UIView::insertSubviewBelow(std::shared_ptr<UIView> view, std::shared_ptr<UIView> belowSubview) {
    auto itr = std::find(subviews().cbegin(), subviews().cend(), belowSubview);
    if (itr == subviews().cend()) { return; }

    bool needToNotifyViewController = false;
//    if (!view->_parentController.expired()) {
//        auto window = this->window();
//        if (window) {
//            needToNotifyViewController = true;
//        }
//    }

//    setNeedsLayout();
    view->removeFromSuperview();

//    if (needToNotifyViewController)
//        view->_parentController.lock()->viewWillAppear(true);

    _layer->insertSublayerBelow(view->_layer, belowSubview->layer());
    _subviews.insert(itr, view);
    view->setSuperview(this->shared_from_this());
//    view->setNeedsUpdateSafeAreaInsets();
}

void UIView::removeFromSuperview() {
    auto superview = this->_superview.lock();
    if (!superview) return;

    _layer->removeFromSuperlayer();

    // If it's mask - remove
//    if (superview->_mask.get() == this) {
//        superview->_mask = nullptr;
//    }
//    else {
        superview->_subviews.erase(std::remove(superview->_subviews.begin(), superview->_subviews.end(), shared_from_this()), superview->_subviews.end());
//    }
    this->setSuperview(nullptr);
//    superview->setNeedsLayout();
}

void UIView::setContentMode(UIViewContentMode mode) {
    switch (mode) {
        case UIViewContentMode::scaleToFill:
            _layer->setContentsGravity(CALayerContentsGravity::resize);
            break;
        case UIViewContentMode::scaleAspectFit:
            _layer->setContentsGravity(CALayerContentsGravity::resizeAspect);
            break;
        case UIViewContentMode::scaleAspectFill:
            _layer->setContentsGravity(CALayerContentsGravity::resizeAspectFill);
            break;
        case UIViewContentMode::redraw:
            _layer->setContentsGravity(CALayerContentsGravity::resize);
            break;
        case UIViewContentMode::center:
            _layer->setContentsGravity(CALayerContentsGravity::center);
            break;
        case UIViewContentMode::top:
            _layer->setContentsGravity(CALayerContentsGravity::top);
            break;
        case UIViewContentMode::bottom:
            _layer->setContentsGravity(CALayerContentsGravity::bottom);
            break;
        case UIViewContentMode::left:
            _layer->setContentsGravity(CALayerContentsGravity::left);
            break;
        case UIViewContentMode::right:
            _layer->setContentsGravity(CALayerContentsGravity::right);
            break;
        case UIViewContentMode::topLeft:
            _layer->setContentsGravity(CALayerContentsGravity::topLeft);
            break;
        case UIViewContentMode::topRight:
            _layer->setContentsGravity(CALayerContentsGravity::topRight);
            break;
        case UIViewContentMode::bottomLeft:
            _layer->setContentsGravity(CALayerContentsGravity::bottomLeft);
            break;
        case UIViewContentMode::bottomRight:
            _layer->setContentsGravity(CALayerContentsGravity::bottomRight);
            break;
    }
}
