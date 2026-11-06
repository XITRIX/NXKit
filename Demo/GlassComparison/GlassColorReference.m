// Public UIKit color-transfer fixture. No private selectors or layer filters.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
@interface ColorController : UIViewController
@property UIView *stage;
@property NSDictionary *request;
@property NSTimer *timer;
@end
@implementation ColorController
- (BOOL)prefersStatusBarHidden { return YES; }
- (NSURL *)documents { return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject; }
- (void)viewDidLoad {
 [super viewDidLoad];self.view.backgroundColor=UIColor.blackColor;self.stage=[[UIView alloc] initWithFrame:CGRectMake(0,0,720,560)];[self.view addSubview:self.stage];
 [NSFileManager.defaultManager removeItemAtURL:[[self documents] URLByAppendingPathComponent:@"request.json"] error:nil];
 __weak ColorController *weak=self;self.timer=[NSTimer scheduledTimerWithTimeInterval:.05 repeats:YES block:^(NSTimer *t){[weak poll];}];
}
- (void)viewDidLayoutSubviews { [super viewDidLayoutSubviews];self.stage.frame=CGRectMake(floor((self.view.bounds.size.width-720)/2),floor((self.view.bounds.size.height-560)/2),720,560); }
- (NSArray *)palette { return @[@[@0,@122,@255],@[@255,@59,@48],@[@52,@199,@89],@[@255,@149,@0],@[@175,@82,@222],@[@0,@199,@190],@[@255,@255,@255],@[@0,@0,@0],@[@128,@128,@128],@[@32,@64,@96],@[@128,@192,@255],@[@255,@45,@85]]; }
- (void)populate:(UIView *)host alpha:(CGFloat)alpha {
 NSArray *palette=[self palette];
 UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];format.scale=2;format.opaque=NO;format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
 UIImage *mask=[[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(24,20) format:format] imageWithActions:^(UIGraphicsImageRendererContext *ctx){[UIColor.whiteColor setFill];UIRectFill(CGRectMake(0,0,24,20));}];mask=[mask imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
 for(NSUInteger i=0;i<palette.count;i++) {
  NSArray *rgb=palette[i];UIColor *color=[UIColor colorWithRed:[rgb[0] doubleValue]/255 green:[rgb[1] doubleValue]/255 blue:[rgb[2] doubleValue]/255 alpha:alpha];if([self.request[@"semantic"] boolValue]) {NSArray *colors=@[UIColor.systemBlueColor,UIColor.systemRedColor,UIColor.systemGreenColor,UIColor.systemOrangeColor,UIColor.systemPurpleColor,UIColor.systemMintColor,UIColor.labelColor,UIColor.secondaryLabelColor,UIColor.systemGrayColor,UIColor.systemTealColor,UIColor.systemCyanColor,UIColor.systemPinkColor];color=[colors[i] colorWithAlphaComponent:alpha];}CGFloat x=14+i*55;
  UIView *swatch=[[UIView alloc] initWithFrame:CGRectMake(x,12,24,20)];swatch.backgroundColor=color;[host addSubview:swatch];
  UILabel *label=[[UILabel alloc] initWithFrame:CGRectMake(x,40,44,32)];label.text=@"HH";label.font=[UIFont systemFontOfSize:28 weight:UIFontWeightBold];label.textColor=color;[host addSubview:label];
  UIImageView *icon=[[UIImageView alloc] initWithImage:mask];icon.frame=CGRectMake(x,82,24,20);icon.tintColor=color;[host addSubview:icon];
 }
}
- (NSArray *)systemColors { return @[UIColor.systemBlueColor,UIColor.systemRedColor,UIColor.systemGreenColor,UIColor.systemOrangeColor,UIColor.systemPurpleColor,UIColor.systemMintColor,UIColor.systemYellowColor,UIColor.systemIndigoColor,UIColor.systemGrayColor,UIColor.systemTealColor,UIColor.systemCyanColor,UIColor.systemPinkColor]; }
- (void)collectImages:(UIView *)view into:(NSMutableArray *)frames {
 if([view isKindOfClass:UIImageView.class]) {CGRect f=[view convertRect:view.bounds toView:self.stage];if(f.size.width>10&&f.size.width<60&&f.size.height>10&&f.size.height<60)[frames addObject:@[@(f.origin.x),@(f.origin.y),@(f.size.width),@(f.size.height)]];}
 for(UIView *child in view.subviews)[self collectImages:child into:frames];
}
- (void)poll {
 NSData *data=[NSData dataWithContentsOfURL:[[self documents] URLByAppendingPathComponent:@"request.json"]];if(!data)return;
 NSDictionary *request=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];if(![request isKindOfClass:NSDictionary.class]||[request isEqual:self.request])return;self.request=request;
 self.overrideUserInterfaceStyle=[request[@"dark"] boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 for(UIView *v in self.stage.subviews)[v removeFromSuperview];
 NSArray *bg=request[@"backgroundRGB"]?:@[@128,@128,@128];self.stage.backgroundColor=[UIColor colorWithRed:[bg[0] doubleValue]/255 green:[bg[1] doubleValue]/255 blue:[bg[2] doubleValue]/255 alpha:1];
 if([request[@"tabPalette"] boolValue]) {
  UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(24,20)];UIImage *image=[[renderer imageWithActions:^(UIGraphicsImageRendererContext *ctx){[UIColor.whiteColor setFill];UIRectFill(CGRectMake(0,0,24,20));}] imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
  NSMutableArray *tabs=[NSMutableArray new];NSArray *colors=[self systemColors];
  for(NSUInteger i=0;i<colors.count;i++) {UITabBar *bar=[[UITabBar alloc] initWithFrame:CGRectMake(5+(i%3)*240,10+(i/3)*135,230,83)];bar.traitOverrides.horizontalSizeClass=UIUserInterfaceSizeClassCompact;NSMutableArray *items=[NSMutableArray new];for(int k=0;k<3;k++)[items addObject:[[UITabBarItem alloc] initWithTitle:@"" image:image tag:k]];bar.items=items;bar.selectedItem=items[1];bar.tintColor=colors[i];[self.stage addSubview:bar];[bar layoutIfNeeded];[tabs addObject:bar];}
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
   if(![self.request isEqual:request])return;NSMutableArray *rows=[NSMutableArray new];for(UITabBar *bar in tabs){NSMutableArray *frames=[NSMutableArray new];[self collectImages:bar into:frames];CGFloat r,g,b,a;[[bar.tintColor resolvedColorWithTraitCollection:bar.traitCollection] getRed:&r green:&g blue:&b alpha:&a];[rows addObject:@{@"images":frames,@"sourceRGB":@[@(r*255),@(g*255),@(b*255)]}];}
   NSDictionary *meta=@{@"request":request,@"x":@(self.stage.frame.origin.x),@"y":@(self.stage.frame.origin.y),@"width":@720,@"height":@560,@"scale":@(self.view.window.screen.scale),@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"tabs":rows};NSData *result=[NSJSONSerialization dataWithJSONObject:meta options:NSJSONWritingPrettyPrinted error:nil];[result writeToURL:[[self documents] URLByAppendingPathComponent:@"capture.json"] atomically:YES];
  });return;
 }
 CGFloat alpha=request[@"alpha"]?[request[@"alpha"] doubleValue]:1;
 UIView *outside=[[UIView alloc] initWithFrame:CGRectMake(20,0,680,110)];[self.stage addSubview:outside];[self populate:outside alpha:alpha];
 for(NSUInteger i=0;i<2;i++) {
  UIGlassEffect *effect=[UIGlassEffect effectWithStyle:i?UIGlassEffectStyleClear:UIGlassEffectStyleRegular];
  if(request[@"tintRGB"]) {NSArray *rgb=request[@"tintRGB"];effect.tintColor=[UIColor colorWithRed:[rgb[0] doubleValue]/255 green:[rgb[1] doubleValue]/255 blue:[rgb[2] doubleValue]/255 alpha:1];}
  UIVisualEffectView *glass=[[UIVisualEffectView alloc] initWithEffect:effect];glass.frame=CGRectMake(20,120+i*120,680,110);glass.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:36]];[self.stage addSubview:glass];[self populate:glass.contentView alpha:alpha];
 }
 // Explicit public vibrancy is a control, separate from automatic glass content.
 UIBlurEffect *blur=[UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemMaterial];UIVisualEffectView *blurView=[[UIVisualEffectView alloc] initWithEffect:blur];blurView.frame=CGRectMake(20,360,680,110);[self.stage addSubview:blurView];
 UIVisualEffectView *vibrant=[[UIVisualEffectView alloc] initWithEffect:[UIVibrancyEffect effectForBlurEffect:blur style:UIVibrancyEffectStyleLabel]];vibrant.frame=blurView.bounds;[blurView.contentView addSubview:vibrant];[self populate:vibrant.contentView alpha:alpha];
 UITabBar *tab=[[UITabBar alloc] initWithFrame:CGRectMake(159,480,402,83)];tab.traitOverrides.horizontalSizeClass=UIUserInterfaceSizeClassCompact;NSMutableArray *items=[NSMutableArray new];for(NSString *name in @[@"house.fill",@"star.fill",@"books.vertical.fill",@"slider.horizontal.3"])[items addObject:[[UITabBarItem alloc] initWithTitle:@"HH" image:[UIImage systemImageNamed:name] tag:items.count]];tab.items=items;tab.selectedItem=items[2];if(![request[@"semantic"] boolValue])tab.tintColor=[UIColor colorWithRed:0 green:122.0/255 blue:1 alpha:1];[self.stage addSubview:tab];
 NSString *nonce=request[@"captureNonce"]?:request[@"id"];
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
  if(![self.request isEqual:request])return;
  NSDictionary *meta=@{@"request":request,@"x":@(self.stage.frame.origin.x),@"y":@(self.stage.frame.origin.y),@"width":@720,@"height":@560,@"scale":@(self.view.window.screen.scale),@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"palette":[self palette],@"rows":@[@"ordinary",@"regular glass",@"clear glass",@"explicit blur vibrancy"],@"probeGeometry":@{@"rowY":@[@0,@120,@240,@360],@"swatchOrigin":@[@34,@12],@"columnStep":@55,@"swatchSize":@[@24,@20],@"templateOffsetY":@70},@"nonce":nonce};
  NSData *result=[NSJSONSerialization dataWithJSONObject:meta options:NSJSONWritingPrettyPrinted error:nil];[result writeToURL:[[self documents] URLByAppendingPathComponent:@"capture.json"] atomically:YES];
 });
}
@end
@interface ColorScene : UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation ColorScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];self.window.rootViewController=[ColorController new];[self.window makeKeyAndVisible];}
@end
@interface ColorDelegate : UIResponder<UIApplicationDelegate>
@end
@implementation ColorDelegate
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Colors" sessionRole:session.role];c.delegateClass=ColorScene.class;return c;}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(ColorDelegate.class));}}
