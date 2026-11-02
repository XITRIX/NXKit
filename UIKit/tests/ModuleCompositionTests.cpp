#include <UIKit.h>
#include <NXKit.h>
#include <UIApplication.h>
#include <SkiaCtx.h>
#include <UINib.h>
#include <iostream>
#include <string>

using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(
    UIApplication*, std::map<std::string, std::any>
) { return true; }
}
namespace {
int failures = 0;
void expect(bool value, const std::string& message) {
    if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
class TestSkiaContext final : public SkiaCtx {
public:
    UIUserInterfaceStyle themeMode = UIUserInterfaceStyle::light;

    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    NXSize getSize() override { return { 1280, 720 }; }
    void swapBuffers() override {}
    UIUserInterfaceStyle getThemeMode() override { return themeMode; }
    bool platformRunLoop(std::function<bool()> loop) override { return loop(); }

protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};

class RecordingTraitView final : public UIView {
public:
    int traitChangeCount = 0;

    void traitCollectionDidChange(
        std::shared_ptr<UITraitCollection> previousTraitCollection
    ) override {
        UIView::traitCollectionDidChange(std::move(previousTraitCollection));
        ++traitChangeCount;
    }
};

std::vector<NXFloat> glassTint(const std::shared_ptr<BackdropEffectView>& view) {
    const auto tint = view->effect().uniform("tint");
    if (!tint || tint->size() != 4) {
        expect(false, "regular glass exposes a four-component tint uniform");
        return std::vector<NXFloat>(4);
    }
    return *tint;
}

NXFloat glassScalar(
    const std::shared_ptr<BackdropEffectView>& view,
    const std::string& name
) {
    const auto value = view->effect().uniform(name);
    if (!value || value->size() != 1) {
        expect(false, "regular glass exposes scalar uniform '" + name + "'");
        return 0;
    }
    return value->front();
}

}
int main() {
    auto skiaContext = std::make_shared<TestSkiaContext>();
    SkiaCtx::_main = skiaContext;
    UIApplication::shared = new_shared<UIApplication>();
    UIApplication::shared->delegate = std::make_shared<UIApplicationDelegate>();
    auto first = new_shared<UIViewController>();
    auto second = new_shared<UIViewController>();
    first->setTitle("First"); second->setTitle("Second");
    auto tabs = new_shared<NXTabBarController>(NXTabBarController::ViewControllerSection{first,second});
    auto focusNavigationController = new_shared<NXNavigationController>(tabs);
    expect(first->tabBarController() == tabs && first->navigationController() == focusNavigationController
           && !tabs->viewIsLoaded() && !first->viewIsLoaded(),
           "configured NX content discovers both capabilities before any view is loaded");
    auto temporary = new_shared<UIViewController>();
    auto unloadedNavigation = first->navigationController();
    if (unloadedNavigation) {
        unloadedNavigation->pushViewController(temporary, false);
        expect(temporary->navigationController() == focusNavigationController
               && !focusNavigationController->viewIsLoaded() && !temporary->viewIsLoaded(),
               "an unloaded push establishes containment without loading views");
        unloadedNavigation->popViewController(false);
        expect(!temporary->navigationController(), "an unloaded pop clears containment");
    }
    tabs->setViewControllers({temporary});
    expect(!first->tabBarController() && temporary->tabBarController() == tabs,
           "unloaded tab replacement detaches old children and adopts new children");
    tabs->setViewControllers({first, second});
    expect(!temporary->tabBarController() && first->tabBarController() == tabs,
           "unloaded tab replacement restores actual associations");
    auto focusWindow = new_shared<UIWindow>();
    focusWindow->setFrame({0,0,1280,720});
    UIApplication::shared->delegate->window = focusWindow;
    focusWindow->setRootViewController(focusNavigationController);
    focusWindow->makeKeyAndVisible();
    expect(applicationRunLoop(), "mixed-kit hierarchy lays out");
    expect(first->tabBarController() == tabs && first->navigationController() == focusNavigationController,
           "NX containers expose shared capabilities through actual containment");
    auto tabInterface = first->tabBarController();
    if (tabInterface) {
        expect(tabInterface->setSelectedIndex(1) && tabInterface->selectedViewController() == second,
               "selection through shared interface updates NX container");
    }
    auto appleButton = new_shared<UIButton>();
    auto nxControl = new_shared<NXControl>();
    second->view()->addSubview(appleButton);
    second->view()->addSubview(nxControl);
    expect(appleButton->superview().lock() == second->view()
           && nxControl->superview().lock() == second->view(), "Apple and Nintendo controls coexist in one screen");
    appleButton->willGainFocus();
    expect(appleButton->transform() == NXAffineTransform::scale(1.06f)
           && appleButton->layer()->shadowRadius() == 18, "UIButton retains its own focus style");
    int presses = 0;
    appleButton->primaryAction = UIAction("Mix", [&] { ++presses; });
    appleButton->performPrimaryAction();
    expect(presses == 1, "Apple control dispatches actions within NX containment");
    registerUIKitViews(); registerUIKitViews();
    UINib::registerXIB("UIView", UIView::init);
    auto inflated = UINib::fromString("<UIView><UIButton text=\"Mixed kit\" style=\"filled\"/></UIView>")->instantiate();
    expect(inflated && inflated->subviews().size() == 1
           && std::dynamic_pointer_cast<UIButton>(inflated->subviews().front()),
           "UIKit explicitly registers its XML components and style parser");
    auto presentationChrome = new_shared<RecordingTraitView>();
    focusWindow->addSubview(presentationChrome);
    presentationChrome->traitChangeCount = 0;

    auto presentedRoot = new_shared<UIView>();
    auto presentedGlass = new_shared<BackdropEffectView>(
        UIGlassEffect(UIGlassEffect::Style::regular)
    );
    presentedGlass->setFrame(NXRect(0, 0, 520, 320));
    presentedRoot->addSubview(presentedGlass);
    auto presentedController = new_shared<UIViewController>();
    presentedController->setView(presentedRoot);
    presentedController->setModalPresentationStyle(
        UIModalPresentationStyle::overFullScreen
    );
    focusNavigationController->present(presentedController, false);

    const auto initialLightTint = glassTint(presentedGlass);
    const auto initialLightLuminosity = glassScalar(
        presentedGlass,
        "luminosity"
    );
    expect(
        initialLightTint[0] > 0.99f
            && initialLightTint[1] > 0.99f
            && initialLightTint[2] > 0.99f
            && initialLightTint[3] > 0.50f && initialLightTint[3] < 0.60f
            && initialLightLuminosity == 0,
        "presented regular glass starts with the translucent light material"
    );

    skiaContext->themeMode = UIUserInterfaceStyle::dark;
    expect(applicationRunLoop(), "the dark trait update completes");
    expect(
        presentationChrome->traitChangeCount == 1
            && presentationChrome->traitCollection()
            && presentationChrome->traitCollection()->userInterfaceStyle()
                == UIUserInterfaceStyle::dark,
        "window-owned presentation chrome receives the dark trait update"
    );
    const auto darkTint = glassTint(presentedGlass);
    const auto darkLuminosity = glassScalar(presentedGlass, "luminosity");
    expect(
        darkTint[0] > 0.99f
            && darkTint[1] > 0.99f
            && darkTint[2] > 0.99f
            && darkTint[3] < initialLightTint[3]
            && darkLuminosity < 0.001f,
        "an active presented glass view reinstalls its dark material"
    );

    skiaContext->themeMode = UIUserInterfaceStyle::light;
    expect(applicationRunLoop(), "the restored light trait update completes");
    const auto restoredLightTint = glassTint(presentedGlass);
    expect(
        restoredLightTint == initialLightTint
            && glassScalar(presentedGlass, "luminosity")
                == initialLightLuminosity,
        "an active presented glass view restores its light material"
    );
    presentedController->dismiss(false);
    auto pushed = new_shared<UIViewController>();
    auto navigation = second->navigationController();
    if (navigation) {
        navigation->pushViewController(pushed, false);
        expect(navigation->topViewController() == pushed, "push uses shared navigation interface");
        expect(navigation->popViewController(false) == pushed && !pushed->navigationController(),
               "pop detaches shared navigation association");
    }
    tabInterface.reset(); navigation.reset();
    focusWindow->setRootViewController(nullptr);
    UIApplication::shared.reset(); SkiaCtx::_main.reset();
    if (!failures) std::cout << "UIKit/NXKit composition tests passed\n";
    return failures ? 1 : 0;
}
