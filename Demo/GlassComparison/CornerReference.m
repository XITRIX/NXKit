// A solid-color Core Animation reference; independent of NXKit and glass tone.
#import <UIKit/UIKit.h>

@interface CornerController : UIViewController
@property(nonatomic, strong) UIView *scene;
@end
@implementation CornerController
- (BOOL)prefersStatusBarHidden { return YES; }
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.blackColor;
    self.scene = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 720, 760)];
    [self.view addSubview:self.scene];
    CGFloat ys[] = {20, 140, 260, 440, 560};
    CGFloat heights[] = {100, 100, 160, 100, 160};
    CGFloat radii[] = {12, 24, 52, 50, 80};
    for (int row = 0; row < 5; ++row) for (int column = 0; column < 2; ++column) {
        UIView *shape = [[UIView alloc] initWithFrame:CGRectMake(20 + column * 360, ys[row], 300, heights[row])];
        shape.backgroundColor = UIColor.whiteColor;
        shape.layer.cornerRadius = radii[row];
        shape.layer.cornerCurve = column ? kCACornerCurveContinuous : kCACornerCurveCircular;
        [self.scene addSubview:shape];
    }
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.scene.center = CGPointMake(CGRectGetMidX(self.view.bounds), CGRectGetMidY(self.view.bounds));
    NSDictionary *metadata = @{@"x": @(self.scene.frame.origin.x), @"y": @(self.scene.frame.origin.y),
        @"scale": @(self.view.window.screen.scale), @"os": UIDevice.currentDevice.systemVersion,
        @"radii": @[@12,@24,@52,@50,@80], @"ys": @[@20,@140,@260,@440,@560],
        @"heights": @[@100,@100,@160,@100,@160]};
    NSURL *directory = [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    [[NSJSONSerialization dataWithJSONObject:metadata options:NSJSONWritingPrettyPrinted error:nil]
        writeToURL:[directory URLByAppendingPathComponent:@"corners.json"] atomically:YES];
}
@end
@interface CornerScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation CornerScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [CornerController new];
    [self.window makeKeyAndVisible];
}
@end
@interface CornerApp : UIResponder <UIApplicationDelegate>
@end
@implementation CornerApp
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *config = [[UISceneConfiguration alloc] initWithName:@"Corner reference" sessionRole:session.role];
    config.delegateClass = CornerScene.class;
    return config;
}
@end
int main(int argc, char **argv) {
    @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(CornerApp.class)); }
}
