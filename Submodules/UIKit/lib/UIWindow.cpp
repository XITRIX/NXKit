#include <UIWindow.h>
#include <UIApplication.h>

using namespace NXKit;

void UIWindow::makeKeyAndVisible() {
//    self.safeAreaInsets = UIWindow.getSafeAreaInsets()
    auto window = std::static_pointer_cast<UIWindow>(shared_from_this());
    // window->setBounds(UIRenderer::main()->bounds());
    UIApplication::shared->keyWindow = window;

    // auto viewController = _rootViewController;
    // if (viewController) {
    //     viewController->loadViewIfNeeded();
    //     viewController->view()->setFrame(this->bounds());
    //     viewController->viewWillAppear(false);
    //     addSubview(viewController->view());
    //     viewController->viewDidAppear(false);
    //     updateFocus();
    // }
}