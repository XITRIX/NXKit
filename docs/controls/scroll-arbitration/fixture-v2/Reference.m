// Native-only observer: actual OS touches; no recognizer policies are replaced.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/message.h>
static NSMutableArray *Events,*Frames;
static double Origin;
static NSArray *P(CGPoint p){return @[@(p.x),@(p.y)];}
static NSArray *R(CGRect r){return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
static void Log(NSString *kind,NSDictionary *data){NSMutableDictionary *r=[data mutableCopy];r[@"kind"]=kind;r[@"t"]=@(CACurrentMediaTime());[Events addObject:r];}
@interface ProbeWindow:UIWindow @end
@implementation ProbeWindow
- (void)sendEvent:(UIEvent *)event{for(UITouch *t in event.allTouches)if(t.phase!=UITouchPhaseStationary)Log(@"window-before",@{@"phase":@(t.phase),@"timestamp":@(t.timestamp),@"point":P([t locationInView:self]),@"view":NSStringFromClass(t.view.class)?:@"nil"});[super sendEvent:event];for(UITouch *t in event.allTouches)if(t.phase!=UITouchPhaseStationary)Log(@"window-after",@{@"phase":@(t.phase),@"point":P([t locationInView:self])});}
@end
@interface ProbeScroll:UIScrollView @end
@implementation ProbeScroll
- (BOOL)touchesShouldBegin:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event inContentView:(UIView *)view{BOOL answer=[super touchesShouldBegin:touches withEvent:event inContentView:view];Log(@"scroll-touchesShouldBegin",@{@"answer":@(answer),@"view":NSStringFromClass(view.class)});return answer;}
- (BOOL)touchesShouldCancelInContentView:(UIView *)view{BOOL answer=[super touchesShouldCancelInContentView:view];Log(@"scroll-touchesShouldCancel",@{@"answer":@(answer),@"view":NSStringFromClass(view.class)});return answer;}
@end
#define TRACKING_IMPL \
- (BOOL)beginTrackingWithTouch:(UITouch *)t withEvent:(UIEvent *)e{BOOL b=[super beginTrackingWithTouch:t withEvent:e];Log(@"beginTracking",@{@"answer":@(b),@"point":P([t locationInView:self]),@"tracking":@(self.tracking)});return b;} \
- (BOOL)continueTrackingWithTouch:(UITouch *)t withEvent:(UIEvent *)e{BOOL b=[super continueTrackingWithTouch:t withEvent:e];Log(@"continueTracking",@{@"answer":@(b),@"point":P([t locationInView:self]),@"tracking":@(self.tracking)});return b;} \
- (void)endTrackingWithTouch:(UITouch *)t withEvent:(UIEvent *)e{[super endTrackingWithTouch:t withEvent:e];Log(@"endTracking",@{@"point":t?P([t locationInView:self]):@[]});} \
- (void)cancelTrackingWithEvent:(UIEvent *)e{[super cancelTrackingWithEvent:e];Log(@"cancelTracking",@{});} \
- (void)touchesBegan:(NSSet<UITouch *> *)ts withEvent:(UIEvent *)e{Log(@"control-touchesBegan",@{@"touchTimestamp":@(ts.anyObject.timestamp)});[super touchesBegan:ts withEvent:e];} \
- (void)touchesMoved:(NSSet<UITouch *> *)ts withEvent:(UIEvent *)e{Log(@"control-touchesMoved",@{@"touchTimestamp":@(ts.anyObject.timestamp)});[super touchesMoved:ts withEvent:e];} \
- (void)touchesEnded:(NSSet<UITouch *> *)ts withEvent:(UIEvent *)e{Log(@"control-touchesEnded",@{@"touchTimestamp":@(ts.anyObject.timestamp)});[super touchesEnded:ts withEvent:e];} \
- (void)touchesCancelled:(NSSet<UITouch *> *)ts withEvent:(UIEvent *)e{Log(@"control-touchesCancelled",@{});[super touchesCancelled:ts withEvent:e];}
@interface ProbeSlider:UISlider @end
@implementation ProbeSlider
TRACKING_IMPL
@end
@interface ProbeSwitch:UISwitch @end
@implementation ProbeSwitch
TRACKING_IMPL
@end
@interface Controller:UIViewController<UIScrollViewDelegate>
@property NSDictionary *config;
@property UIControl *control;
@property ProbeScroll *scroll;
@property UILabel *status;
@property CADisplayLink *link;
@property NSMutableSet *observed;
@property BOOL finished;
@end
@implementation Controller
- (void)viewDidLoad{[super viewDidLoad];Origin=CACurrentMediaTime();Events=[NSMutableArray new];Frames=[NSMutableArray new];self.observed=[NSMutableSet new];NSString *raw=NSProcessInfo.processInfo.environment[@"CONTROL_SCROLL_CASE"];self.config=raw?[NSJSONSerialization JSONObjectWithData:[raw dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil]:@{};self.view.backgroundColor=UIColor.systemBackgroundColor;
self.scroll=[[ProbeScroll alloc] initWithFrame:self.view.bounds];self.scroll.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;self.scroll.contentSize=[self.config[@"horizontalScroll"] boolValue]?CGSizeMake(1400,300):CGSizeMake(self.view.bounds.size.width,1800);self.scroll.delegate=self;self.scroll.contentInsetAdjustmentBehavior=UIScrollViewContentInsetAdjustmentNever;if(self.config[@"delays"])self.scroll.delaysContentTouches=[self.config[@"delays"] boolValue];if(self.config[@"canCancel"])self.scroll.canCancelContentTouches=[self.config[@"canCancel"] boolValue];if(![self.config[@"outside"] boolValue])[self.view addSubview:self.scroll];
self.control=[self.config[@"control"] isEqual:@"switch"]?[[ProbeSwitch alloc] initWithFrame:CGRectMake(80,250,63,28)]:[[ProbeSlider alloc] initWithFrame:CGRectMake(50,250,300,50)];if([self.control isKindOfClass:UISlider.class])[(UISlider *)self.control setValue:.5];self.control.accessibilityIdentifier=@"control";[([self.config[@"outside"] boolValue]?self.view:self.scroll) addSubview:self.control];
for(NSNumber *event in @[@(UIControlEventTouchDown),@(UIControlEventTouchDragInside),@(UIControlEventTouchDragOutside),@(UIControlEventTouchUpInside),@(UIControlEventTouchUpOutside),@(UIControlEventTouchCancel),@(UIControlEventValueChanged)]){__weak Controller *weak=self;[self.control addAction:[UIAction actionWithHandler:^(__kindof UIAction *a){Controller *s=weak;Log(@"action",@{@"event":event,@"tracking":@(s.control.tracking),@"highlighted":@(s.control.highlighted),@"value":@([s value])});}] forControlEvents:event.unsignedLongLongValue];}
self.status=[[UILabel alloc] initWithFrame:CGRectMake(40,100,300,40)];self.status.accessibilityIdentifier=@"status";self.status.text=@"ready";[self.view addSubview:self.status];self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(sample:)];self.link.preferredFramesPerSecond=120;[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];__weak Controller *weak=self;[NSTimer scheduledTimerWithTimeInterval:.15 repeats:YES block:^(NSTimer *t){[weak save];}];}
- (double)value{return [self.control isKindOfClass:UISlider.class]?[(UISlider *)self.control value]:[(UISwitch *)self.control isOn];}
- (void)grChanged:(UIGestureRecognizer *)g{Log(@"recognizer-action",@{@"class":NSStringFromClass(g.class),@"state":@(g.state)});}
- (void)sample:(CADisplayLink *)link{NSMutableArray *queue=[NSMutableArray arrayWithObjects:self.view,nil],*gs=[NSMutableArray new],*lenses=[NSMutableArray new];while(queue.count){UIView *v=queue.firstObject;[queue removeObjectAtIndex:0];[queue addObjectsFromArray:v.subviews];if([NSStringFromClass(v.class) isEqual:@"_UILiquidLensView"]){CALayer *p=v.layer.presentationLayer?:v.layer;NSMutableDictionary *row=[@{@"class":NSStringFromClass(v.class),@"bounds":R(v.bounds),@"presentationBounds":R(p.bounds),@"frame":R(v.frame)} mutableCopy];for(NSString *name in @[@"isLifted",@"lifted"]){SEL sel=NSSelectorFromString(name);if([v respondsToSelector:sel]){row[@"lifted"]=@(((BOOL(*)(id,SEL))objc_msgSend)(v,sel));break;}}[lenses addObject:row];}for(UIGestureRecognizer *g in v.gestureRecognizers){NSString *identity=[NSString stringWithFormat:@"%p",g];if(![self.observed containsObject:identity]){[self.observed addObject:identity];[g addTarget:self action:@selector(grChanged:)];Log(@"recognizer-inventory",@{@"id":identity,@"class":NSStringFromClass(g.class),@"view":NSStringFromClass(v.class),@"delegate":g.delegate?NSStringFromClass([(id)g.delegate class]):@"nil",@"description":g.description});}NSMutableDictionary *r=[@{@"id":identity,@"class":NSStringFromClass(g.class),@"state":@(g.state),@"view":NSStringFromClass(v.class),@"delaysTouchesBegan":@(g.delaysTouchesBegan),@"delaysTouchesEnded":@(g.delaysTouchesEnded),@"cancelsTouchesInView":@(g.cancelsTouchesInView)} mutableCopy];if([g isKindOfClass:UILongPressGestureRecognizer.class]){r[@"minimumPressDuration"]=@([(UILongPressGestureRecognizer *)g minimumPressDuration]);r[@"allowableMovement"]=@([(UILongPressGestureRecognizer *)g allowableMovement]);}if([g isKindOfClass:UIPanGestureRecognizer.class]){r[@"translation"]=P([(UIPanGestureRecognizer *)g translationInView:self.view]);r[@"velocity"]=P([(UIPanGestureRecognizer *)g velocityInView:self.view]);}[gs addObject:r];}}
[Frames addObject:@{@"t":@(CACurrentMediaTime()),@"value":@([self value]),@"tracking":@(self.control.tracking),@"highlighted":@(self.control.highlighted),@"scrollOffset":P(self.scroll.contentOffset),@"gestures":gs,@"lenses":lenses}];}
- (void)scrollViewWillBeginDragging:(UIScrollView *)s{Log(@"scroll-willBeginDragging",@{@"offset":P(s.contentOffset)});}
- (void)scrollViewDidEndDragging:(UIScrollView *)s willDecelerate:(BOOL)b{Log(@"scroll-didEndDragging",@{@"offset":P(s.contentOffset),@"decelerate":@(b)});}
- (void)save{NSString *directory=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:self.config[@"id"]?:@"default"];[[NSFileManager defaultManager] createDirectoryAtPath:directory withIntermediateDirectories:YES attributes:nil error:nil];NSDictionary *d=@{@"config":self.config,@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"origin":@(Origin),@"controlFrame":R([self.control convertRect:self.control.bounds toView:self.view]),@"delaysContentTouches":@(self.scroll.delaysContentTouches),@"canCancelContentTouches":@(self.scroll.canCancelContentTouches),@"events":Events,@"frames":Frames,@"scope":@"Actual XCUITest OS touch paths; only observation targets added to native recognizers. Public scroll/control overrides forward to super. No delegate replacement or failure relationship mutation."};[[NSJSONSerialization dataWithJSONObject:d options:0 error:nil] writeToFile:[directory stringByAppendingPathComponent:@"trace.json"] atomically:YES];}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>@property UIWindow *window;@end
@implementation Scene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options{self.window=[[ProbeWindow alloc] initWithWindowScene:(id)scene];self.window.rootViewController=[Controller new];[self.window makeKeyAndVisible];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration *)application:(UIApplication *)a configurationForConnectingSceneSession:(UISceneSession *)s options:(UISceneConnectionOptions *)o{UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"probe" sessionRole:s.role];c.delegateClass=Scene.class;return c;}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
