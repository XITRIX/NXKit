// Research-only app-space native lens setter timing. No control or synthetic touch.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
@protocol NativeLens
- (instancetype)initWithRestingBackground:(UIView *)background;
- (void)setStyle:(NSInteger)style;
- (void)setLiftedContainerView:(UIView *)container;
- (void)setLifted:(BOOL)lifted animated:(BOOL)animated alongsideAnimations:(void (^)(void))animations completion:(void (^)(BOOL,BOOL))completion;
@end
static id Read(id object,NSString *key){@try{return [object valueForKey:key]?:NSNull.null;}@catch(NSException *e){return @{ @"exception":e.name };}}
static NSArray *LensRect(CGRect r){return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
@interface MinimumController:UIViewController
@property NSMutableArray *cases,*events,*frames;
@property CADisplayLink *link;
@end
@implementation MinimumController
- (void)event:(NSString *)kind test:(NSDictionary *)test fields:(NSDictionary *)fields{NSMutableDictionary *d=[@{@"kind":kind,@"id":test[@"id"],@"t":@(CACurrentMediaTime())} mutableCopy];[d addEntriesFromDictionary:fields?:@{}];[self.events addObject:d];}
- (void)change:(NSDictionary *)test lifted:(BOOL)lifted animated:(BOOL)animated{
 UIView<NativeLens> *lens=test[@"lens"];[self event:@"call" test:test fields:@{@"lifted":@(lifted),@"animated":@(animated)}];
 [lens setLifted:lifted animated:animated alongsideAnimations:^{[self event:@"alongside" test:test fields:@{@"lifted":@(lifted)}];lens.bounds=CGRectMake(0,0,lifted?58:37,lifted?38.333333333333336:24);} completion:^(BOOL a,BOOL b){[self event:@"completion" test:test fields:@{@"lifted":@(lifted),@"a":@(a),@"b":@(b)}];}];
 [self event:@"returned" test:test fields:@{@"requestedLifted":@(lifted),@"actualLifted":Read(lens,@"lifted")}];
}
- (void)viewDidAppear:(BOOL)animated{
 [super viewDidAppear:animated];self.view.backgroundColor=UIColor.systemBackgroundColor;self.cases=[NSMutableArray new];self.events=[NSMutableArray new];self.frames=[NSMutableArray new];
 NSArray *delays=@[@0,@.03,@.10,@.30];for(NSInteger style=0;style<2;style++)for(NSUInteger i=0;i<delays.count;i++){
  UIView *background=[UIView new];background.backgroundColor=UIColor.whiteColor;
  UIView<NativeLens> *lens=(id)[(id<NativeLens>)[NSClassFromString(@"_UILiquidLensView") alloc] initWithRestingBackground:background];NSAssert(lens,@"Native lens missing");[lens setStyle:style];[lens setLiftedContainerView:self.view];lens.bounds=CGRectMake(0,0,37,24);lens.center=CGPointMake(100+style*180,160+i*100);[self.view addSubview:lens];
  NSDictionary *test=@{@"id":[NSString stringWithFormat:@"style%ld-down%g",(long)style,[delays[i] doubleValue]],@"style":@(style),@"delay":delays[i],@"lens":lens};[self.cases addObject:test];
 }
 self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(sample:)];self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(),^{for(NSDictionary *test in self.cases){[self change:test lifted:YES animated:YES];double delay=[test[@"delay"] doubleValue];if(delay==0)[self change:test lifted:NO animated:YES];else dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(delay*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[self change:test lifted:NO animated:YES];});}});
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,3*NSEC_PER_SEC),dispatch_get_main_queue(),^{[self.link invalidate];NSMutableArray *configuration=[NSMutableArray new];for(NSDictionary *test in self.cases)[configuration addObject:@{@"id":test[@"id"],@"style":test[@"style"],@"delay":test[@"delay"]}];NSDictionary *trace=@{@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"scale":@(self.view.window.screen.scale),@"configuration":configuration,@"events":self.events,@"frames":self.frames,@"scope":@"Authored native private setLifted:animated:alongsideAnimations:completion: calls at0/30/100/300ms forsmall/largestyle; no control, no touches, no mutation of native timing/spec. Write once after collection."};NSString *path=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"minimum-lift.json"];[[NSJSONSerialization dataWithJSONObject:trace options:0 error:nil] writeToFile:path atomically:YES];});
}
- (void)sample:(CADisplayLink *)link{for(NSDictionary *test in self.cases){UIView *lens=test[@"lens"];CALayer *p=lens.layer.presentationLayer?:lens.layer;[self.frames addObject:@{@"id":test[@"id"],@"t":@(CACurrentMediaTime()),@"lifted":Read(lens,@"lifted"),@"modelBounds":LensRect(lens.bounds),@"presentationBounds":LensRect(p.bounds)}];}}
@end
@interface MinimumApp:UIResponder<UIApplicationDelegate,UIWindowSceneDelegate>@property(nonatomic,strong) UIWindow *window;@end
@implementation MinimumApp
- (UISceneConfiguration *)application:(UIApplication *)a configurationForConnectingSceneSession:(UISceneSession *)s options:(UISceneConnectionOptions *)o{UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Minimum lens timing" sessionRole:s.role];c.delegateClass=MinimumApp.class;return c;}
- (void)scene:(UIScene *)s willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)o{self.window=[[UIWindow alloc] initWithWindowScene:(id)s];self.window.rootViewController=[MinimumController new];[self.window makeKeyAndVisible];}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(MinimumApp.class));}}
