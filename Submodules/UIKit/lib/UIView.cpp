#include <UIView.h>

using namespace NXKit;

UIView::UIView() {
    _layer = new_shared<CALayer>();
    _layer->setAnchorPoint(NXPoint::zero);
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
