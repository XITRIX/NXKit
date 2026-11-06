// Research-only private UIKit API caller. Never linked into NXKit.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
@interface UIView (LensCallbackResearch)
- (instancetype)initWithRestingBackground:(UIView *)view;
- (void)setLifted:(BOOL)lifted animated:(BOOL)animated alongsideAnimations:(void (^)(void))animations completion:(void (^)(BOOL,BOOL))completion;
@end
static NSMutableArray *events;
static CFTimeInterval start;
static void record(NSString *name, NSDictionary *fields) {
    NSMutableDictionary *row=[@{@"event":name,@"t":@(CACurrentMediaTime()-start)} mutableCopy];
    [row addEntriesFromDictionary:fields?:@{}]; [events addObject:row];
}
static id safe(id object, NSString *key) { @try { return [object valueForKey:key]?:[NSNull null]; } @catch(NSException *e) {return @{@"exception":e.name};} }
@interface App : UIResponder <UIApplicationDelegate,UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@property(nonatomic,strong) UIView *lens;
@end
@implementation App
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)options { return YES; }
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *configuration=[[UISceneConfiguration alloc]initWithName:@"Lens API" sessionRole:session.role]; configuration.delegateClass=App.class; return configuration;
}
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    events=[NSMutableArray array]; start=CACurrentMediaTime();
    self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController=[UIViewController new]; self.window.rootViewController.view.backgroundColor=UIColor.grayColor;
    [self.window makeKeyAndVisible];
    self.lens=[[NSClassFromString(@"_UILiquidLensView") alloc]initWithRestingBackground:nil];
    self.lens.frame=CGRectMake(100,200,116,56); [self.window.rootViewController.view addSubview:self.lens];
    record(@"defaults",@{@"liftedContentMode":safe(self.lens,@"liftedContentMode"),@"style":safe(self.lens,@"style"),@"warpsContentBelow":safe(self.lens,@"warpsContentBelow"),@"hasCustomRestingBackground":safe(self.lens,@"hasCustomRestingBackground"),@"restingBackgroundColor":@([safe(self.lens,@"restingBackgroundColor") isKindOfClass:NSNull.class]),@"restingBorderColor":@([safe(self.lens,@"restingBorderColor") isKindOfClass:NSNull.class]),@"restingShadowProperties":@([safe(self.lens,@"restingShadowProperties") isKindOfClass:NSNull.class])});
    NSArray *schedule=@[@[@.25,@"non-up",@YES,@NO],@[@.50,@"non-same-up",@YES,@NO],@[@.75,@"non-down",@NO,@NO],@[@1.0,@"non-same-down",@NO,@NO],@[@1.5,@"anim-up",@YES,@YES],@[@2.3,@"anim-same-up",@YES,@YES],@[@2.6,@"anim-down",@NO,@YES],@[@3.5,@"interrupted-up",@YES,@YES],@[@3.55,@"interrupted-down",@NO,@YES],@[@3.7,@"retarget-up",@YES,@YES],@[@4.5,@"final-down",@NO,@YES]];
    for(NSArray *row in schedule) dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)([row[0] doubleValue]*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        NSString *name=row[1]; record(@"call",@{@"name":name,@"lifted":row[2],@"animated":row[3]});
        [self.lens setLifted:[row[2] boolValue] animated:[row[3] boolValue] alongsideAnimations:^{ record(@"alongside",@{@"name":name}); } completion:^(BOOL a,BOOL b){ record(@"completion",@{@"name":name,@"a":@(a),@"b":@(b)}); }];
        record(@"returned",@{@"name":name,@"lifted":safe(self.lens,@"lifted")});
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,6*NSEC_PER_SEC),dispatch_get_main_queue(),^{
        NSString *path=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"callbacks.json"];
        NSDictionary *result=@{@"events":events,@"systemVersion":UIDevice.currentDevice.systemVersion,@"scale":@(self.window.screen.scale),@"scope":@"Authored private API calls, no touch routing or physical device"};
        [[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil] writeToFile:path atomically:YES];
    });
}
@end
int main(int argc,char **argv) { @autoreleasepool { return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class)); } }
