// Public UIKit live-frame oracle. Class/animation names are read-only observations.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
static NSArray *RectValues(CGRect r){return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
static UIBarButtonItem *Icon(NSString *symbol,NSString *identifier){
 UIImage *image=[UIImage systemImageNamed:symbol withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:20 weight:UIImageSymbolWeightRegular scale:UIImageSymbolScaleMedium]];
 UIBarButtonItem *item=[[UIBarButtonItem alloc]initWithImage:image style:UIBarButtonItemStylePlain target:nil action:NULL];item.identifier=identifier;return item;
}
static NSArray *Items(NSString *kind){
 UIBarButtonItem *a=Icon([kind hasPrefix:@"trash"]?@"trash":@"plus",[kind isEqual:@"trash-new"]?@"replacement":@"a"),*b=Icon(@"ellipsis",@"b");
 if([kind isEqual:@"single"]||[kind hasPrefix:@"trash"])return @[a];
 if([kind isEqual:@"accent"])a.style=UIBarButtonItemStyleProminent;
 if([kind isEqual:@"split"])return @[a,[UIBarButtonItem fixedSpaceItemOfWidth:0],b];
 if([kind isEqual:@"three"])return @[a,b,Icon(@"trash",@"c")];
 if([kind isEqual:@"text"]){a=[[UIBarButtonItem alloc]initWithTitle:@"Edit" style:UIBarButtonItemStylePlain target:nil action:NULL];a.identifier=@"a";b=[[UIBarButtonItem alloc]initWithTitle:@"Save" style:UIBarButtonItemStylePlain target:nil action:NULL];b.identifier=@"b";}
 return @[a,b];
}
@interface MotionProbe:NSObject
@property UIWindow *window;@property UINavigationController *nav;
@property CADisplayLink *link;@property NSMutableArray *clips,*frames;@property NSArray *cases;
@property NSUInteger index;@property NSString *name;@property CFTimeInterval origin,lastPicture;
@property dispatch_queue_t writer;@property dispatch_group_t pending;
@end
@implementation MotionProbe
- (NSURL*)directory{return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;}
- (void)views:(UIView*)view path:(NSString*)path rows:(NSMutableArray*)rows {
 if(view.hidden||view.alpha<.001)return;CALayer *layer=view.layer.presentationLayer?:view.layer,*bar=self.nav.navigationBar.layer.presentationLayer?:self.nav.navigationBar.layer;
 NSString *name=NSStringFromClass(view.class);
 if([name containsString:@"Glass"]||[name containsString:@"ModernBarButton"]||[name containsString:@"ButtonLabel"]||[name containsString:@"ItemWrapperView"]){
  NSMutableDictionary *row=[@{@"id":[NSString stringWithFormat:@"%p",view],@"path":path,@"class":name,@"frame":RectValues([view convertRect:view.bounds toView:self.nav.navigationBar]),@"presented":RectValues([layer convertRect:layer.bounds toLayer:bar]),@"alpha":@(layer.opacity)}mutableCopy];
  if([view isKindOfClass:UILabel.class])row[@"text"]=((UILabel*)view).text?:@"";
  [rows addObject:row];
 }
 NSUInteger i=0;for(UIView *child in view.subviews)[self views:child path:[path stringByAppendingFormat:@"/%lu",(unsigned long)i++] rows:rows];
}
- (void)layers:(CALayer*)layer path:(NSString*)path rows:(NSMutableArray*)rows {
 CALayer *p=layer.presentationLayer?:layer;NSArray *keys=layer.animationKeys;
 if(keys.count||layer.filters.count){
  NSMutableArray *animations=[NSMutableArray new];for(NSString *key in keys){CAAnimation *a=[layer animationForKey:key];NSMutableDictionary *r=[@{@"key":key,@"duration":@(a.duration),@"begin":@(a.beginTime)}mutableCopy];if([a isKindOfClass:CABasicAnimation.class]){r[@"from"]=[((CABasicAnimation*)a).fromValue description]?:@"";r[@"to"]=[((CABasicAnimation*)a).toValue description]?:@"";}[animations addObject:r];}
  [rows addObject:@{@"path":path,@"class":NSStringFromClass(layer.class),@"bounds":RectValues(p.bounds),@"opacity":@(p.opacity),@"filters":[layer.filters description]?:@"",@"animations":animations}];
 }
 NSUInteger i=0;for(CALayer *child in layer.sublayers)[self layers:child path:[path stringByAppendingFormat:@"/%lu",(unsigned long)i++] rows:rows];
}
- (void)capture:(CFTimeInterval)t {
 NSMutableArray *views=[NSMutableArray new],*layers=[NSMutableArray new];[self views:self.nav.navigationBar path:@"bar" rows:views];[self layers:self.nav.navigationBar.layer path:@"bar" rows:layers];
 NSString *file=[NSString stringWithFormat:@"%@/frame-%03lu.png",self.name,(unsigned long)self.frames.count];
 CGRect region=CGRectMake(0,self.nav.navigationBar.frame.origin.y-12,self.window.bounds.size.width,80);
 UIGraphicsImageRendererFormat *format=UIGraphicsImageRendererFormat.defaultFormat;format.scale=3;
 UIImage *image=[[[UIGraphicsImageRenderer alloc]initWithSize:region.size format:format]imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,0,-region.origin.y);[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
 [self.frames addObject:@{@"t":@(t),@"image":file,@"views":views,@"layers":layers}];
 NSURL *url=[[self directory]URLByAppendingPathComponent:file];dispatch_group_async(self.pending,self.writer,^{[UIImagePNGRepresentation(image)writeToURL:url atomically:YES];});
}
- (void)tick:(CADisplayLink*)link {
 if(!self.name)return;const CFTimeInterval t=CACurrentMediaTime()-self.origin;
 if(t-self.lastPicture>=1./65.){self.lastPicture=t;[self capture:t];}
 if(t>=1.05){self.name=nil;self.index++;[self later:.25 run:^{[self prepare];}];}
}
- (void)later:(double)t run:(void(^)(void))run{dispatch_after(dispatch_time(DISPATCH_TIME_NOW,t*NSEC_PER_SEC),dispatch_get_main_queue(),run);}
- (void)prepare {
 if(self.index>=self.cases.count){[self.link invalidate];dispatch_group_notify(self.pending,dispatch_get_main_queue(),^{NSDictionary *result=@{@"os":UIDevice.currentDevice.systemVersion,@"scale":@3,@"imageBarOriginY":@12,@"clips":self.clips};[[NSJSONSerialization dataWithJSONObject:result options:0 error:nil]writeToURL:[[self directory]URLByAppendingPathComponent:@"motion.json"]atomically:YES];});return;}
 NSDictionary *c=self.cases[self.index];self.window.overrideUserInterfaceStyle=[c[@"dark"]boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
 UIViewController *(^screen)(NSString*)=^UIViewController*(NSString *kind){UIViewController *v=[UIViewController new];v.view.backgroundColor=[c[@"dark"]boolValue]?UIColor.blackColor:UIColor.systemGroupedBackgroundColor;v.navigationItem.rightBarButtonItems=Items(kind);return v;};
 UIViewController *source=screen(c[@"from"]),*target=screen(c[@"to"]);
 self.nav=[[UINavigationController alloc]initWithRootViewController:[c[@"pop"]boolValue]?target:source];self.window.rootViewController=self.nav;
 if([c[@"pop"]boolValue])[self.nav pushViewController:source animated:NO];[self.window layoutIfNeeded];
 [self later:.65 run:^{
  self.frames=[NSMutableArray new];NSString *name=[c[@"name"]stringByAppendingString:[c[@"dark"]boolValue]?@"-dark":@"-light"];
  [NSFileManager.defaultManager createDirectoryAtURL:[[self directory]URLByAppendingPathComponent:name]withIntermediateDirectories:YES attributes:nil error:nil];
  [self.clips addObject:@{@"name":name,@"from":c[@"from"],@"to":c[@"to"],@"pop":c[@"pop"],@"frames":self.frames}];self.name=name;self.origin=CACurrentMediaTime();self.lastPicture=-1;[self capture:0];
  if([c[@"pop"]boolValue])[self.nav popViewControllerAnimated:YES];else [self.nav pushViewController:target animated:YES];
 }];
}
- (void)start {
 self.writer=dispatch_queue_create("native.motion.png",DISPATCH_QUEUE_SERIAL);self.pending=dispatch_group_create();self.clips=[NSMutableArray new];
 NSArray *specs=@[@[@"split",@"pair",@"split",@NO],@[@"join",@"split",@"pair",@NO],@[@"grow",@"single",@"pair",@NO],@[@"shrink",@"pair",@"single",@NO],@[@"replace",@"single",@"trash",@NO],@[@"replace-unmatched",@"single",@"trash-new",@NO],@[@"text",@"pair",@"text",@NO],@[@"accent",@"pair",@"accent",@NO],@[@"pop-split",@"pair",@"split",@YES],@[@"pop-join",@"split",@"pair",@YES]];
 NSMutableArray *cases=[NSMutableArray new];NSArray *args=NSProcessInfo.processInfo.arguments;NSUInteger selection=[args indexOfObject:@"--case"];
 for(NSNumber *dark in @[@NO,@YES])for(NSArray *s in specs){if(selection!=NSNotFound&&![s[0]isEqual:args[selection+1]])continue;[cases addObject:@{@"name":s[0],@"from":s[1],@"to":s[2],@"pop":s[3],@"dark":dark}];}self.cases=cases;
 self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 // The first native push can stall >300 ms during framework/material setup.
 // Exercise a real push/pop before measuring steady-state animation; retain
 // the cold-start capture separately rather than time-aligning it away.
 UIViewController *warm=[UIViewController new];warm.navigationItem.rightBarButtonItems=Items(@"pair");self.nav=[[UINavigationController alloc]initWithRootViewController:warm];self.window.rootViewController=self.nav;[self.window makeKeyAndVisible];
 [self later:.3 run:^{UIViewController *detail=[UIViewController new];detail.navigationItem.rightBarButtonItems=Items(@"split");[self.nav pushViewController:detail animated:YES];[self later:1.0 run:^{[self.nav popViewControllerAnimated:YES];[self later:1.0 run:^{[self prepare];}];}];}];
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>@property UIWindow *window;@property MotionProbe *probe;@end
@implementation Scene
- (void)scene:(UIScene*)s willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)o{self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)s];self.probe=[MotionProbe new];self.probe.window=self.window;[self.probe start];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)a configurationForConnectingSceneSession:(UISceneSession*)s options:(UISceneConnectionOptions*)o{UISceneConfiguration *c=[[UISceneConfiguration alloc]initWithName:@"Motion" sessionRole:s.role];c.delegateClass=Scene.class;return c;}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
