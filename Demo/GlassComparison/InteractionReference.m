// Native-only measurement app. XCTest supplies real, public touch gestures;
// logging observes UIKit's live presentation layers without changing them.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

static id FiniteJSON(id value) {
    if ([value isKindOfClass:NSNumber.class]) return isfinite([value doubleValue]) ? value : NSNull.null;
    if ([value isKindOfClass:NSArray.class]) {
        NSMutableArray *result=[NSMutableArray new]; for (id item in value) [result addObject:FiniteJSON(item)]; return result;
    }
    if ([value isKindOfClass:NSDictionary.class]) {
        NSMutableDictionary *result=[NSMutableDictionary new]; for (id key in value) result[key]=FiniteJSON(value[key]); return result;
    }
    return value;
}

@interface TouchWindow : UIWindow
@property(nonatomic,copy) void (^observe)(UITouch *touch);
@end
@implementation TouchWindow
- (void)sendEvent:(UIEvent *)event {
    for (UITouch *touch in event.allTouches) if (self.observe) self.observe(touch);
    [super sendEvent:event];
}
@end

@interface InteractionController : UIViewController
@property(nonatomic,strong) UIView *stage;
@property(nonatomic,strong) UILabel *status;
@property(nonatomic,strong) NSDictionary *configuration;
@property(nonatomic,strong) NSMutableArray *frames, *events, *schema, *snapshots;
@property(nonatomic,strong) NSMutableArray *animations;
@property(nonatomic,strong) NSMutableSet *seenAnimations;
@property(nonatomic,strong) NSMutableDictionary *layerIDs;
@property(nonatomic,strong) NSMutableDictionary *resolvedItemTints;
@property(nonatomic,strong) CADisplayLink *displayLink;
@property(nonatomic) CFTimeInterval start, ended, lastMove;
@property(nonatomic) BOOL completed, heldImage, pulledImage;
@property(nonatomic) NSUInteger releaseImage;
@end

@implementation InteractionController
- (void)viewDidLoad {
    [super viewDidLoad];
    NSData *data=[NSProcessInfo.processInfo.environment[@"GLASS_CASE"] dataUsingEncoding:NSUTF8StringEncoding];
    self.configuration=data ? [NSJSONSerialization JSONObjectWithData:data options:0 error:nil] : @{};
    self.overrideUserInterfaceStyle=[self.configuration[@"dark"] boolValue] ? UIUserInterfaceStyleDark : UIUserInterfaceStyleLight;
    self.view.backgroundColor=UIColor.systemBackgroundColor;
    self.status=[UILabel new]; self.status.text=@"preparing"; self.status.accessibilityIdentifier=@"status";
    self.status.frame=CGRectMake(20,80,600,40); [self.view addSubview:self.status];
    self.stage=[[UIView alloc] initWithFrame:CGRectMake(0,160,720,440)];
    NSArray *background=self.configuration[@"backgroundRGB"];
    self.stage.backgroundColor=background.count>=3 ? [UIColor colorWithRed:[background[0] doubleValue]/255 green:[background[1] doubleValue]/255 blue:[background[2] doubleValue]/255 alpha:1] : [UIColor colorWithWhite:.5 alpha:1];
    if ([self.configuration[@"artwork"] boolValue]) {
        UIImageView *image=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"scene.png"]];
        image.frame=self.stage.bounds; [self.stage addSubview:image];
    }
    [self.view addSubview:self.stage];
    UIView *host=self.stage;
    BOOL pair=[self.configuration[@"pair"] boolValue];
    if (pair) {
        UIGlassContainerEffect *group=[UIGlassContainerEffect new];
        group.spacing=self.configuration[@"spacing"] ? [self.configuration[@"spacing"] doubleValue] : 8;
        UIVisualEffectView *container=[[UIVisualEffectView alloc] initWithEffect:group];
        container.frame=self.stage.bounds; [host addSubview:container]; host=container.contentView;
    }
    CGFloat width=self.configuration[@"width"] ? [self.configuration[@"width"] doubleValue] : 52;
    CGFloat height=self.configuration[@"height"] ? [self.configuration[@"height"] doubleValue] : width;
    CGFloat gap=self.configuration[@"gap"] ? [self.configuration[@"gap"] doubleValue] : 12;
    self.resolvedItemTints=[NSMutableDictionary new];
    const int count=pair ? 2 : 1;
    for (int slot=0;slot<count;++slot) {
        const int i=[self.configuration[@"reverseOrder"] boolValue] ? count-1-slot : slot;
        UIGlassEffect *effect=[UIGlassEffect effectWithStyle:[self.configuration[@"clear"] boolValue] ? UIGlassEffectStyleClear : UIGlassEffectStyleRegular];
        effect.interactive=YES;
        if ([self.configuration[@"tinted"] boolValue] && (!pair || i==1)) effect.tintColor=[self.configuration[@"fixedTint"] boolValue] ? [UIColor colorWithRed:0 green:122.0/255 blue:1 alpha:1] : UIColor.systemBlueColor;
        NSArray *tints=self.configuration[@"itemTints"];
        if (i<tints.count) {
            id supplied=tints[i];
            effect.tintColor=[supplied isKindOfClass:NSArray.class] && [supplied count]>=3
                ? [UIColor colorWithRed:[supplied[0] doubleValue]/255 green:[supplied[1] doubleValue]/255 blue:[supplied[2] doubleValue]/255 alpha:[supplied count]>3 ? [supplied[3] doubleValue]/255 : 1] : nil;
        }
        CGFloat tr=0,tg=0,tb=0,ta=0;
        if (effect.tintColor) [[effect.tintColor resolvedColorWithTraitCollection:self.traitCollection] getRed:&tr green:&tg blue:&tb alpha:&ta];
        self.resolvedItemTints[[NSString stringWithFormat:@"glass%d",i]]=effect.tintColor ? @[@(tr),@(tg),@(tb),@(ta)] : (id)NSNull.null;
        UIVisualEffectView *glass=[[UIVisualEffectView alloc] initWithEffect:effect];
        CGFloat x=360-width*.5+(pair ? (i-.5)*(width+gap) : 0);
        glass.frame=CGRectMake(x,220-height*.5,width,height);
        glass.layer.cornerRadius=self.configuration[@"radius"] ? [self.configuration[@"radius"] doubleValue] : MIN(width,height)*.5;
        glass.layer.cornerCurve=kCACornerCurveContinuous; glass.layer.name=[NSString stringWithFormat:@"glass%d",i];
        [host addSubview:glass];
        UIButton *button=[UIButton buttonWithType:UIButtonTypeCustom];
        button.frame=glass.bounds; button.accessibilityIdentifier=i==0 && pair ? @"neighbor" : @"target";
        button.accessibilityLabel=button.accessibilityIdentifier;
        [glass.contentView addSubview:button];
        for (int axis=0;axis<2;++axis) {
            UIView *stroke=[[UIView alloc] initWithFrame:axis ? CGRectMake(width*.5-1,height*.5-10,2,20) : CGRectMake(width*.5-10,height*.5-1,20,2)];
            stroke.backgroundColor=effect.tintColor ? UIColor.whiteColor : UIColor.labelColor;
            stroke.userInteractionEnabled=NO; [button addSubview:stroke];
        }
    }
    self.frames=[NSMutableArray new]; self.events=[NSMutableArray new]; self.schema=[NSMutableArray new];
    self.snapshots=[NSMutableArray new]; self.layerIDs=[NSMutableDictionary new];
    self.animations=[NSMutableArray new]; self.seenAnimations=[NSMutableSet new];
    self.displayLink=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];
    self.displayLink.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);
    [self.displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.stage.frame=CGRectMake((self.view.bounds.size.width-720)*.5,160,720,440);
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    __weak InteractionController *weak=self;
    ((TouchWindow *)self.view.window).observe=^(UITouch *touch) { [weak observe:touch]; };
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.15*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        [weak snapshot:@"initial"]; weak.status.text=@"ready";
    });
}
- (NSURL *)directory {
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSString *identifier=self.configuration[@"id"] ?: @"probe";
    NSURL *directory=[documents URLByAppendingPathComponent:identifier isDirectory:YES];
    [NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil];
    return directory;
}
- (void)observe:(UITouch *)touch {
    if (self.completed || touch.phase==UITouchPhaseStationary) return;
    if (!self.start) {
        if (touch.phase!=UITouchPhaseBegan) return;
        self.start=touch.timestamp; [self sample:0];
    }
    CGPoint point=[touch locationInView:self.stage];
    [self.events addObject:@{@"t":@(touch.timestamp-self.start),@"dispatch":@(CACurrentMediaTime()-self.start),@"phase":@(touch.phase),@"x":@(point.x),@"y":@(point.y)}];
    if (touch.phase==UITouchPhaseMoved) self.lastMove=touch.timestamp;
    if (touch.phase==UITouchPhaseEnded || touch.phase==UITouchPhaseCancelled) self.ended=CACurrentMediaTime();
}
- (void)collect:(CALayer *)model path:(NSString *)path rows:(NSMutableArray *)rows {
    // UIKit inserts/removes presentation wrappers during feedback. A tree path
    // identifies a position, not a layer; keep the actual observed object ID.
    NSValue *identity=[NSValue valueWithNonretainedObject:model];
    NSNumber *identifier=self.layerIDs[identity];
    if (!identifier) {
        identifier=@(self.schema.count); self.layerIDs[identity]=identifier;
        [self.schema addObject:@{@"id":identifier,@"path":path,@"class":NSStringFromClass(model.class),@"name":model.name ?: @""}];
    }
    CALayer *layer=model.presentationLayer ?: model;
    CALayer *root=model.presentationLayer ? (self.stage.layer.presentationLayer ?: self.stage.layer) : self.stage.layer;
    CGRect f=[root convertRect:layer.bounds fromLayer:layer]; CATransform3D m=layer.transform;
    CATransform3D target=model.transform;
    NSMutableArray *values=[@[identifier,@(f.origin.x),@(f.origin.y),@(f.size.width),@(f.size.height),
                      @(layer.bounds.origin.x),@(layer.bounds.origin.y),@(layer.bounds.size.width),@(layer.bounds.size.height),
                      @(m.m11),@(m.m12),@(m.m21),@(m.m22),@(m.m41),@(m.m42),@(layer.cornerRadius),
                      @(target.m11),@(target.m22),@(target.m41),@(target.m42)] mutableCopy];
    // Transient private layers can have singular geometry; keep that unknown
    // explicit instead of turning it into a fabricated zero or invalid JSON.
    for (NSUInteger i=1;i<values.count;++i) if (!isfinite([values[i] doubleValue])) values[i]=NSNull.null;
    [rows addObject:values];
    for (NSString *key in model.animationKeys) {
        CAAnimation *animation=[model animationForKey:key];
        NSString *signature=[NSString stringWithFormat:@"%@/%@/%.9f",path,key,animation.beginTime];
        if (![self.seenAnimations containsObject:signature]) {
            [self.seenAnimations addObject:signature];
            NSMutableDictionary *entry=[[self describeAnimation:animation] mutableCopy];
            entry[@"layer"]=identifier; entry[@"key"]=key; entry[@"t"]=@(CACurrentMediaTime()-self.start);
            [self.animations addObject:entry];
        }
    }
    NSArray *children=model.sublayers;
    for (NSUInteger i=0;i<children.count;++i) [self collect:children[i] path:[path stringByAppendingFormat:@".%lu",(unsigned long)i] rows:rows];
}
- (id)animationValue:(id)value {
    if (!value) return NSNull.null;
    if ([value isKindOfClass:NSNumber.class]) return value;
    if ([value isKindOfClass:NSValue.class] && strcmp([value objCType],@encode(CATransform3D))==0) {
        CATransform3D t=[value CATransform3DValue]; return @[@(t.m11),@(t.m22),@(t.m41),@(t.m42)];
    }
    return [value description];
}
- (NSDictionary *)describeAnimation:(CAAnimation *)animation {
    NSMutableDictionary *entry=[@{@"class":NSStringFromClass(animation.class),@"duration":@(animation.duration),@"begin":@(animation.beginTime),@"speed":@(animation.speed)} mutableCopy];
    if ([animation isKindOfClass:CABasicAnimation.class]) {
        CABasicAnimation *basic=(CABasicAnimation *)animation;
        entry[@"path"]=basic.keyPath ?: @""; entry[@"from"]=[self animationValue:basic.fromValue]; entry[@"to"]=[self animationValue:basic.toValue];
    }
    if ([animation isKindOfClass:CASpringAnimation.class]) {
        CASpringAnimation *spring=(CASpringAnimation *)animation;
        entry[@"mass"]=@(spring.mass); entry[@"stiffness"]=@(spring.stiffness); entry[@"damping"]=@(spring.damping); entry[@"velocity"]=@(spring.initialVelocity);
    }
    if ([animation isKindOfClass:CAAnimationGroup.class]) {
        NSMutableArray *children=[NSMutableArray new];
        for (CAAnimation *child in ((CAAnimationGroup *)animation).animations) [children addObject:[self describeAnimation:child]];
        entry[@"children"]=children;
    }
    return entry;
}
- (void)sample:(CFTimeInterval)time {
    NSMutableArray *rows=[NSMutableArray new]; [self collect:self.stage.layer path:@"stage" rows:rows];
    [self.frames addObject:@{@"t":@(time),@"layers":rows}];
}
- (void)snapshot:(NSString *)name {
    if (![self.configuration[@"snapshots"] boolValue]) return;
    CFTimeInterval sampledAt=self.start ? CACurrentMediaTime()-self.start : -1;
    [self sample:sampledAt];
    UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat]; format.scale=2;
    UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:self.stage.bounds.size format:format];
    __block BOOL success=NO;
    UIImage *image=[renderer imageWithActions:^(UIGraphicsImageRendererContext *context) {
        success=[self.stage drawViewHierarchyInRect:self.stage.bounds afterScreenUpdates:NO];
    }];
    [UIImagePNGRepresentation(image) writeToURL:[[self directory] URLByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
    [self.snapshots addObject:@{@"name":name,@"t":@(sampledAt),@"completedAt":@(self.start ? CACurrentMediaTime()-self.start : -1),@"success":@(success)}];
}
- (void)tick:(CADisplayLink *)link {
    if (!self.start || self.completed) return;
    CFTimeInterval now=CACurrentMediaTime(); [self sample:now-self.start];
    if (!self.heldImage && now-self.start>.3) { self.heldImage=YES; [self snapshot:@"held"]; }
    if (!self.pulledImage && self.lastMove && !self.ended && now-self.lastMove>.2) { self.pulledImage=YES; [self snapshot:@"pulled"]; }
    if (self.ended) {
        const double checkpoints[]={.016,.066,.15,.3,.6,1};
        if (self.releaseImage<6 && now-self.ended>=checkpoints[self.releaseImage]) {
            [self snapshot:[NSString stringWithFormat:@"release-%lu",(unsigned long)self.releaseImage]]; ++self.releaseImage;
        }
        if (now-self.ended>1.25) {
            self.completed=YES; [self.displayLink invalidate];
            CGRect stage=[self.stage convertRect:self.stage.bounds toView:self.view.window];
            CGFloat red=0,green=0,blue=0,alpha=0;
            [[UIColor.systemBlueColor resolvedColorWithTraitCollection:self.traitCollection] getRed:&red green:&green blue:&blue alpha:&alpha];
            NSDictionary *trace=@{@"configuration":self.configuration,@"os":UIDevice.currentDevice.systemVersion,@"clockOrigin":@(self.start),@"resolvedSystemBlue":@[@(red),@(green),@(blue),@(alpha)],@"resolvedItemTints":self.resolvedItemTints,
                @"scale":@(self.view.window.screen.scale),@"stage":@[@(stage.origin.x),@(stage.origin.y),@(stage.size.width),@(stage.size.height)],
                @"schema":self.schema,@"events":self.events,@"frames":self.frames,@"snapshots":self.snapshots,@"animations":self.animations};
            NSData *data=[NSJSONSerialization dataWithJSONObject:FiniteJSON(trace) options:0 error:nil];
            [data writeToURL:[[self directory] URLByAppendingPathComponent:@"trace.json"] atomically:YES];
            self.status.text=@"complete";
        }
    }
}
- (void)dealloc { [self.displayLink invalidate]; }
@end

@interface InteractionScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) TouchWindow *window;
@end
@implementation InteractionScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window=[[TouchWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController=[InteractionController new]; [self.window makeKeyAndVisible];
}
@end
@interface InteractionApp : UIResponder <UIApplicationDelegate>
@end
@implementation InteractionApp
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *configuration=[[UISceneConfiguration alloc] initWithName:@"Interaction" sessionRole:session.role];
    configuration.delegateClass=InteractionScene.class; return configuration;
}
@end
int main(int argc,char **argv) { @autoreleasepool { return UIApplicationMain(argc,argv,nil,NSStringFromClass(InteractionApp.class)); } }
