// Independent UIKit framebuffer fixture. All material inputs are public APIs.
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
static const CGRect Frames[] = {{40,64,64,64},{160,48,96,96},{300,24,144,144},{530,62,132,56},{240,230,240,160}};
static const CGFloat Radii[] = {32,48,72,28,36};
@interface ShadowController : UIViewController
@property UIView *stage;
@property UIVisualEffectView *group;
@property UIImageView *background;
@property NSMutableArray<UIVisualEffectView *> *glasses;
@property NSTimer *timer;
@property NSString *pending;
@end
@implementation ShadowController
- (BOOL)prefersStatusBarHidden {
  return YES;
}
- (NSURL *)documents {
  return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory
                                              inDomains:NSUserDomainMask]
      .firstObject;
}
- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = UIColor.blackColor;
  self.stage = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 720, 440)];
  self.stage.clipsToBounds = YES;
  [self.view addSubview:self.stage];
  self.background = [[UIImageView alloc] initWithFrame:self.stage.bounds];
  self.background.layer.magnificationFilter = kCAFilterNearest;
  self.background.layer.minificationFilter = kCAFilterNearest;
  [self.stage addSubview:self.background];
  self.group = [[UIVisualEffectView alloc] initWithEffect:nil];
  self.group.frame=self.stage.bounds; [self.stage addSubview:self.group];
  self.glasses = [NSMutableArray new];
  for (int i = 0; i < 5; i++) {
    UIVisualEffectView *g = [[UIVisualEffectView alloc]
        initWithEffect:[UIGlassEffect
                           effectWithStyle:UIGlassEffectStyleRegular]];
    g.frame = Frames[i];
    g.cornerConfiguration = [UICornerConfiguration
        configurationWithUniformRadius:[UICornerRadius fixedRadius:Radii[i]]];
    [self.group.contentView addSubview:g];
    [self.glasses addObject:g];
  }
  [NSFileManager.defaultManager
      removeItemAtURL:[[self documents]
                          URLByAppendingPathComponent:@"request.json"]
                error:nil];
  __weak ShadowController *weak = self;
  self.timer = [NSTimer scheduledTimerWithTimeInterval:.05
                                               repeats:YES
                                                 block:^(NSTimer *t) {
                                                   [weak poll];
                                                 }];
}
- (void)viewDidLayoutSubviews {
  [super viewDidLayoutSubviews];
  self.stage.frame =
      CGRectMake(floor((self.view.bounds.size.width - 720) / 2),
                 floor((self.view.bounds.size.height - 440) / 2), 720, 440);
}
- (void)poll {
  NSData *d = [NSData
      dataWithContentsOfURL:[[self documents]
                                URLByAppendingPathComponent:@"request.json"]];
  if (!d)
    return;
  NSDictionary *r = [NSJSONSerialization JSONObjectWithData:d
                                                    options:0
                                                      error:nil];
  if (!r || [r[@"id"] isEqual:self.pending])
    return;
  self.pending = r[@"id"];
  self.overrideUserInterfaceStyle = [r[@"dark"] boolValue]
                                        ? UIUserInterfaceStyleDark
                                        : UIUserInterfaceStyleLight;
  self.background.image = [UIImage
      imageWithContentsOfFile:[NSBundle.mainBundle.bundlePath
                                  stringByAppendingPathComponent:
                                      [r[@"background"]
                                          stringByAppendingString:@".png"]]];
  UIGlassContainerEffect *container=[UIGlassContainerEffect new]; container.spacing=8;
  self.group.effect=[r[@"group"] boolValue] ? container : nil;
  NSString *t = r[@"tint"];
  UIColor *c = [t isEqual:@"blue"]
                   ? [UIColor colorWithRed:0 green:122. / 255 blue:1 alpha:1]
               : [t isEqual:@"coral"] ? [UIColor colorWithRed:1
                                                        green:69. / 255
                                                         blue:58. / 255
                                                        alpha:1]
                                      : nil;
  for (int index=0;index<self.glasses.count;++index) {
    UIVisualEffectView *g=self.glasses[index];
    const BOOL pair=[r[@"pair"] boolValue] && index<2;
    g.frame=pair ? CGRectMake(40+index*54,64,52,52) : Frames[index];
    g.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:pair ? 26 : Radii[index]]];
    UIGlassEffect *e =
        [UIGlassEffect effectWithStyle:[r[@"clear"] boolValue] ? UIGlassEffectStyleClear : UIGlassEffectStyleRegular];
    e.interactive = YES;
    e.tintColor = c;
    g.effect = e;
  }
  NSString *identifier = self.pending;
  dispatch_after(
      dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.1 * NSEC_PER_SEC)),
      dispatch_get_main_queue(), ^{
        if (![self.pending isEqual:identifier])
          return;
        NSMutableArray *frames = [NSMutableArray new];
        for (int i = 0; i < 5; i++) {
          CALayer *l =
              self.glasses[i].layer.presentationLayer ?: self.glasses[i].layer;
          CALayer *root =
              self.stage.layer.presentationLayer ?: self.stage.layer;
          CGRect f = [l convertRect:l.bounds toLayer:root];
          [frames addObject:@[
            @(f.origin.x), @(f.origin.y), @(f.size.width), @(f.size.height),
            @([r[@"pair"] boolValue] && i<2 ? 26 : Radii[i])
          ]];
        }
        NSDictionary *m = @{
          @"request" : r,
          @"x" : @(self.stage.frame.origin.x),
          @"y" : @(self.stage.frame.origin.y),
          @"width" : @720,
          @"height" : @440,
          @"scale" : @(self.view.window.screen.scale),
          @"frames" : frames,
          @"os" : UIDevice.currentDevice.systemVersion,
          @"osBuild" : NSProcessInfo.processInfo.operatingSystemVersionString,
          @"reduceTransparency" :
              @(UIAccessibilityIsReduceTransparencyEnabled()),
          @"darkerSystemColors" : @(UIAccessibilityDarkerSystemColorsEnabled())
        };
        [[NSJSONSerialization dataWithJSONObject:m
                                         options:NSJSONWritingPrettyPrinted
                                           error:nil]
            writeToURL:[[self documents]
                           URLByAppendingPathComponent:@"capture.json"]
            atomically:YES];
      });
}
@end
@interface OpticalScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation OpticalScene
- (void)scene:(UIScene *)scene
    willConnectToSession:(UISceneSession *)session
                 options:(UISceneConnectionOptions *)options {
  self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
  self.window.rootViewController = [ShadowController new];
  [self.window makeKeyAndVisible];
}
@end
@interface OpticalApp : UIResponder <UIApplicationDelegate>
@end
@implementation OpticalApp
- (UISceneConfiguration *)application:(UIApplication *)app
    configurationForConnectingSceneSession:(UISceneSession *)s
                                   options:(UISceneConnectionOptions *)o {
  UISceneConfiguration *c =
      [[UISceneConfiguration alloc] initWithName:@"Optical" sessionRole:s.role];
  c.delegateClass = OpticalScene.class;
  return c;
}
@end
int main(int argc, char **argv) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil,
                             NSStringFromClass(OpticalApp.class));
  }
}
