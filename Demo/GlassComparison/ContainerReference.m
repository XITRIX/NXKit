// Native UIKit reference: no NXKit implementation is linked into this app.
#import <UIKit/UIKit.h>
@interface ContainerController : UIViewController
@property(nonatomic, strong) UIView *scene;
@property(nonatomic, strong) NSMutableArray<UIVisualEffectView *> *items;
@property(nonatomic, strong) NSTimer *poller;
@property(nonatomic, copy) NSDictionary *request;
@end
@implementation ContainerController
- (BOOL)prefersStatusBarHidden { return YES; }
- (void)viewDidLoad {
    [super viewDidLoad];
    self.overrideUserInterfaceStyle = UIUserInterfaceStyleLight;
    self.view.backgroundColor = UIColor.blackColor;
    self.scene = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 720, 760)];
    self.scene.backgroundColor = [UIColor colorWithWhite:0.12 alpha:1];
    [self.view addSubview:self.scene];
    self.items = [NSMutableArray new];
    CGFloat spacings[] = {0, 10, 20, 40}, gaps[] = {4, 12, 24, 44};
    for (int row = 0; row < 4; ++row) for (int column = 0; column < 4; ++column) {
        UIGlassContainerEffect *effect = [UIGlassContainerEffect new];
        effect.spacing = spacings[column];
        UIVisualEffectView *container = [[UIVisualEffectView alloc] initWithEffect:effect];
        container.frame = CGRectMake(column * 180, row * 150, 180, 150);
        [self.scene addSubview:container];
        CGFloat gap = gaps[row], x = (180 - 120 - gap) / 2;
        for (int item = 0; item < 2; ++item) {
            UIGlassEffect *glass = [UIGlassEffect effectWithStyle:UIGlassEffectStyleRegular];
            glass.tintColor = UIColor.systemRedColor;
            UIVisualEffectView *view = [[UIVisualEffectView alloc] initWithEffect:glass];
            view.frame = CGRectMake(x + item * (60 + gap), 45, 60, 60);
            view.layer.cornerRadius = 30;
            view.layer.cornerCurve = kCACornerCurveContinuous;
            [container.contentView addSubview:view];
            [self.items addObject:view];
        }
    }
    // A larger overlapping pair and an isolated control have identical tint.
    for (int col = 0; col < 2; ++col) {
        UIGlassContainerEffect *effect = [UIGlassContainerEffect new];
        effect.spacing = 20;
        UIVisualEffectView *container = [[UIVisualEffectView alloc] initWithEffect:effect];
        container.frame = CGRectMake(col * 360, 610, 360, 140);
        [self.scene addSubview:container];
        for (int i = 0; i < 2; ++i) {
            UIGlassEffect *glass = [UIGlassEffect effectWithStyle:UIGlassEffectStyleRegular];
            glass.tintColor = UIColor.systemRedColor;
            UIVisualEffectView *view = [[UIVisualEffectView alloc] initWithEffect:glass];
            view.frame = CGRectMake(30 + i * (col ? 170 : 80), 20, 120, 100);
            view.layer.cornerRadius = 35;
            view.layer.cornerCurve = kCACornerCurveContinuous;
            [container.contentView addSubview:view];
            [self.items addObject:view];
        }
    }
    __weak ContainerController *weakSelf = self;
    self.poller = [NSTimer scheduledTimerWithTimeInterval:0.1 repeats:YES block:^(NSTimer *timer) { [weakSelf poll]; }];
}
- (void)poll {
    NSURL *directory = [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSData *data = [NSData dataWithContentsOfURL:[directory URLByAppendingPathComponent:@"request.json"]];
    if (!data) return;
    NSDictionary *request = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if (!request || [request isEqual:self.request]) return;
    self.request = request;
    self.overrideUserInterfaceStyle = [request[@"dark"] boolValue] ? UIUserInterfaceStyleDark : UIUserInterfaceStyleLight;
    NSArray<NSNumber *> *gaps = @[@4,@12,@24,@44];
    CGFloat factor = request[@"sizeScale"] ? [request[@"sizeScale"] doubleValue] : 1;
    CGFloat offset = [request[@"gapOffset"] doubleValue];
    [UIView performWithoutAnimation:^{
        for (NSInteger index=0; index<32; ++index) {
            NSInteger row=index/8, item=index%2;
            CGFloat size=60*factor, gap=gaps[row].doubleValue+offset, x=(180-size*2-gap)/2;
            UIVisualEffectView *view=self.items[index];
            view.frame=CGRectMake(x+item*(size+gap),75-size/2,size,size);
            view.layer.cornerRadius=size/2;
        }
    }];
    [self.view setNeedsLayout];
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.scene.center = CGPointMake(CGRectGetMidX(self.view.bounds), CGRectGetMidY(self.view.bounds));
    NSDictionary *metadata = @{@"x": @(self.scene.frame.origin.x), @"y": @(self.scene.frame.origin.y),
        @"scale": @(self.view.window.screen.scale), @"os": UIDevice.currentDevice.systemVersion,
        @"defaultSpacing": @([UIGlassContainerEffect new].spacing),
        @"request": self.request ?: @{},
        @"spacings": @[@0,@10,@20,@40], @"gaps": @[@4,@12,@24,@44]};
    NSURL *directory = [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    [[NSJSONSerialization dataWithJSONObject:metadata options:NSJSONWritingPrettyPrinted error:nil]
        writeToURL:[directory URLByAppendingPathComponent:@"container.json"] atomically:YES];
}
@end
@interface ContainerScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation ContainerScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [ContainerController new];
    [self.window makeKeyAndVisible];
}
@end
@interface ContainerApp : UIResponder <UIApplicationDelegate>
@end
@implementation ContainerApp
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *config = [[UISceneConfiguration alloc] initWithName:@"Container reference" sessionRole:session.role];
    config.delegateClass = ContainerScene.class;
    return config;
}
@end
int main(int argc, char **argv) {
    @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(ContainerApp.class)); }
}
