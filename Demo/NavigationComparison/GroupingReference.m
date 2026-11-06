// Public UIKit oracle. Private class names are read-only hierarchy observations.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
static NSArray *R(CGRect r){return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
static NSArray *names;
static UIBarButtonItem *Image(NSString *symbol,NSString *identifier){
 UIImage *image=[UIImage systemImageNamed:symbol];
 if([NSProcessInfo.processInfo.arguments containsObject:@"--explicit-images"])image=[image imageByApplyingSymbolConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:20 weight:UIImageSymbolWeightRegular scale:UIImageSymbolScaleMedium]];
 UIBarButtonItem *i=[[UIBarButtonItem alloc] initWithImage:image style:UIBarButtonItemStylePlain target:nil action:NULL];i.accessibilityIdentifier=identifier;i.identifier=identifier;return i;
}
static NSArray *Items(NSString *name){
 UIBarButtonItem *a=Image(@"plus",@"a"),*b=Image(@"ellipsis",@"b"),*c=Image(@"trash",@"c");
 if([name isEqual:@"single"])return @[a];
 if([name isEqual:@"tinted"])a.tintColor=UIColor.systemPinkColor;
 if([name isEqual:@"prominent"])a.style=UIBarButtonItemStyleProminent;
 if([name isEqual:@"both-prominent"]){a.style=b.style=UIBarButtonItemStyleProminent;}
 if([name isEqual:@"text-pair"]||[name isEqual:@"text-image"]){a=[[UIBarButtonItem alloc]initWithTitle:@"Edit" style:UIBarButtonItemStylePlain target:nil action:NULL];a.accessibilityIdentifier=@"a";a.identifier=@"a";}
 if([name isEqual:@"text-pair"]){b=[[UIBarButtonItem alloc]initWithTitle:@"Save" style:UIBarButtonItemStylePlain target:nil action:NULL];b.accessibilityIdentifier=@"b";b.identifier=@"b";}
 if([name hasPrefix:@"fixed-"])return @[a,[UIBarButtonItem fixedSpaceItemOfWidth:[[name substringFromIndex:6] doubleValue]],b];
 if([name isEqual:@"flexible"])return @[a,[UIBarButtonItem flexibleSpaceItem],b];
 if([name isEqual:@"unshared"])a.sharesBackground=NO;
 if([name isEqual:@"hidden"])a.hidesSharedBackground=YES;
 if([name isEqual:@"disabled"])a.enabled=NO;
 if([name isEqual:@"custom"]){UIButton *v=[UIButton buttonWithType:UIButtonTypeSystem];[v setTitle:@"Menu" forState:UIControlStateNormal];v.frame=CGRectMake(0,0,60,32);a=[[UIBarButtonItem alloc]initWithCustomView:v];a.accessibilityIdentifier=@"a";a.identifier=@"a";}
 if([name isEqual:@"three"])return @[a,b,c];
 if([name isEqual:@"middle-prominent"]){b.style=UIBarButtonItemStyleProminent;return @[a,b,c];}
 if([name isEqual:@"middle-hidden"]){b.hidesSharedBackground=YES;return @[a,b,c];}
 if([name isEqual:@"done"]){a=[[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone primaryAction:nil];a.accessibilityIdentifier=@"a";a.identifier=@"a";}
 return @[a,b];
}
@interface Probe:NSObject
@property UIWindow *window;
@property UINavigationController *nav;
@property NSMutableArray *states,*frames;
@property NSString *operation;
@property CADisplayLink *link;
@property CFTimeInterval origin;
@property UILabel *status;
@property NSInteger actions;
@end
@implementation Probe
- (NSURL*)directory{return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;}
- (void)collect:(UIView*)v path:(NSString*)path rows:(NSMutableArray*)rows {
 if(v.hidden||v.alpha<.001)return;CALayer *p=v.layer.presentationLayer?:v.layer,*bar=self.nav.navigationBar.layer.presentationLayer?:self.nav.navigationBar.layer;
 NSMutableDictionary *row=[@{@"id":[NSString stringWithFormat:@"%p",v],@"path":path,@"class":NSStringFromClass(v.class),@"frame":R([v convertRect:v.bounds toView:self.nav.navigationBar]),@"presentation":R([p convertRect:p.bounds toLayer:bar]),@"alpha":@(p.opacity),@"identifier":v.accessibilityIdentifier?:@""}mutableCopy];
 if([v isKindOfClass:UILabel.class])row[@"text"]=((UILabel*)v).text?:@"";
 [rows addObject:row];NSUInteger n=0;for(UIView *child in v.subviews)[self collect:child path:[path stringByAppendingFormat:@"/%lu",(unsigned long)n++] rows:rows];
}
- (NSArray*)views{NSMutableArray *rows=[NSMutableArray new];[self collect:self.nav.navigationBar path:@"bar" rows:rows];return rows;}
- (void)tick:(CADisplayLink*)link{if(self.operation)[self.frames addObject:@{@"operation":self.operation,@"t":@(CACurrentMediaTime()-self.origin),@"views":[self views]}];}
- (void)picture:(NSString*)name{
 UIGraphicsImageRendererFormat *f=UIGraphicsImageRendererFormat.defaultFormat;f.scale=3;CGRect region=CGRectMake(0,self.nav.navigationBar.frame.origin.y,self.window.bounds.size.width,64);
 UIImage *im=[[[UIGraphicsImageRenderer alloc]initWithSize:region.size format:f]imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,0,-region.origin.y);[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
 [UIImagePNGRepresentation(im)writeToURL:[[self directory]URLByAppendingPathComponent:[name stringByAppendingString:@".png"]]atomically:YES];
}
- (void)later:(double)t run:(void(^)(void))run{dispatch_after(dispatch_time(DISPATCH_TIME_NOW,t*NSEC_PER_SEC),dispatch_get_main_queue(),run);}
- (void)cases:(NSUInteger)n{
 if(n>=names.count*4){self.nav.topViewController.navigationItem.leftBarButtonItems=nil;self.nav.topViewController.navigationItem.rightBarButtonItems=Items(@"pair");[self later:.5 run:^{[self transition:0];}];return;}
 NSString *name=names[n%names.count];BOOL dark=(n/names.count)%2,left=(n/names.count)/2;
 self.operation=nil;self.window.overrideUserInterfaceStyle=dark?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 UIViewController *v=self.nav.topViewController;v.navigationItem.leftBarButtonItems=nil;v.navigationItem.rightBarButtonItems=nil;
 if(left)v.navigationItem.leftBarButtonItems=Items(name);else v.navigationItem.rightBarButtonItems=Items(name);
 [self.window layoutIfNeeded];
 [self later:.36 run:^{NSString *label=[NSString stringWithFormat:@"%@-%@-%@",left?@"left":@"right",dark?@"dark":@"light",name];[self.states addObject:@{@"name":label,@"views":[self views]}];[self picture:label];[self cases:n+1];}];
}
- (void)transition:(NSUInteger)n{
 NSArray *targets=@[@"single",@"pair",@"prominent",@"fixed-0",@"pair",@"three",@"middle-prominent",@"pair",@"text-pair",@"pair"];
 if(n>=targets.count*2){[self.link invalidate];NSDictionary *result=@{@"os":UIDevice.currentDevice.systemVersion,@"states":self.states,@"frames":self.frames};[[NSJSONSerialization dataWithJSONObject:result options:0 error:nil]writeToURL:[[self directory]URLByAppendingPathComponent:@"grouping.json"]atomically:YES];return;}
 BOOL pop=n%2;NSString *name=targets[n/2];
 self.operation=[NSString stringWithFormat:@"%@-%@",pop?@"pop":@"push",name];self.origin=CACurrentMediaTime();
 if(pop)[self.nav popViewControllerAnimated:YES];else {UIViewController *v=[UIViewController new];v.title=@"Detail";v.view.backgroundColor=UIColor.systemGroupedBackgroundColor;v.navigationItem.rightBarButtonItems=Items(name);[self.nav pushViewController:v animated:YES];}
 NSString *op=self.operation;[self later:.15 run:^{[self picture:[op stringByAppendingString:@"-mid"]];}];
 [self later:1.05 run:^{[self.states addObject:@{@"name":op,@"views":[self views]}];[self picture:[op stringByAppendingString:@"-end"]];[self transition:n+1];}];
}
- (void)start{
 names=@[@"single",@"pair",@"tinted",@"prominent",@"both-prominent",@"text-pair",@"text-image",@"fixed-0",@"fixed-12",@"fixed-30",@"flexible",@"unshared",@"hidden",@"disabled",@"custom",@"three",@"middle-prominent",@"middle-hidden",@"done"];
 self.states=[NSMutableArray new];self.frames=[NSMutableArray new];UIViewController *v=[UIViewController new];v.title=@"Groups";v.view.backgroundColor=UIColor.systemGroupedBackgroundColor;
 self.nav=[[UINavigationController alloc]initWithRootViewController:v];self.window.rootViewController=self.nav;[self.window makeKeyAndVisible];
 self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 NSString *hold=NSProcessInfo.processInfo.environment[@"HOLD_CASE"];
 if(hold){
  self.window.overrideUserInterfaceStyle=UIUserInterfaceStyleDark;v.navigationItem.rightBarButtonItems=Items(hold);
  self.status=[[UILabel alloc]initWithFrame:CGRectMake(40,200,300,50)];self.status.accessibilityIdentifier=@"status";self.status.text=@"ready";[v.view addSubview:self.status];
  __weak Probe *weak=self;for(UIBarButtonItem *item in v.navigationItem.rightBarButtonItems)if(item.image)item.primaryAction=[UIAction actionWithHandler:^(__kindof UIAction *action){weak.actions++;weak.status.text=@"complete";}];
  self.operation=hold;self.origin=CACurrentMediaTime();[self saveHold:hold];return;
 }
 [self later:.5 run:^{[self cases:0];}];
}
- (void)saveHold:(NSString*)name {
 NSDictionary *result=@{@"os":UIDevice.currentDevice.systemVersion,@"actions":@(self.actions),@"frames":self.frames};
 [[NSJSONSerialization dataWithJSONObject:result options:0 error:nil]writeToURL:[[self directory]URLByAppendingPathComponent:[@"hold-" stringByAppendingFormat:@"%@.json",name]]atomically:YES];
 if(CACurrentMediaTime()-self.origin<12)[self later:.5 run:^{[self saveHold:name];}];
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>
@property UIWindow *window;@property Probe *probe;
@end
@implementation Scene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options{self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)scene];self.probe=[Probe new];self.probe.window=self.window;[self.probe start];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)a configurationForConnectingSceneSession:(UISceneSession*)s options:(UISceneConnectionOptions*)o{UISceneConfiguration *c=[[UISceneConfiguration alloc]initWithName:@"Grouping" sessionRole:s.role];c.delegateClass=Scene.class;return c;}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
