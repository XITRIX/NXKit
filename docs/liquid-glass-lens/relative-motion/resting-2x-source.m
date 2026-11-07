// Native-only research fixture: isolate lens placement from ancestor motion.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/message.h>

@protocol MotionLens
- (instancetype)initWithRestingBackground:(UIView *)background;
- (void)setStyle:(NSInteger)style;
- (void)setLiftedContainerView:(UIView *)container;
- (void)setLifted:(BOOL)lifted animated:(BOOL)animated alongsideAnimations:(void (^)(void))block completion:(void (^)(BOOL,BOOL))completion;
@end
static id Getter(id object,NSString *name) {
    SEL selector=NSSelectorFromString(name);
    NSMethodSignature *signature=[object methodSignatureForSelector:selector];
    if(!signature||signature.numberOfArguments!=2)return NSNull.null;
    NSInvocation *call=[NSInvocation invocationWithMethodSignature:signature];call.target=object;call.selector=selector;
    @try {
        [call invoke];const char *type=signature.methodReturnType;
        if(type[0]=='@'){__unsafe_unretained id value;[call getReturnValue:&value];return value?:NSNull.null;}
        if(!strcmp(type,@encode(CGPoint))){CGPoint p;[call getReturnValue:&p];return @[@(p.x),@(p.y)];}
        if(!strcmp(type,@encode(BOOL))){BOOL value;[call getReturnValue:&value];return @(value);}
        if(!strcmp(type,@encode(NSInteger))){NSInteger value;[call getReturnValue:&value];return @(value);}
        if(!strcmp(type,@encode(NSUInteger))){NSUInteger value;[call getReturnValue:&value];return @(value);}
        if(!strcmp(type,@encode(int))){int value;[call getReturnValue:&value];return @(value);}
    }@catch(NSException *exception){return @{ @"exception":exception.name };}
    return NSNull.null;
}
@interface RelativeMotionController:UIViewController
@property NSMutableArray<NSDictionary *> *cases;
@property NSMutableArray *frames;
@property CADisplayLink *link;
@property CFTimeInterval start;
@end
@implementation RelativeMotionController
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];self.view.backgroundColor=UIColor.systemBackgroundColor;
    self.cases=[NSMutableArray new];self.frames=[NSMutableArray new];
    NSArray *names=@[@"stationary",@"lens-position",@"parent-position",@"ancestor-position",@"parent-transform",@"scroll-offset",@"parent-bounds",@"lens-animation",@"ancestor-animation"];
    for(NSUInteger i=0;i<names.count;i++)for(NSInteger style=0;style<2;style++) {
        UIView *ancestor=[[UIView alloc] initWithFrame:CGRectMake(20+style*210,70+i*82,190,75)];[self.view addSubview:ancestor];
        UIView *parent=[[UIView alloc] initWithFrame:CGRectMake(0,0,180,70)];[ancestor addSubview:parent];
        UIScrollView *scroll=nil;
        if([names[i] isEqual:@"scroll-offset"]) {
            scroll=[[UIScrollView alloc] initWithFrame:parent.bounds];scroll.clipsToBounds=NO;scroll.contentSize=CGSizeMake(400,70);[parent addSubview:scroll];
            parent=[[UIView alloc] initWithFrame:CGRectMake(0,0,400,70)];[scroll addSubview:parent];
        }
        UIView *background=[UIView new];background.backgroundColor=UIColor.secondarySystemFillColor;
        UIView<MotionLens> *lens=(id)[(id<MotionLens>)[NSClassFromString(@"_UILiquidLensView") alloc] initWithRestingBackground:background];
        NSAssert(lens,@"Native lens unavailable");[lens setStyle:style];[lens setLiftedContainerView:ancestor];
        lens.bounds=CGRectMake(0,0,style?100:58,style?44:38);lens.center=CGPointMake(90,35);lens.userInteractionEnabled=NO;
        BOOL lifted=![[NSProcessInfo.processInfo.environment objectForKey:@"RELATIVE_MOTION_LIFTED"] isEqual:@"0"];
        [parent addSubview:lens];[lens setLifted:lifted animated:NO alongsideAnimations:^{} completion:nil];
        [self.cases addObject:@{@"name":names[i],@"style":@(style),@"lens":lens,@"parent":parent,@"ancestor":ancestor,@"scroll":scroll?:NSNull.null,@"ancestorOrigin":[NSValue valueWithCGPoint:ancestor.frame.origin]}];
    }
    self.start=CACurrentMediaTime();self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];
    self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,1*NSEC_PER_SEC),dispatch_get_main_queue(),^{
        for(NSDictionary *test in self.cases)if([test[@"name"] hasSuffix:@"animation"]) {
            UIView *target=[test[@"name"] isEqual:@"lens-animation"]?test[@"lens"]:test[@"ancestor"];
            [UIView animateWithDuration:.8 delay:0 options:UIViewAnimationOptionCurveLinear animations:^{target.center=CGPointMake(target.center.x+60,target.center.y);} completion:nil];
        }
    });
}
- (void)tick:(CADisplayLink *)link {
    double t=CACurrentMediaTime()-self.start;
    double motion=(t>=1&&t<2.2)?45*sin((t-1)*2*M_PI/.6):0;
    [CATransaction begin];[CATransaction setDisableActions:YES];
    for(NSDictionary *test in self.cases) {
        NSString *name=test[@"name"];UIView *lens=test[@"lens"],*parent=test[@"parent"],*ancestor=test[@"ancestor"];
        if([name isEqual:@"lens-position"])lens.center=CGPointMake(90+motion,35);
        if([name isEqual:@"parent-position"])parent.center=CGPointMake(90+motion,35);
        if([name isEqual:@"ancestor-position"]){CGPoint p=[test[@"ancestorOrigin"] CGPointValue];ancestor.frame=CGRectMake(p.x+motion,p.y,190,75);}
        if([name isEqual:@"parent-transform"])parent.transform=CGAffineTransformMakeTranslation(motion,0);
        if([name isEqual:@"scroll-offset"])[test[@"scroll"] setContentOffset:CGPointMake(motion,0) animated:NO];
        if([name isEqual:@"parent-bounds"])parent.bounds=CGRectMake(motion,0,180,70);
    }
    [CATransaction commit];
    for(NSDictionary *test in self.cases) {
        UIView *lens=test[@"lens"];id flex=Getter(lens,@"flexInteraction");id integrator=Getter(flex,@"velocityIntegrator");
        CALayer *p=lens.layer.presentationLayer?:lens.layer;CATransform3D a=p.transform;
        [self.frames addObject:@{@"name":test[@"name"],@"style":test[@"style"],@"t":@(t),@"position":@[@(lens.layer.position.x),@(lens.layer.position.y)],@"presentationPosition":@[@(p.position.x),@(p.position.y)],@"transform":@[@(a.m11),@(a.m12),@(a.m21),@(a.m22),@(a.m41),@(a.m42)],@"integratorPosition":Getter(integrator,@"position"),@"integratorVelocity":Getter(integrator,@"velocity"),@"flexSources":Getter(flex,@"preferredFlexSources"),@"activation":Getter(flex,@"resolvedActivationMode")}];
    }
    if(t>4) {
        [self.link invalidate];self.link=nil;
        NSDictionary *result=@{@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"scale":@(self.view.window.screen.scale),@"lifted":@(![[NSProcessInfo.processInfo.environment objectForKey:@"RELATIVE_MOTION_LIFTED"] isEqual:@"0"]),@"frames":self.frames};
        NSURL *folder=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
        [NSFileManager.defaultManager createDirectoryAtURL:folder withIntermediateDirectories:YES attributes:nil error:nil];
        [[NSJSONSerialization dataWithJSONObject:result options:0 error:nil] writeToURL:[folder URLByAppendingPathComponent:@"relative-motion.json"] atomically:YES];
    }
}
@end
@interface RelativeMotionScene:UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation RelativeMotionScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];self.window.rootViewController=[RelativeMotionController new];[self.window makeKeyAndVisible];
}
@end
@interface RelativeMotionApp:UIResponder<UIApplicationDelegate>
@end
@implementation RelativeMotionApp
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Relative motion" sessionRole:session.role];c.delegateClass=RelativeMotionScene.class;return c;
}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(RelativeMotionApp.class));}}
