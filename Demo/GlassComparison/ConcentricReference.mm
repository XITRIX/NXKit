// Native public UIKit oracle for NXKit's portable concentric-corner API.
#import <UIKit/UIKit.h>
#include "../../UIKitCore/lib/platforms/apple/ios/IOSWindowGeometry.hpp"
static NSArray *radii(UIView *v) {
    return @[@([v effectiveRadiusForCorner:UIRectCornerTopLeft]),@([v effectiveRadiusForCorner:UIRectCornerTopRight]),
             @([v effectiveRadiusForCorner:UIRectCornerBottomLeft]),@([v effectiveRadiusForCorner:UIRectCornerBottomRight])];
}
@interface ConcentricController : UIViewController
@end
@implementation ConcentricController
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    NSMutableDictionary *result=[NSMutableDictionary dictionary];
    NXKit::Internal::IOSWindowGeometry geometry((__bridge void*)self.view.window);
    const auto frame=geometry.sample();
    NSMutableArray *bridge=[NSMutableArray array];
    for(const auto radius:frame.cornerRadii)[bridge addObject:@(radius)];
    result[@"productionBridge"]=bridge;
    if (@available(iOS 26.0,*)) {
    UIView *probe=[[UIView alloc] initWithFrame:self.view.bounds];
    probe.cornerConfiguration=[UICornerConfiguration configurationWithRadius:UICornerRadius.containerConcentricRadius];
    [self.view addSubview:probe]; [self.view layoutIfNeeded];
    result[@"window"]=radii(probe);
    UIView *parent=[[UIView alloc] initWithFrame:CGRectMake(0,100,300,400)];
    parent.cornerConfiguration=[UICornerConfiguration configurationWithRadius:[UICornerRadius fixedRadius:60]];
    [self.view addSubview:parent];
    UIView *child=[[UIView alloc] initWithFrame:CGRectMake(10,10,260,360)];
    child.cornerConfiguration=[UICornerConfiguration configurationWithRadius:UICornerRadius.containerConcentricRadius];
    [parent addSubview:child]; [parent layoutIfNeeded];result[@"unequalInsets"]=radii(child);
    child.frame=CGRectMake(10,20,260,340);[parent layoutIfNeeded];result[@"differentAxes"]=radii(child);
    UIView *wrapper=[[UIView alloc] initWithFrame:parent.bounds];[parent addSubview:wrapper];[wrapper addSubview:child];
    [parent layoutIfNeeded];result[@"squareWrapper"]=radii(child);
    wrapper.layer.cornerRadius=40;[parent layoutIfNeeded];result[@"layerWrapper"]=radii(child);result[@"layerWrapperEffective"]=radii(wrapper);
    wrapper.cornerConfiguration=[UICornerConfiguration configurationWithRadius:[UICornerRadius fixedRadius:0]];
    [parent layoutIfNeeded];result[@"explicitSquareWrapper"]=radii(child);
    wrapper.cornerConfiguration=[UICornerConfiguration configurationWithRadius:[UICornerRadius fixedRadius:40]];
    [parent layoutIfNeeded];result[@"configuredWrapper"]=radii(child);
    [parent addSubview:child];
    child.transform=CGAffineTransformMakeTranslation(20,30);[parent layoutIfNeeded];result[@"translated"]=radii(child);
    child.transform=CGAffineTransformIdentity;
    child.frame=CGRectMake(-10,0,260,340);[parent layoutIfNeeded];result[@"outside"]=radii(child);
    [child removeFromSuperview];[child layoutIfNeeded];result[@"detached"]=radii(child);
    child.layer.cornerRadius=17;[child layoutIfNeeded];result[@"layerAfterConfiguration"]=radii(child);
    [parent addSubview:child];child.frame=CGRectMake(10,20,260,340);
    child.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:UICornerRadius.containerConcentricRadius];
    [parent layoutIfNeeded];result[@"uniform"]=radii(child);
    child.cornerConfiguration=[UICornerConfiguration configurationWithTopLeftRadius:nil topRightRadius:[UICornerRadius fixedRadius:12] bottomLeftRadius:nil bottomRightRadius:nil];
    [parent layoutIfNeeded];result[@"nilCorners"]=radii(child);
    child.cornerConfiguration=[UICornerConfiguration configurationWithRadius:[UICornerRadius fixedRadius:900]];
    [parent layoutIfNeeded];result[@"largeFixed"]=radii(child);
    }
    result[@"os"]=UIDevice.currentDevice.systemVersion;
    NSURL *dir=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    [[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil] writeToURL:[dir URLByAppendingPathComponent:@"concentric.json"] atomically:YES];
}
@end
@interface ConcentricScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation ConcentricScene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
    self.window.rootViewController=[ConcentricController new];[self.window makeKeyAndVisible];
}
@end
@interface ConcentricApp : UIResponder <UIApplicationDelegate>
@end
@implementation ConcentricApp
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    UISceneConfiguration *config=[[UISceneConfiguration alloc] initWithName:@"Concentric" sessionRole:session.role];
    config.delegateClass=ConcentricScene.class;return config;
}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(ConcentricApp.class));}}
