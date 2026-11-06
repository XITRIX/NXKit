// Public UIKit oracle: programmatic operations, lifecycle/delegate traces and
// read-only UIView/CALayer presentation geometry. No private UIKit calls.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
static NSMutableArray *events;
static NSString *operation=@"launch";
static void Log(NSString *event) {[events addObject:@{@"operation":operation,@"event":event}];}
static NSArray *RectValues(CGRect r) {return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
@interface Screen:UIViewController
@end
@implementation Screen
- (void)loadView {Log([self.title stringByAppendingString:@".load"]);self.view=[UIView new];self.view.backgroundColor=UIColor.systemGroupedBackgroundColor;}
- (void)viewWillAppear:(BOOL)a {[super viewWillAppear:a];Log([self.title stringByAppendingString:@".willAppear"]);}
- (void)viewDidAppear:(BOOL)a {[super viewDidAppear:a];Log([self.title stringByAppendingString:@".didAppear"]);}
- (void)viewWillDisappear:(BOOL)a {[super viewWillDisappear:a];Log([self.title stringByAppendingString:@".willDisappear"]);}
- (void)viewDidDisappear:(BOOL)a {[super viewDidDisappear:a];Log([self.title stringByAppendingString:@".didDisappear"]);}
@end
@interface Probe:NSObject<UINavigationControllerDelegate,UINavigationBarDelegate>
@property UIWindow *window;
@property UINavigationController *nav;
@property NSArray<Screen*> *screens;
@property NSMutableArray *states,*frames;
@property CADisplayLink *link;
@property CFTimeInterval origin;
@property BOOL veto;
@end
@implementation Probe
- (void)navigationController:(UINavigationController*)n willShowViewController:(UIViewController*)v animated:(BOOL)a {Log([@"willShow:" stringByAppendingString:v.title]);}
- (void)navigationController:(UINavigationController*)n didShowViewController:(UIViewController*)v animated:(BOOL)a {Log([@"didShow:" stringByAppendingString:v.title]);}
- (BOOL)navigationBar:(UINavigationBar*)b shouldPushItem:(UINavigationItem*)i {Log(@"bar.shouldPush");return !self.veto;}
- (void)navigationBar:(UINavigationBar*)b didPushItem:(UINavigationItem*)i {Log(@"bar.didPush");}
- (BOOL)navigationBar:(UINavigationBar*)b shouldPopItem:(UINavigationItem*)i {Log(@"bar.shouldPop");return !self.veto;}
- (void)navigationBar:(UINavigationBar*)b didPopItem:(UINavigationItem*)i {Log(@"bar.didPop");}
- (NSURL*)directory {NSURL *url=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;return url;}
- (void)state:(NSString*)name {
 [self.window layoutIfNeeded];UIViewController *top=self.nav.topViewController;
 [self.states addObject:@{@"name":name,@"stack":[self.nav.viewControllers valueForKey:@"title"],@"top":top.title?:@"nil",@"visible":self.nav.visibleViewController.title?:@"nil",@"bar":RectValues(self.nav.navigationBar.frame),@"safe":@[@(top.view.safeAreaInsets.top),@(top.view.safeAreaInsets.bottom)],@"barStack":@(self.nav.navigationBar.items.count)}];
}
- (void)picture:(NSString*)name {
 UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];format.scale=2;
 UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithSize:self.window.bounds.size format:format] imageWithActions:^(UIGraphicsImageRendererContext *ctx){[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
 [UIImagePNGRepresentation(image) writeToURL:[[self directory] URLByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
}
- (void)collect:(UIView*)v path:(NSString*)path rows:(NSMutableArray*)rows {
 CALayer *p=v.layer.presentationLayer?:v.layer;
 if(!v.hidden && v.alpha>.001) {
  NSMutableDictionary *row=[@{@"path":path,@"class":NSStringFromClass(v.class),@"frame":RectValues(v.frame),@"presentation":RectValues(p.frame),@"alpha":@(p.opacity)} mutableCopy];
  if([v isKindOfClass:UILabel.class]){UILabel *label=(UILabel*)v;row[@"text"]=label.text?:@"";row[@"fontName"]=label.font.fontName;row[@"fontSize"]=@(label.font.pointSize);row[@"fontTraits"]=[label.font.fontDescriptor objectForKey:UIFontDescriptorTraitsAttribute]?:@{};}
  row[@"barFrame"]=RectValues([v convertRect:v.bounds toView:self.nav.navigationBar]);
  CALayer *barLayer=self.nav.navigationBar.layer.presentationLayer?:self.nav.navigationBar.layer;
  row[@"barPresentation"]=RectValues([p convertRect:p.bounds toLayer:barLayer]);
  [rows addObject:row];
  NSUInteger index=0;for(UIView *child in v.subviews)[self collect:child path:[path stringByAppendingFormat:@"/%lu",(unsigned long)index++] rows:rows];
 }
}
- (void)tick:(CADisplayLink*)link {
 NSMutableArray *rows=[NSMutableArray new];[self collect:self.nav.navigationBar path:@"bar" rows:rows];
 [self.frames addObject:@{@"t":@(CACurrentMediaTime()-self.origin),@"operation":operation,@"views":rows}];
}
- (void)later:(double)t run:(void(^)(void))run {dispatch_after(dispatch_time(DISPATCH_TIME_NOW,t*NSEC_PER_SEC),dispatch_get_main_queue(),run);}
- (void)start {
 self.window.overrideUserInterfaceStyle=[NSProcessInfo.processInfo.arguments containsObject:@"--dark"]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 events=[NSMutableArray new];self.states=[NSMutableArray new];self.frames=[NSMutableArray new];
 Screen *a=[Screen new],*b=[Screen new],*c=[Screen new];a.title=@"Home";b.title=@"Details";c.title=@"Editor";self.screens=@[a,b,c];
 UIBarButtonItem *more=[[UIBarButtonItem alloc] initWithImage:[UIImage systemImageNamed:@"ellipsis"] style:UIBarButtonItemStylePlain target:nil action:NULL];
 a.navigationItem.leftBarButtonItem=[[UIBarButtonItem alloc] initWithTitle:@"Close" style:UIBarButtonItemStylePlain target:nil action:NULL];
 a.navigationItem.rightBarButtonItems=@[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd primaryAction:nil],more];
 b.navigationItem.rightBarButtonItems=@[more];
 c.navigationItem.rightBarButtonItems=@[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone primaryAction:nil]];
 self.nav=[[UINavigationController alloc] initWithRootViewController:a];self.nav.delegate=self;self.window.rootViewController=self.nav;[self.window makeKeyAndVisible];
 self.origin=CACurrentMediaTime();self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 [self later:.8 run:^{[self state:@"root"];
  UINavigationBar *bar=[[UINavigationBar alloc] initWithFrame:CGRectMake(0,0,390,44)];bar.delegate=self;
  operation=@"standalone";UINavigationItem *r=[[UINavigationItem alloc] initWithTitle:@"R"],*d=[[UINavigationItem alloc] initWithTitle:@"D"];
  [bar pushNavigationItem:r animated:NO];[bar pushNavigationItem:d animated:NO];self.veto=YES;
  UINavigationItem *vetoResult=[bar popNavigationItemAnimated:NO];NSUInteger vetoCount=bar.items.count;self.veto=NO;[bar popNavigationItemAnimated:NO];UINavigationItem *rootResult=[bar popNavigationItemAnimated:NO];NSUInteger rootCount=bar.items.count;
  [bar setItems:@[r,d] animated:NO];[self.states addObject:@{@"name":@"barSemantics",@"vetoReturn":vetoResult.title?:@"nil",@"vetoCount":@(vetoCount),@"rootReturn":rootResult.title?:@"nil",@"rootCount":@(rootCount),@"count":@(bar.items.count),@"back":bar.backItem.title?:@"nil"}];
 }];
 [self later:1.2 run:^{operation=@"push";[self.nav pushViewController:b animated:YES];[self state:@"push-immediate"];}];
 [self later:1.38 run:^{[self state:@"push-morph"];}];
 [self later:2.2 run:^{[self state:@"pushed"];operation=@"push-prominent";[self.nav pushViewController:c animated:YES];}];
 [self later:3.2 run:^{[self state:@"editor"];operation=@"pop";[self.nav popViewControllerAnimated:YES];[self state:@"pop-immediate"];}];
 [self later:3.38 run:^{[self state:@"pop-morph"];}];
 [self later:4.2 run:^{[self state:@"popped"];operation=@"pop-root";[self.nav popToRootViewControllerAnimated:YES];}];
 [self later:5.2 run:^{[self state:@"root-restored"];operation=@"replace";[self.nav setViewControllers:@[a,c] animated:NO];[self state:@"replaced"];}];
 [self later:5.5 run:^{operation=@"hidden";[self.nav setNavigationBarHidden:YES animated:NO];[self state:@"hidden"];}];
 [self later:5.8 run:^{operation=@"shown";[self.nav setNavigationBarHidden:NO animated:NO];[self state:@"shown"];}];
 [self later:.95 run:^{[self picture:@"root"];}];
 [self later:2.1 run:^{[self picture:@"pushed"];}];
 [self later:3.1 run:^{[self picture:@"editor"];}];
 [self later:4.1 run:^{[self picture:@"popped"];}];
 [self later:5.1 run:^{[self picture:@"root-restored"];}];
 [self later:6.2 run:^{[self.link invalidate];NSDictionary *result=@{@"os":UIDevice.currentDevice.systemVersion,@"events":events,@"states":self.states,@"frames":self.frames};[[NSJSONSerialization dataWithJSONObject:result options:0 error:nil] writeToURL:[[self directory] URLByAppendingPathComponent:@"native.json"] atomically:YES];}];
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>
@property UIWindow *window;
@property Probe *probe;
@end
@implementation Scene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];self.probe=[Probe new];self.probe.window=self.window;[self.probe start];}
@end
@interface App:UIResponder<UIApplicationDelegate>
@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Navigation" sessionRole:session.role];c.delegateClass=Scene.class;return c;}
@end
int main(int argc,char**argv) {@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
