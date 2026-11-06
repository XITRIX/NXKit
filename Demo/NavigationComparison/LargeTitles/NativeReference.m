// Public UIKit large-title layout oracle. No private selectors or view mutation.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
@interface Screen:UIViewController
@property UIScrollView *scroll;
@end
@implementation Screen
- (void)loadView {self.scroll=[UIScrollView new];self.scroll.alwaysBounceVertical=YES;self.scroll.contentSize=CGSizeMake(466,1800);self.view=self.scroll;self.view.backgroundColor=UIColor.systemBackgroundColor;for(int i=0;i<30;i++){UILabel*l=[[UILabel alloc]initWithFrame:CGRectMake(20,i*50,360,44)];l.text=[NSString stringWithFormat:@"Content row %d",i];[self.scroll addSubview:l];}}
@end
@interface Probe:NSObject<UIScrollViewDelegate>
@property UIWindow *window;@property UINavigationController *nav;@property NSMutableArray *states;@property Screen *root;@property CADisplayLink *link;@property NSMutableArray *frames;@property CFTimeInterval origin;
@end
@implementation Probe
- (void)labels:(UIView*)v into:(NSMutableArray*)rows {if([v isKindOfClass:UILabel.class]){UILabel*l=(UILabel*)v;[rows addObject:@{@"text":l.text?:@"",@"font":@(l.font.pointSize),@"weight":[l.font.fontDescriptor objectForKey:UIFontDescriptorTraitsAttribute]?:@{},@"frame":NSStringFromCGRect([v convertRect:v.bounds toView:self.nav.navigationBar]),@"alpha":@(v.alpha),@"hidden":@(v.hidden)}];}for(UIView*c in v.subviews)[self labels:c into:rows];}
- (void)state:(NSString*)name {[self.window layoutIfNeeded];Screen*s=(Screen*)self.nav.topViewController;NSMutableArray*labels=[NSMutableArray new];[self labels:self.nav.navigationBar into:labels];[self.states addObject:@{@"name":name,@"bar":NSStringFromCGRect(self.nav.navigationBar.frame),@"safeTop":@(s.view.safeAreaInsets.top),@"offset":@(s.scroll.contentOffset.y),@"inset":@(s.scroll.adjustedContentInset.top),@"labels":labels}];}
- (void)later:(double)t run:(void(^)(void))run {dispatch_after(dispatch_time(DISPATCH_TIME_NOW,t*NSEC_PER_SEC),dispatch_get_main_queue(),run);}
- (void)tick:(CADisplayLink*)link {NSMutableArray*labels=[NSMutableArray new];[self labels:self.nav.navigationBar into:labels];[self.frames addObject:@{@"t":@(CACurrentMediaTime()-self.origin),@"offset":@(self.root.scroll.contentOffset.y),@"inset":@(self.root.scroll.adjustedContentInset.top),@"safe":@(self.root.scroll.safeAreaInsets.top),@"height":@(self.nav.navigationBar.frame.size.height),@"tracking":@(self.root.scroll.tracking),@"labels":labels}];}
- (void)scrollViewWillEndDragging:(UIScrollView*)scroll withVelocity:(CGPoint)velocity targetContentOffset:(inout CGPoint*)target {[self.states addObject:@{@"name":@"release",@"velocity":@(velocity.y),@"target":@(target->y),@"offset":@(scroll.contentOffset.y)}];}
- (void)start {self.states=[NSMutableArray new];self.window.overrideUserInterfaceStyle=UIUserInterfaceStyleDark;self.root=[Screen new];self.root.title=@"Collection";self.nav=[[UINavigationController alloc]initWithRootViewController:self.root];self.nav.navigationBar.prefersLargeTitles=YES;self.window.rootViewController=self.nav;[self.window makeKeyAndVisible];
if(NSProcessInfo.processInfo.environment[@"TOUCH_CASE"]) {
 self.root.scroll.delegate=self;self.frames=[NSMutableArray new];self.origin=CACurrentMediaTime();self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 [self later:9 run:^{[self.link invalidate];NSDictionary*r=@{@"frames":self.frames,@"states":self.states};NSString*p=[NSHomeDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"Documents/touch-%@.json",NSProcessInfo.processInfo.environment[@"TOUCH_CASE"]]];[[NSJSONSerialization dataWithJSONObject:r options:0 error:nil]writeToFile:p atomically:YES];}];return;
}

[self later:.7 run:^{[self state:@"default"];self.nav.navigationBar.prefersLargeTitles=YES;}];
[self later:1.3 run:^{[self state:@"enabled"]; }];
[self later:1.60 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-168) animated:NO];}];
[self later:1.75 run:^{[self state:@"offset-168"];}];
[self later:1.82 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-160) animated:NO];}];
[self later:1.97 run:^{[self state:@"offset-160"];}];
[self later:2.04 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-150) animated:NO];}];
[self later:2.19 run:^{[self state:@"offset-150"];}];
[self later:2.26 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-140) animated:NO];}];
[self later:2.41 run:^{[self state:@"offset-140"];}];
[self later:2.48 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-130) animated:NO];}];
[self later:2.63 run:^{[self state:@"offset-130"];}];
[self later:2.70 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-120) animated:NO];}];
[self later:2.85 run:^{[self state:@"offset-120"];}];
[self later:2.92 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-116) animated:NO];}];
[self later:3.07 run:^{[self state:@"offset-116"];}];
[self later:3.14 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-20) animated:NO];}];
[self later:3.29 run:^{[self state:@"offset-20"];}];
[self later:3.36 run:^{[self.root.scroll setContentOffset:CGPointMake(0,0) animated:NO];}];
[self later:3.51 run:^{[self state:@"offset0"];}];
[self later:3.58 run:^{[self.root.scroll setContentOffset:CGPointMake(0,100) animated:NO];}];
[self later:3.73 run:^{[self state:@"offset100"];}];
[self later:3.80 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-200) animated:NO];}];
[self later:3.95 run:^{[self state:@"offset-200"];}];
[self later:4.3 run:^{[self.root.scroll setContentOffset:CGPointMake(0,-self.root.scroll.adjustedContentInset.top) animated:NO];}];
[self later:4.6 run:^{[self state:@"expanded"];Screen*s=[Screen new];s.title=@"Automatic";[self.nav pushViewController:s animated:NO];}];
[self later:5.0 run:^{[self state:@"automatic-child"];Screen*s=[Screen new];s.title=@"Never";s.navigationItem.largeTitleDisplayMode=UINavigationItemLargeTitleDisplayModeNever;[self.nav pushViewController:s animated:NO];}];
[self later:5.4 run:^{[self state:@"never"];Screen*s=[Screen new];s.title=@"Inherited";[self.nav pushViewController:s animated:NO];}];
[self later:5.8 run:^{[self state:@"automatic-after-never"];self.nav.topViewController.navigationItem.largeTitleDisplayMode=UINavigationItemLargeTitleDisplayModeAlways;}];
[self later:6.2 run:^{[self state:@"always"];[self.nav popToRootViewControllerAnimated:NO];}];
[self later:6.6 run:^{[self state:@"pop-root"];NSString*p=[NSHomeDirectory() stringByAppendingPathComponent:@"Documents/large-titles.json"];[[NSJSONSerialization dataWithJSONObject:self.states options:NSJSONWritingPrettyPrinted error:nil]writeToFile:p atomically:YES];}];
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
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
