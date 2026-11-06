#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
@class ScrollOracle;
@interface TraceWindow:UIWindow
@property(nonatomic,weak) ScrollOracle *oracle;
@end
@interface ScrollOracle:UIViewController<UIScrollViewDelegate>
@property NSDictionary *configuration;
@property UIView *target;
@property UIScrollView *scroll;
@property UIPanGestureRecognizer *pan;
@property UILabel *status;
@property CADisplayLink *link;
@property NSMutableArray *events,*frames,*actions,*delegates;
@property NSMapTable *identifiers;
@property double start,lastEnd;
@property NSInteger nextID,active;
@property BOOL complete,resized;
- (void)observe:(UIEvent *)event;
- (void)afterEvent:(UIEvent *)event;
@end
@implementation TraceWindow
- (void)sendEvent:(UIEvent *)event { [self.oracle observe:event];[super sendEvent:event];[self.oracle afterEvent:event]; }
@end
@implementation ScrollOracle
- (void)viewDidLoad {
 [super viewDidLoad];self.view.backgroundColor=UIColor.systemBackgroundColor;
 NSString *json=NSProcessInfo.processInfo.environment[@"SCROLL_CASE"];
 self.configuration=[NSJSONSerialization JSONObjectWithData:[json dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
 self.delegates=[NSMutableArray new];self.events=[NSMutableArray new];self.frames=[NSMutableArray new];self.actions=[NSMutableArray new];self.identifiers=[NSMapTable mapTableWithKeyOptions:NSPointerFunctionsStrongMemory|NSPointerFunctionsObjectPointerPersonality valueOptions:NSPointerFunctionsStrongMemory];
 if([self.configuration[@"target"] isEqual:@"pan"]) {
  self.target=[UIView new];self.target.multipleTouchEnabled=YES;
  self.pan=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panned:)];[self.target addGestureRecognizer:self.pan];
 } else {
  self.scroll=[UIScrollView new];self.scroll.delegate=self;self.scroll.contentInsetAdjustmentBehavior=UIScrollViewContentInsetAdjustmentNever;self.scroll.alwaysBounceVertical=YES;
  self.scroll.contentSize=CGSizeMake(350,1600);self.target=self.scroll;self.pan=self.scroll.panGestureRecognizer;[self.pan addTarget:self action:@selector(panned:)];
  for(int i=0;i<32;i++){UILabel *row=[[UILabel alloc] initWithFrame:CGRectMake(10,i*50,330,49)];row.text=[NSString stringWithFormat:@"Row %d",i];row.backgroundColor=i%2?UIColor.secondarySystemBackgroundColor:UIColor.tertiarySystemBackgroundColor;[self.scroll addSubview:row];}
 }
 self.target.backgroundColor=UIColor.systemTealColor;self.target.accessibilityIdentifier=@"target";[self.view addSubview:self.target];
 self.status=[[UILabel alloc] initWithFrame:CGRectMake(20,35,350,40)];self.status.accessibilityIdentifier=@"status";self.status.text=@"preparing";[self.view addSubview:self.status];
 if(self.configuration[@"minimum"])self.pan.minimumNumberOfTouches=[self.configuration[@"minimum"] unsignedIntegerValue];
 if(self.configuration[@"maximum"])self.pan.maximumNumberOfTouches=[self.configuration[@"maximum"] unsignedIntegerValue];
 self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,.3*NSEC_PER_SEC),dispatch_get_main_queue(),^{self.status.text=@"ready";});
}
- (void)viewDidLayoutSubviews {
 [super viewDidLayoutSubviews];
 self.target.frame=CGRectMake(20,100,MIN(350,self.view.bounds.size.width-40),self.resized?300:MIN(500,self.view.bounds.size.height-140));
 if(!self.start&&self.configuration[@"offset"])self.scroll.contentOffset=CGPointMake(0,[self.configuration[@"offset"] doubleValue]);
}
- (NSArray *)sample:(double)t {
 CGPoint tr=[self.pan translationInView:self.target],v=[self.pan velocityInView:self.target],offset=self.scroll.contentOffset;
 return @[@(t),@(offset.x),@(offset.y),@(tr.x),@(tr.y),@(v.x),@(v.y),@(self.pan.state),@(self.pan.numberOfTouches),@(self.target.bounds.size.width),@(self.target.bounds.size.height),@(self.scroll.dragging),@(self.scroll.decelerating)];
}
- (void)panned:(UIPanGestureRecognizer *)pan {
 if(self.configuration[@"setTranslation"]&&pan.state==UIGestureRecognizerStateBegan){NSArray *p=self.configuration[@"setTranslation"];[pan setTranslation:CGPointMake([p[0] doubleValue],[p[1] doubleValue]) inView:self.target];}
 if(self.start)[self.actions addObject:[self sample:CACurrentMediaTime()-self.start]];
}
- (void)observe:(UIEvent *)event {
 if(event.type!=UIEventTypeTouches||self.complete)return;
 if(!self.start)self.start=event.timestamp;
 NSMutableArray *touches=[NSMutableArray new];
 for(UITouch *touch in event.allTouches){
  NSNumber *key=[self.identifiers objectForKey:touch];if(!key){key=@(++self.nextID);[self.identifiers setObject:key forKey:touch];}
  CGPoint point=[touch locationInView:self.view.window];
  [touches addObject:@[key,@(touch.phase),@(touch.timestamp-self.start),@(point.x),@(point.y)]];
 }
 self.active=0;for(UITouch *touch in event.allTouches)if(touch.phase!=UITouchPhaseEnded&&touch.phase!=UITouchPhaseCancelled)self.active++;
 if(!self.active)self.lastEnd=CACurrentMediaTime();
 [self.events addObject:@{@"t":@(event.timestamp-self.start),@"deliveredAt":@(CACurrentMediaTime()-self.start),@"touches":touches,@"sample":[self sample:event.timestamp-self.start]}];
}
- (void)scrollViewWillBeginDragging:(UIScrollView *)view { if(self.start)[self.delegates addObject:@{@"t":@(CACurrentMediaTime()-self.start),@"name":@"begin"}]; }
- (void)scrollViewWillEndDragging:(UIScrollView *)view withVelocity:(CGPoint)velocity targetContentOffset:(inout CGPoint *)target {
 [self.delegates addObject:@{@"t":@(CACurrentMediaTime()-self.start),@"name":@"willEnd",@"velocity":@[@(velocity.x),@(velocity.y)],@"target":@[@(target->x),@(target->y)]}];
}
- (void)scrollViewDidEndDragging:(UIScrollView *)view willDecelerate:(BOOL)decelerate { [self.delegates addObject:@{@"t":@(CACurrentMediaTime()-self.start),@"name":@"end",@"decelerate":@(decelerate)}]; }
- (void)scrollViewDidEndDecelerating:(UIScrollView *)view { [self.delegates addObject:@{@"t":@(CACurrentMediaTime()-self.start),@"name":@"settled"}]; }
- (void)afterEvent:(UIEvent *)event {
 if(event.type!=UIEventTypeTouches||!self.events.count||self.complete)return;
 NSMutableDictionary *row=[self.events.lastObject mutableCopy];row[@"sample"]=[self sample:event.timestamp-self.start];self.events[self.events.count-1]=row;
}
- (void)tick:(CADisplayLink *)link {
 if(!self.start||self.complete)return;
 double t=CACurrentMediaTime()-self.start;
 if(self.configuration[@"resizeAt"]&&!self.resized&&t>=[self.configuration[@"resizeAt"] doubleValue]){self.resized=YES;[self.view setNeedsLayout];[self.view layoutIfNeeded];}
 [self.frames addObject:[self sample:t]];
 double duration=self.configuration[@"duration"]?[self.configuration[@"duration"] doubleValue]:4;
 if(t<duration)return;
 self.complete=YES;[self.link invalidate];
 NSString *directory=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES) firstObject];
 NSString *path=[directory stringByAppendingPathComponent:[self.configuration[@"id"] stringByAppendingString:@".json"]];
 CGRect frame=[self.target convertRect:self.target.bounds toView:self.view.window];
 NSDictionary *trace=@{@"configuration":self.configuration,@"os":UIDevice.currentDevice.systemVersion,@"scale":@(self.view.window.screen.scale),@"minimum":@(self.pan.minimumNumberOfTouches),@"maximum":@(self.pan.maximumNumberOfTouches),@"stage":@[@(frame.origin.x),@(frame.origin.y),@(frame.size.width),@(frame.size.height)],@"columns":@[@"t",@"offsetX",@"offsetY",@"translationX",@"translationY",@"velocityX",@"velocityY",@"state",@"touches",@"width",@"height",@"dragging",@"decelerating"],@"delegates":self.delegates,@"events":self.events,@"frames":self.frames,@"actions":self.actions};
 [[NSJSONSerialization dataWithJSONObject:trace options:0 error:nil] writeToFile:path atomically:YES];self.status.text=@"complete";
}
@end
@interface ScrollScene:UIResponder<UIWindowSceneDelegate>
@property TraceWindow *window;
@end
@implementation ScrollScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
 self.window=[[TraceWindow alloc] initWithWindowScene:(UIWindowScene *)scene];ScrollOracle *oracle=[ScrollOracle new];self.window.oracle=oracle;self.window.rootViewController=oracle;[self.window makeKeyAndVisible];
}
@end
@interface ScrollApp:UIResponder<UIApplicationDelegate>@end
@implementation ScrollApp
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Scroll" sessionRole:session.role];c.delegateClass=ScrollScene.class;return c;}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(ScrollApp.class));}}
