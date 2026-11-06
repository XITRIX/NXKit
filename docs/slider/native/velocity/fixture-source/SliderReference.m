// Native-only UIKit oracle. No private UIKit implementation enters NXKit.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

static NSArray *R(CGRect r) { return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)]; }
static NSArray *S(CGSize s) { return @[@(s.width),@(s.height)]; }
static NSArray *ImageSize(UIImage *image) { return image?S(image.size):@[]; }
static NSArray *Color(UIColor *color,UITraitCollection *traits) { CGFloat r=0,g=0,b=0,a=0;return [[color resolvedColorWithTraitCollection:traits] getRed:&r green:&g blue:&b alpha:&a]?@[@(r),@(g),@(b),@(a)]:@[]; }
static NSDictionary *Tree(UIView *v) {
    NSMutableArray *children=[NSMutableArray new]; for (UIView *c in v.subviews) [children addObject:Tree(c)];
    return @{ @"class":NSStringFromClass(v.class), @"frame":R(v.frame), @"bounds":R(v.bounds), @"alpha":@(v.alpha), @"backgroundColor":Color(v.backgroundColor,v.traitCollection), @"hidden":@(v.hidden), @"children":children };
}
static UIImage *TestImage(CGSize size) { UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithSize:size]; return [r imageWithActions:^(UIGraphicsImageRendererContext *c){ [UIColor.systemPinkColor setFill]; UIRectFill((CGRect){CGPointZero,size}); }]; }
static NSMutableArray *RecordedTouches;
@interface SliderWindow : UIWindow @end
@implementation SliderWindow
- (void)sendEvent:(UIEvent *)event { for (UITouch *touch in event.allTouches) if(touch.phase!=UITouchPhaseStationary) { [RecordedTouches addObject:@{@"t":@(CACurrentMediaTime()),@"timestamp":@(touch.timestamp),@"phase":@(touch.phase),@"point":@[@([touch locationInView:self.rootViewController.view].x),@([touch locationInView:self.rootViewController.view].y)]}]; if(touch.phase==UITouchPhaseBegan) [self.rootViewController performSelector:NSSelectorFromString(@"captureKnobState:") withObject:@"contact" afterDelay:.2];if(touch.phase==UITouchPhaseEnded)[self.rootViewController performSelector:NSSelectorFromString(@"captureKnobState:") withObject:@"released" afterDelay:.8]; }[super sendEvent:event]; }
@end

@interface SliderProbe : UISlider
@property NSMutableArray *events;
@end
@implementation SliderProbe
- (BOOL)beginTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event { float before=self.value; BOOL answer=[super beginTrackingWithTouch:touch withEvent:event]; [self.events addObject:@{@"kind":@"begin",@"point":@[@([touch locationInView:self].x),@([touch locationInView:self].y)],@"before":@(before),@"after":@(self.value),@"accepted":@(answer)}]; return answer; }
- (BOOL)continueTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event { float before=self.value; BOOL answer=[super continueTrackingWithTouch:touch withEvent:event]; [self.events addObject:@{@"kind":@"continue",@"point":@[@([touch locationInView:self].x),@([touch locationInView:self].y)],@"before":@(before),@"after":@(self.value),@"accepted":@(answer)}]; return answer; }
- (void)endTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event { [super endTrackingWithTouch:touch withEvent:event]; [self.events addObject:@{@"kind":@"end",@"value":@(self.value)}]; }
- (void)cancelTrackingWithEvent:(UIEvent *)event { [super cancelTrackingWithEvent:event]; [self.events addObject:@{@"kind":@"cancel",@"value":@(self.value)}]; }
@end

@interface SliderController : UIViewController
@property SliderProbe *slider;
@property UILabel *status;
@property NSMutableArray *events;
@property NSDictionary *configuration;
@property NSMutableArray *frames;
@end
@implementation SliderController
- (NSDictionary *)measure:(UISlider *)s name:(NSString *)name {
    [s layoutIfNeeded]; CGRect track=[s trackRectForBounds:s.bounds];
    return @{ @"id":name,@"bounds":R(s.bounds),@"track":R(track),@"thumbAt0":R([s thumbRectForBounds:s.bounds trackRect:track value:0]),@"thumbAtHalf":R([s thumbRectForBounds:s.bounds trackRect:track value:.5]),@"thumbAt1":R([s thumbRectForBounds:s.bounds trackRect:track value:1]),@"minImageRect":R([s minimumValueImageRectForBounds:s.bounds]),@"maxImageRect":R([s maximumValueImageRectForBounds:s.bounds]),@"sizeThatFits":S([s sizeThatFits:CGSizeMake(300,100)]),@"intrinsic":S(s.intrinsicContentSize),@"value":@(s.value),@"minimum":@(s.minimumValue),@"maximum":@(s.maximumValue),@"continuous":@(s.continuous),@"enabled":@(s.enabled),@"style":@(s.sliderStyle),@"currentThumbImage":ImageSize(s.currentThumbImage),@"thumbImageForNormal":ImageSize([s thumbImageForState:UIControlStateNormal]),@"thumbImageForHighlighted":ImageSize([s thumbImageForState:UIControlStateHighlighted]),@"thumbImageForDisabled":ImageSize([s thumbImageForState:UIControlStateDisabled]),@"currentMinImage":s.currentMinimumTrackImage?S(s.currentMinimumTrackImage.size):@[],@"currentMaxImage":s.currentMaximumTrackImage?S(s.currentMaximumTrackImage.size):@[],@"tree":Tree(s) };
}
- (void)viewDidLoad {
    [super viewDidLoad]; self.view.backgroundColor=UIColor.systemBackgroundColor;
    NSString *raw=NSProcessInfo.processInfo.environment[@"SLIDER_CASE"];
    self.configuration=raw?[NSJSONSerialization JSONObjectWithData:[raw dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil]:@{};
    self.overrideUserInterfaceStyle=[self.configuration[@"dark"] boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
    self.events=[NSMutableArray new];self.frames=[NSMutableArray new];RecordedTouches=[NSMutableArray new]; self.slider=[[SliderProbe alloc] initWithFrame:CGRectMake(80,200,300,50)]; self.slider.events=self.events; self.slider.value=.25;
    if([self.configuration[@"customThumb"] boolValue]) [self.slider setThumbImage:TestImage(CGSizeMake(30,40)) forState:UIControlStateNormal];
    self.slider.accessibilityIdentifier=@"slider"; [self.view addSubview:self.slider];
    NSArray *eventTypes=@[@(UIControlEventTouchDown),@(UIControlEventTouchDragInside),@(UIControlEventTouchDragOutside),@(UIControlEventTouchUpInside),@(UIControlEventTouchUpOutside),@(UIControlEventTouchCancel),@(UIControlEventValueChanged)];
    for(NSNumber *type in eventTypes) { __weak SliderController *weakSelf=self;[self.slider addAction:[UIAction actionWithHandler:^(__kindof UIAction *a){SliderController *owner=weakSelf;[owner.events addObject:@{@"kind":@"action",@"t":@(CACurrentMediaTime()),@"event":type,@"value":@(owner.slider.value),@"tracking":@(owner.slider.tracking),@"highlighted":@(owner.slider.highlighted)}];}] forControlEvents:type.unsignedLongLongValue]; }
    if (self.configuration[@"continuous"]) self.slider.continuous=[self.configuration[@"continuous"] boolValue];
    self.status=[[UILabel alloc] initWithFrame:CGRectMake(50,80,500,40)]; self.status.accessibilityIdentifier=@"status";self.status.text=@"ready";[self.view addSubview:self.status];
    if(!self.configuration[@"x"]) dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC/2),dispatch_get_main_queue(),^{ [self runGeometry]; });
    if([self.configuration[@"snapshots"] boolValue]) dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC/2),dispatch_get_main_queue(),^{ [self captureKnobState:@"rest"]; });
    CADisplayLink *link=[CADisplayLink displayLinkWithTarget:self selector:@selector(sample:)];link.preferredFramesPerSecond=60;[link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    __weak SliderController *weakSelf=self;[NSTimer scheduledTimerWithTimeInterval:.1 repeats:YES block:^(NSTimer *t){[weakSelf saveEvents];}];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,5*NSEC_PER_SEC),dispatch_get_main_queue(),^{ [self saveEvents]; self.status.text=@"complete"; });
}
- (void)sample:(CADisplayLink *)link {
    UIView *lens=nil;NSMutableArray *queue=[NSMutableArray arrayWithObject:self.slider],*gestures=[NSMutableArray new];
    while(queue.count) {UIView *v=queue.firstObject;[queue removeObjectAtIndex:0];if(!lens && [NSStringFromClass(v.class) isEqual:@"_UILiquidLensView"])lens=v;for(UIGestureRecognizer *g in v.gestureRecognizers) {NSMutableDictionary *row=[@{@"class":NSStringFromClass(g.class),@"state":@(g.state)} mutableCopy];if([g isKindOfClass:UIPanGestureRecognizer.class]){CGPoint vel=[(UIPanGestureRecognizer *)g velocityInView:self.slider];row[@"velocity"]=@[@(vel.x),@(vel.y)];} [gestures addObject:row];} [queue addObjectsFromArray:v.subviews];}
    CALayer *p=lens.layer.presentationLayer?:lens.layer;[self.frames addObject:@{@"t":@(CACurrentMediaTime()),@"value":@(self.slider.value),@"tracking":@(self.slider.tracking),@"highlighted":@(self.slider.highlighted),@"gestures":gestures,@"lensFrame":lens?R(lens.frame):@[],@"presentationBounds":p?R(p.bounds):@[],@"presentationTransform":p?@[@(p.transform.m11),@(p.transform.m22),@(p.transform.m41),@(p.transform.m42)]:@[]}];
}
- (void)captureKnobState:(NSString *)name { if(![self.configuration[@"snapshots"] boolValue])return;CGRect rect=CGRectInset(self.slider.frame,-30,-30);double start=CACurrentMediaTime();UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithSize:rect.size];UIImage *image=[r imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,-rect.origin.x,-rect.origin.y);[self.view drawViewHierarchyInRect:self.view.bounds afterScreenUpdates:YES];}];[UIImagePNGRepresentation(image) writeToFile:[[self output] stringByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];[self.events addObject:@{@"kind":@"snapshot",@"name":name,@"start":@(start),@"end":@(CACurrentMediaTime()),@"rectInRoot":R(rect),@"note":@"Snapshot capture perturbs timing; exclude from motion estimation"}]; }
- (void)runGeometry {
    UIControl *control=[UIControl new]; NSMutableArray *controlRows=[NSMutableArray new]; __block NSInteger count=0;
    UIAction *action=[UIAction actionWithTitle:@"probe" image:nil identifier:@"same" handler:^(__kindof UIAction *a){count++;}];
    [control addAction:action forControlEvents:UIControlEventTouchDown|UIControlEventValueChanged];
    [control sendActionsForControlEvents:UIControlEventTouchDown|UIControlEventValueChanged];[controlRows addObject:@{@"case":@"overlap-two-bits",@"count":@(count)}];
    count=0;control.enabled=NO;[control sendActionsForControlEvents:UIControlEventValueChanged];[controlRows addObject:@{@"case":@"disabled-send",@"count":@(count)}];
    count=0;control.enabled=YES;control.userInteractionEnabled=NO;[control sendActionsForControlEvents:UIControlEventValueChanged];[controlRows addObject:@{@"case":@"interaction-disabled-send",@"count":@(count)}];
    count=0;UIAction *replacement=[UIAction actionWithTitle:@"replacement" image:nil identifier:@"same" handler:^(__kindof UIAction *a){count+=10;}];[control addAction:replacement forControlEvents:UIControlEventTouchUpInside];
    [control sendActionsForControlEvents:UIControlEventValueChanged];[controlRows addObject:@{@"case":@"same-id-new-bit-old-dispatch",@"count":@(count)}];
    count=0;[control sendActionsForControlEvents:UIControlEventTouchUpInside];[controlRows addObject:@{@"case":@"same-id-new-bit-new-dispatch",@"count":@(count)}];
    [control removeAction:replacement forControlEvents:UIControlEventTouchUpInside];count=0;[control sendActionsForControlEvents:UIControlEventValueChanged|UIControlEventTouchUpInside];[controlRows addObject:@{@"case":@"partial-remove",@"count":@(count)}];
    [controlRows addObject:@{@"case":@"defaults",@"enabled":@(control.enabled),@"selected":@(control.selected),@"highlighted":@(control.highlighted),@"tracking":@(control.tracking),@"touchInside":@(control.touchInside),@"state":@(control.state)}];
    [[NSJSONSerialization dataWithJSONObject:controlRows options:NSJSONWritingPrettyPrinted error:nil] writeToFile:[[self output] stringByAppendingPathComponent:@"control.json"] atomically:YES];
    NSMutableArray *rows=[NSMutableArray new];
    for (NSNumber *width in @[@100,@200,@300,@440]) for (NSNumber *height in @[@20,@31,@50,@80]) {
        UISlider *s=[[UISlider alloc] initWithFrame:CGRectMake(80,300,width.doubleValue,height.doubleValue)]; [self.view addSubview:s]; [rows addObject:[self measure:s name:[NSString stringWithFormat:@"plain-%@x%@",width,height]]];[s removeFromSuperview];
    }
    NSArray *variants=@[@"default",@"explicit-zero",@"explicit-one",@"disabled",@"highlighted",@"selected",@"thumbless",@"min-image",@"max-image",@"both-images",@"thumb-image",@"thumb-image-highlighted",@"thumb-image-disabled",@"range",@"collapsed-range",@"min-above-max",@"max-below-min",@"ticks",@"ticks-range",@"ticks-neutral",@"tinted"];
    for (NSString *variant in variants) {
        UISlider *s=[[UISlider alloc] initWithFrame:CGRectMake(80,300,300,50)];s.value=.25;
        if ([variant isEqual:@"disabled"]) s.enabled=NO;
        if ([variant isEqual:@"highlighted"]) s.highlighted=YES;
        if ([variant isEqual:@"selected"]) s.selected=YES;
        if ([variant isEqual:@"thumbless"]) s.sliderStyle=UISliderStyleThumbless;
        if ([variant isEqual:@"min-image"] || [variant isEqual:@"both-images"]) s.minimumValueImage=TestImage(CGSizeMake(18,20));
        if ([variant isEqual:@"max-image"] || [variant isEqual:@"both-images"]) s.maximumValueImage=TestImage(CGSizeMake(26,24));
        if ([variant hasPrefix:@"thumb-image"]) [s setThumbImage:TestImage(CGSizeMake(30,40)) forState:UIControlStateNormal];
        if ([variant isEqual:@"thumb-image-highlighted"]) s.highlighted=YES;
        if ([variant isEqual:@"thumb-image-disabled"]) s.enabled=NO;
        if ([variant isEqual:@"explicit-zero"]) s.value=0;
        if ([variant isEqual:@"explicit-one"]) s.value=1;
        if ([variant isEqual:@"range"]) { s.minimumValue=-10; s.maximumValue=10; s.value=0; }
        if ([variant isEqual:@"collapsed-range"]) { s.minimumValue=2; s.maximumValue=2; }
        if ([variant isEqual:@"min-above-max"]) { s.minimumValue=2; }
        if ([variant isEqual:@"max-below-min"]) { s.maximumValue=-2; }
        if ([variant isEqual:@"ticks"]) { s.trackConfiguration=[UISliderTrackConfiguration configurationWithNumberOfTicks:5]; }
        if ([variant isEqual:@"ticks-range"]) {s.minimumValue=-10;s.maximumValue=10;s.value=5;s.trackConfiguration=[UISliderTrackConfiguration configurationWithNumberOfTicks:5];}
        if ([variant isEqual:@"ticks-neutral"]) {s.minimumValue=-10;s.maximumValue=10;s.value=5;UISliderTrackConfiguration *cfg=[UISliderTrackConfiguration configurationWithNumberOfTicks:5];cfg.neutralValue=.5;s.trackConfiguration=cfg;}
        if ([variant isEqual:@"tinted"]) {s.thumbTintColor=UIColor.systemPinkColor;s.minimumTrackTintColor=UIColor.systemGreenColor;s.maximumTrackTintColor=UIColor.systemOrangeColor;}
        [self.view addSubview:s]; [rows addObject:[self measure:s name:variant]];
        s.hidden=YES;
        double delay=.1+[variants indexOfObject:variant]*.2;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(delay*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
            s.hidden=NO;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.1*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
                UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(340,110)];UIImage *image=[renderer imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,-60,-270);[self.view drawViewHierarchyInRect:self.view.bounds afterScreenUpdates:YES];}];
                [UIImagePNGRepresentation(image) writeToFile:[[self output] stringByAppendingPathComponent:[variant stringByAppendingString:@".png"]] atomically:YES];[s removeFromSuperview];
            });
        });
    }
    UISliderTrackConfiguration *tickConfiguration=[UISliderTrackConfiguration configurationWithNumberOfTicks:5];NSMutableArray *positions=[NSMutableArray new];for(UISliderTick *tick in tickConfiguration.ticks)[positions addObject:@(tick.position)];
    NSDictionary *tickDefaults=@{@"allowsTickValuesOnly":@(tickConfiguration.allowsTickValuesOnly),@"neutralValue":@(tickConfiguration.neutralValue),@"minimumEnabledValue":@(tickConfiguration.minimumEnabledValue),@"maximumEnabledValue":@(tickConfiguration.maximumEnabledValue),@"tickPositions":positions};
    NSDictionary *result=@{@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"scale":@(self.view.window.screen.scale),@"configuration":self.configuration,@"cases":rows,@"tickDefaults":tickDefaults};
    [[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil] writeToFile:[[self output] stringByAppendingPathComponent:@"geometry.json"] atomically:YES];
}
- (NSString *)output { NSString *dir=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:self.configuration[@"id"]?:@"default"]; [[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil]; return dir; }
- (void)saveEvents { [[NSJSONSerialization dataWithJSONObject:self.events options:NSJSONWritingPrettyPrinted error:nil] writeToFile:[[self output] stringByAppendingPathComponent:@"events.json"] atomically:YES];[[NSJSONSerialization dataWithJSONObject:self.frames options:0 error:nil] writeToFile:[[self output] stringByAppendingPathComponent:@"frames.json"] atomically:YES];[[NSJSONSerialization dataWithJSONObject:RecordedTouches options:0 error:nil] writeToFile:[[self output] stringByAppendingPathComponent:@"touches.json"] atomically:YES]; }
@end
@interface SliderScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation SliderScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options { self.window=[[SliderWindow alloc] initWithWindowScene:(UIWindowScene *)scene];self.window.rootViewController=[SliderController new];[self.window makeKeyAndVisible]; }
@end
@interface AppDelegate : UIResponder <UIApplicationDelegate> @end
@implementation AppDelegate
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options { UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Slider" sessionRole:session.role];c.delegateClass=SliderScene.class;return c; }
@end
int main(int argc,char **argv) { @autoreleasepool { return UIApplicationMain(argc,argv,nil,NSStringFromClass(AppDelegate.class)); } }
