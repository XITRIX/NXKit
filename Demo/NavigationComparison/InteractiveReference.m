// Public UIKit edge-swipe fixture using the same eight navigation arrangements.
#define main CycleReferenceMain
#import "CycleReference.m"
#undef main
static NSMutableArray *InteractiveEvents;
@interface InteractiveScreen:CycleScreen@end
@implementation InteractiveScreen
- (void)viewWillAppear:(BOOL)a {[super viewWillAppear:a];[InteractiveEvents addObject:@[@(self.arrangement),@"willAppear",@(CACurrentMediaTime()),@(self.navigationController.viewControllers.count),self.navigationController.topViewController.title?:@""]];}
- (void)viewDidAppear:(BOOL)a {[super viewDidAppear:a];[InteractiveEvents addObject:@[@(self.arrangement),@"didAppear",@(CACurrentMediaTime()),@(self.navigationController.viewControllers.count),self.navigationController.topViewController.title?:@""]];}
- (void)viewWillDisappear:(BOOL)a {[super viewWillDisappear:a];[InteractiveEvents addObject:@[@(self.arrangement),@"willDisappear",@(CACurrentMediaTime()),@(self.navigationController.viewControllers.count),self.navigationController.topViewController.title?:@""]];}
- (void)viewDidDisappear:(BOOL)a {[super viewDidDisappear:a];[InteractiveEvents addObject:@[@(self.arrangement),@"didDisappear",@(CACurrentMediaTime()),@(self.navigationController.viewControllers.count),self.navigationController.topViewController.title?:@""]];}
@end
@interface InteractiveProbe:CycleProbe<UINavigationControllerDelegate>
@property CFTimeInterval began;
@property BOOL saved;
@property NSString *name;
@end
@implementation InteractiveProbe
- (void)start {
    NSDictionary *env=NSProcessInfo.processInfo.environment;
    NSInteger source=[env[@"SOURCE"]integerValue],target=[env[@"TARGET"]integerValue];self.name=env[@"CASE"]?:@"manual";
    self.window.overrideUserInterfaceStyle=[env[@"DARK"]boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
    InteractiveScreen *root=[[InteractiveScreen alloc]initWithArrangement:-1];
    self.target=[[InteractiveScreen alloc]initWithArrangement:target];self.source=[[InteractiveScreen alloc]initWithArrangement:source];
    self.nav=[[UINavigationController alloc]initWithRootViewController:root];self.nav.viewControllers=@[root,self.target,self.source];
    // Isolate the edge recognizer when testing the modern full-content variant.
    if([self.nav respondsToSelector:@selector(interactiveContentPopGestureRecognizer)])self.nav.interactiveContentPopGestureRecognizer.enabled=NO;
    self.nav.delegate=self;self.window.rootViewController=self.nav;[self.window makeKeyAndVisible];
    self.frames=[NSMutableArray new];InteractiveEvents=[NSMutableArray new];
    [self.nav.interactivePopGestureRecognizer addTarget:self action:@selector(gesture:)];
    self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
}
- (void)navigationController:(UINavigationController*)nav willShowViewController:(UIViewController*)screen animated:(BOOL)animated {[InteractiveEvents addObject:@[@"delegate",@"willShow",@(CACurrentMediaTime()),@(nav.viewControllers.count),screen.title?:@""]];}
- (void)navigationController:(UINavigationController*)nav didShowViewController:(UIViewController*)screen animated:(BOOL)animated {[InteractiveEvents addObject:@[@"delegate",@"didShow",@(CACurrentMediaTime()),@(nav.viewControllers.count),screen.title?:@""]];}
- (void)gesture:(UIPanGestureRecognizer*)pan {if(!self.began&&(pan.state==UIGestureRecognizerStateBegan||pan.state==UIGestureRecognizerStateChanged))self.began=CACurrentMediaTime();if(pan.state==UIGestureRecognizerStateEnded||pan.state==UIGestureRecognizerStateCancelled)[InteractiveEvents addObject:@[@"gesture",@(pan.state),@(CACurrentMediaTime()),@([pan translationInView:self.nav.view].x),@([pan velocityInView:self.nav.view].x)]];}
- (NSURL*)directory {NSURL *base=[[[NSFileManager defaultManager]URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:@"interactive"];NSURL *d=[base URLByAppendingPathComponent:self.name];[NSFileManager.defaultManager createDirectoryAtURL:d withIntermediateDirectories:YES attributes:nil error:nil];return d;}
- (void)tick:(CADisplayLink*)link {
    if(self.saved||!self.began)return;double t=CACurrentMediaTime()-self.began;
    NSMutableArray *views=[NSMutableArray new];[self views:self.nav.navigationBar path:@"bar" opacity:1 into:views];
    UIPanGestureRecognizer *pan=(UIPanGestureRecognizer*)self.nav.interactivePopGestureRecognizer;
    NSMutableDictionary *frame=[@{@"t":@(t),@"state":@(pan.state),@"translation":@([pan translationInView:self.nav.view].x),@"velocity":@([pan velocityInView:self.nav.view].x),@"count":@(self.nav.viewControllers.count),@"top":self.nav.topViewController.title?:@"",@"sourceContent":[self contentFrame:self.source],@"targetContent":[self contentFrame:self.target],@"views":views}mutableCopy];
    if(self.frames.count%4==0){
        NSString *file=[NSString stringWithFormat:@"%04lu.png",(unsigned long)self.frames.count];
        CGRect crop=CGRectMake(0,self.nav.navigationBar.frame.origin.y-12,self.window.bounds.size.width,80);
        UIGraphicsImageRendererFormat *format=UIGraphicsImageRendererFormat.defaultFormat;format.scale=2;
        UIImage *image=[[[UIGraphicsImageRenderer alloc]initWithSize:crop.size format:format]imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,0,-crop.origin.y);[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
        [UIImagePNGRepresentation(image)writeToURL:[[self directory]URLByAppendingPathComponent:file]atomically:YES];frame[@"image"]=file;
    }
    [self.frames addObject:frame];
    if(t>2.0){self.saved=YES;[self.link invalidate];NSDictionary *report=@{@"name":self.name,@"os":UIDevice.currentDevice.systemVersion,@"source":@(((CycleScreen*)self.source).arrangement),@"target":@(((CycleScreen*)self.target).arrangement),@"frames":self.frames,@"events":InteractiveEvents,@"began":@(self.began)};[[NSJSONSerialization dataWithJSONObject:report options:0 error:nil]writeToURL:[[self directory]URLByAppendingPathComponent:@"trace.json"]atomically:YES];}
}
@end
@interface InteractiveScene:UIResponder<UIWindowSceneDelegate>@property UIWindow *window;@property InteractiveProbe *probe;@end
@implementation InteractiveScene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)scene];self.probe=[InteractiveProbe new];self.probe.window=self.window;[self.probe start];}
@end
@interface InteractiveApp:UIResponder<UIApplicationDelegate>@end
@implementation InteractiveApp
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {UISceneConfiguration *c=[[UISceneConfiguration alloc]initWithName:@"Interactive" sessionRole:session.role];c.delegateClass=InteractiveScene.class;return c;}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(InteractiveApp.class));}}
