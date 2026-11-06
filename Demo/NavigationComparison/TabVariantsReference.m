// Public UIKit oracle for floating tab geometry; private class names are
// observed through ordinary UIView traversal, never called as private APIs.
#import <UIKit/UIKit.h>
static NSArray *RectValues(CGRect r) {return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
@interface TabProbe:NSObject
@property UIWindow *window;
@property UITabBarController *tabs;
@property NSMutableArray *records;
@property NSUInteger step;
@end
@implementation TabProbe
- (NSURL*)directory {return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;}
- (void)collect:(UIView*)view rows:(NSMutableArray*)rows {
 if(view.hidden||view.alpha<.01)return;
 NSMutableDictionary *row=[@{@"class":NSStringFromClass(view.class),@"frame":RectValues([view convertRect:view.bounds toView:self.window])} mutableCopy];
 if([view isKindOfClass:UILabel.class])row[@"text"]=((UILabel*)view).text?:@"";
 if([view isKindOfClass:UIControl.class]){CGSize intrinsic=view.intrinsicContentSize;row[@"intrinsic"]=@[@(intrinsic.width),@(intrinsic.height)];}
 [rows addObject:row];for(UIView *child in view.subviews)[self collect:child rows:rows];
}
- (void)start {
 self.records=[NSMutableArray new];self.tabs=[UITabBarController new];self.tabs.mode=UITabBarControllerModeTabBar;
 self.window.rootViewController=self.tabs;[self.window makeKeyAndVisible];[self next];
}
- (void)next {
 if(self.step==32){
  NSDictionary *result=@{@"os":UIDevice.currentDevice.systemVersion,@"scale":@(self.window.screen.scale),@"records":self.records};
  [[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil] writeToURL:[[self directory] URLByAppendingPathComponent:@"tab-variants.json"] atomically:YES];return;
 }
 if(self.step==16&&!self.tabs.parentViewController){UIViewController *host=[UIViewController new];self.window.rootViewController=host;[self.window layoutIfNeeded];self.tabs=[UITabBarController new];self.tabs.mode=UITabBarControllerModeTabBar;[host addChildViewController:self.tabs];[host.view addSubview:self.tabs.view];[self.tabs didMoveToParentViewController:host];self.tabs.view.autoresizingMask=UIViewAutoresizingNone;self.tabs.view.frame=CGRectMake(0,0,390,self.window.bounds.size.height);dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.5*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[self next];});return;}
 NSUInteger counts[]={2,3,4,5};NSUInteger count=counts[(self.step%8)/2];BOOL last=self.step%2,dark=self.step%16>=8;
 self.window.overrideUserInterfaceStyle=dark?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 if(!last){
  NSArray *titles=@[@"Home",@"Favorites",@"Library",@"Settings",@"Search"],*symbols=@[@"house.fill",@"star.fill",@"books.vertical.fill",@"slider.horizontal.3",@"magnifyingglass"];
  NSMutableArray *controllers=[NSMutableArray new];
  for(NSUInteger i=0;i<count;i++){UIViewController *c=[UIViewController new];c.view.backgroundColor=UIColor.systemGroupedBackgroundColor;c.tabBarItem=[[UITabBarItem alloc] initWithTitle:titles[i] image:[UIImage systemImageNamed:symbols[i]] tag:i];[controllers addObject:c];}
  [self.tabs setViewControllers:controllers animated:NO];
 }
 self.tabs.selectedIndex=last?count-1:0;
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.5*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
  [self.window layoutIfNeeded];NSMutableArray *rows=[NSMutableArray new];[self collect:self.tabs.tabBar rows:rows];
  UIEdgeInsets safe=self.window.safeAreaInsets,child=self.tabs.selectedViewController.view.safeAreaInsets;
  NSString *name=[NSString stringWithFormat:@"%.0f-%lu-%@-%@",self.tabs.view.bounds.size.width,(unsigned long)count,last?@"last":@"first",dark?@"dark":@"light"];
  [self.records addObject:@{@"name":name,@"count":@(count),@"selected":@(self.tabs.selectedIndex),@"dark":@(dark),@"bounds":RectValues(self.window.bounds),@"viewport":RectValues(self.tabs.view.bounds),@"safe":@[@(safe.top),@(safe.left),@(safe.bottom),@(safe.right)],@"childSafe":@[@(child.top),@(child.left),@(child.bottom),@(child.right)],@"views":rows}];
  UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];format.scale=2;
  UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithSize:self.window.bounds.size format:format] imageWithActions:^(UIGraphicsImageRendererContext *context){[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
  [UIImagePNGRepresentation(image) writeToURL:[[self directory] URLByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
  self.step++;[self next];
 });
}
@end
@interface TabScene:UIResponder<UIWindowSceneDelegate>
@property UIWindow *window;
@property TabProbe *probe;
@end
@implementation TabScene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];self.probe=[TabProbe new];self.probe.window=self.window;[self.probe start];}
@end
@interface TabApp:UIResponder<UIApplicationDelegate>
@end
@implementation TabApp
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Tab variants" sessionRole:session.role];c.delegateClass=TabScene.class;return c;}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(TabApp.class));}}
