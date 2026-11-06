// Public UIGlassEffect renderer. The preference adapter is fixture-only.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dlfcn.h>
static NSNumber *(*getAmount)(void);
@interface MatrixController : UIViewController
@property UIView *stage;
@property NSArray *cases;
@property NSUInteger index;
@property NSTimer *ackTimer;
@end
@implementation MatrixController
- (BOOL)prefersStatusBarHidden { return YES; }
- (NSURL *)documents { return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject; }
- (void)viewDidLoad {
 [super viewDidLoad]; self.view.backgroundColor=UIColor.blackColor;
 self.stage=[[UIView alloc] initWithFrame:CGRectMake(0,0,384,704)];[self.view addSubview:self.stage];
 self.cases=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:[NSBundle.mainBundle URLForResource:@"cases" withExtension:@"json"]] options:0 error:nil];
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(),^{[self next];});
}
- (void)viewDidLayoutSubviews { [super viewDidLayoutSubviews];self.stage.center=CGPointMake(CGRectGetMidX(self.view.bounds),CGRectGetMidY(self.view.bounds)); }
- (void)next {
 if(self.index>=self.cases.count){NSLog(@"GLASS_MATRIX_DONE");return;}
 NSDictionary *c=self.cases[self.index++];self.overrideUserInterfaceStyle=[c[@"dark"] boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 for(UIView *v in self.stage.subviews)[v removeFromSuperview];
 self.stage.backgroundColor=UIColor.blackColor;
 UIImage *background=[UIImage imageWithContentsOfFile:[NSBundle.mainBundle.bundlePath stringByAppendingPathComponent:c[@"background"]]];
 UIImageView *bg=[[UIImageView alloc] initWithImage:background];bg.frame=self.stage.bounds;[self.stage addSubview:bg];
 for(NSDictionary *p in c[@"probes"]) {
  UIGlassEffect *e=[UIGlassEffect effectWithStyle:[p[@"clear"] boolValue]?UIGlassEffectStyleClear:UIGlassEffectStyleRegular];
  if([p[@"tinted"] boolValue]) e.tintColor=[UIColor colorWithRed:0 green:122./255 blue:1 alpha:1];
  UIVisualEffectView *g=[[UIVisualEffectView alloc] initWithEffect:e];NSArray *f=p[@"frame"];g.frame=CGRectMake([f[0] doubleValue],[f[1] doubleValue],[f[2] doubleValue],[f[3] doubleValue]);
  g.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:g.bounds.size.height/2]];[self.stage addSubview:g];
 }
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.7*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
  UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];format.scale=self.view.window.screen.scale;format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
  UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithBounds:self.stage.bounds format:format] imageWithActions:^(UIGraphicsImageRendererContext *ctx){[self.stage drawViewHierarchyInRect:self.stage.bounds afterScreenUpdates:YES];}];
  NSString *prefix=[NSString stringWithFormat:@"%@-%@",NSProcessInfo.processInfo.environment[@"MATRIX_AMOUNT"]?:@"system",c[@"id"]];
  [UIImagePNGRepresentation(image) writeToURL:[[self documents] URLByAppendingPathComponent:[prefix stringByAppendingString:@".png"]] atomically:YES];
  NSDictionary *meta=@{@"case":c,@"amount":getAmount?getAmount()?:NSNull.null:NSNull.null,@"requested":NSProcessInfo.processInfo.environment[@"MATRIX_AMOUNT"]?:@"system",@"scale":@(format.scale),@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"stageOrigin":@[@(self.stage.frame.origin.x),@(self.stage.frame.origin.y)],@"reduceTransparency":@(UIAccessibilityIsReduceTransparencyEnabled())};
  [[NSJSONSerialization dataWithJSONObject:meta options:NSJSONWritingPrettyPrinted error:nil] writeToURL:[[self documents] URLByAppendingPathComponent:[prefix stringByAppendingString:@".json"]] atomically:YES];
  self.ackTimer=[NSTimer scheduledTimerWithTimeInterval:.05 repeats:YES block:^(NSTimer *timer){
   NSString *ack=[NSString stringWithContentsOfURL:[[self documents] URLByAppendingPathComponent:@"ack.txt"] encoding:NSUTF8StringEncoding error:nil];
   if([ack isEqual:prefix]){[timer invalidate];self.ackTimer=nil;[self next];}
  }];
 });
}
@end
@interface MatrixScene:UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation MatrixScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];self.window.rootViewController=[MatrixController new];[self.window makeKeyAndVisible];}
@end
@interface MatrixDelegate:UIResponder<UIApplicationDelegate>
@end
@implementation MatrixDelegate
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Matrix" sessionRole:session.role];c.delegateClass=MatrixScene.class;return c;}
@end
int main(int argc,char **argv){@autoreleasepool {
 // Volatile argument domain is local to this process, never a persistent setting.
 NSString *amount=NSProcessInfo.processInfo.environment[@"MATRIX_AMOUNT"];
 NSUserDefaults *(*defaults)(void)=dlsym(RTLD_DEFAULT,"_UIKitUserDefaults");
 if(amount && defaults) {
  NSMutableDictionary *domain=[[defaults() volatileDomainForName:NSArgumentDomain] mutableCopy]?:[NSMutableDictionary new];domain[@"UIViewGlassTintAmount"]=@([amount doubleValue]);
  [defaults() setVolatileDomain:domain forName:NSArgumentDomain];
 }
 getAmount=dlsym(RTLD_DEFAULT,"UIViewGlassGetTintAmount");
 NSLog(@"GLASS_MATRIX_REQUEST=%@ EFFECTIVE=%@",amount,getAmount?getAmount():nil);
 return UIApplicationMain(argc,argv,nil,NSStringFromClass(MatrixDelegate.class));
}}
