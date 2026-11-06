// Spatial native lighting perturbations. UIKit only; never links NXKit.
#import <UIKit/UIKit.h>
#import <dlfcn.h>
static NSMutableArray *observations;
static NSDictionary *Changes(void) {
    NSString *text=NSProcessInfo.processInfo.environment[@"PASS_CHANGES"];
    return text?[NSJSONSerialization JSONObjectWithData:[text dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil]:@{};
}
static void Mutate(CALayer *layer) {
    NSMutableArray *filters=[NSMutableArray new]; BOOL changed=NO;
    for(id original in layer.filters) {
        id filter=original;
        if([[filter valueForKey:@"type"] isEqual:@"glassBackground"]) {
            filter=[filter copy]; changed=YES;
            NSMutableDictionary *record=[@{@"class":NSStringFromClass(layer.class),@"bounds":NSStringFromCGRect(layer.bounds),@"changes":[NSMutableDictionary new]} mutableCopy];
            for(NSString *key in Changes()) @try {
                id old=[filter valueForKey:key],value=Changes()[key];
                [filter setValue:value forKey:key];
                record[@"changes"][key]=@{@"before":[old description]?:@"nil",@"after":[[filter valueForKey:key] description]?:@"nil"};
            } @catch(NSException *exception) {record[@"error"]=exception.reason?:@"mutation failed";}
            [observations addObject:record];
        }
        [filters addObject:filter];
    }
    if(changed) { layer.filters=nil;layer.filters=filters;[layer setNeedsDisplay]; }
    if([NSStringFromClass(layer.class) isEqual:@"CASDFLayer"]) @try {
        id effect=[layer valueForKey:@"effect"];
        if([NSStringFromClass([effect class]) isEqual:@"CASDFKeyFillHighlightEffect"]) {
            NSString *mode=NSProcessInfo.processInfo.environment[@"PASS_HIGHLIGHT"];
            NSMutableDictionary *record=[@{@"highlight":@YES,@"mode":mode?:@"normal",@"bounds":NSStringFromCGRect(layer.bounds),@"filtersBefore":@(layer.filters.count)} mutableCopy];
            NSString *sdfText=NSProcessInfo.processInfo.environment[@"PASS_SDF_CHANGES"];
            if(sdfText) {
                NSDictionary *changes=[NSJSONSerialization JSONObjectWithData:[sdfText dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
                BOOL canCopy=[effect respondsToSelector:@selector(copyWithZone:)];
                id replacement=canCopy?[effect copy]:effect;
                NSMutableDictionary *mutations=[NSMutableDictionary new];
                for(NSString *key in changes) {
                    id previous=[replacement valueForKey:key];[replacement setValue:changes[key] forKey:key];
                    mutations[key]=@{@"before":[previous description]?:@"nil",@"after":[[replacement valueForKey:key] description]?:@"nil"};
                }
                [layer setValue:nil forKey:@"effect"];[layer setValue:replacement forKey:@"effect"];
                record[@"sdfChanges"]=mutations;record[@"copiedEffect"]=@(canCopy);
            }
            if([mode isEqual:@"off"]) layer.opacity=0;
            if([mode isEqual:@"noMatrix"]) layer.filters=nil;
            record[@"opacityAfter"]=@(layer.opacity);record[@"filtersAfter"]=@(layer.filters.count);
            [observations addObject:record];
        }
    } @catch(NSException *exception) {[observations addObject:@{@"error":exception.reason?:@"highlight access failed"}];}
    for(CALayer *child in layer.sublayers) Mutate(child);
}
@interface Probe:UIViewController @end
@implementation Probe
- (BOOL)prefersStatusBarHidden{return YES;}
- (void)viewDidLoad {
    [super viewDidLoad]; self.overrideUserInterfaceStyle=NSProcessInfo.processInfo.environment[@"PASS_LIGHT"]?UIUserInterfaceStyleLight:UIUserInterfaceStyleDark;
    NSString *uniform=NSProcessInfo.processInfo.environment[@"PASS_UNIFORM_GRAY"];
    self.view.backgroundColor=uniform?[UIColor colorWithWhite:[uniform doubleValue] alpha:1]:UIColor.blackColor;
    NSArray *grays=uniform?@[@([uniform doubleValue]),@([uniform doubleValue]),@([uniform doubleValue])]:@[@0,@.5,@1];
    NSMutableArray *frames=[NSMutableArray new];
    for(int row=0;row<3;row++) {
        CGFloat y=80+row*240;
        UIView *background=[[UIView alloc] initWithFrame:CGRectMake(0,y-35,500,220)];
        background.backgroundColor=[UIColor colorWithWhite:[grays[row] doubleValue] alpha:1];[self.view addSubview:background];
        const CGFloat positions[]={20,90,220},sizes[]={44,80,120};
        for(int column=0;column<3;column++) {
            CGFloat size=sizes[column];
            UIGlassEffect *effect=[UIGlassEffect effectWithStyle:UIGlassEffectStyleRegular];
            UIVisualEffectView *view=[[UIVisualEffectView alloc] initWithEffect:effect];
            view.frame=CGRectMake(positions[column],y,size,size);
            view.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:size/2]];
            [self.view addSubview:view];[frames addObject:@{@"frame":NSStringFromCGRect(view.frame),@"gray":grays[row]}];
        }
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(),^{
        observations=[NSMutableArray new];[self.view.window layoutIfNeeded];Mutate(self.view.window.layer);[CATransaction flush];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC/2),dispatch_get_main_queue(),^{
            NSDictionary *result=@{@"runID":NSProcessInfo.processInfo.environment[@"PASS_RUN_ID"]?:@"manual",@"changes":Changes(),@"highlight":NSProcessInfo.processInfo.environment[@"PASS_HIGHLIGHT"]?:@"normal",@"requestedTint":NSProcessInfo.processInfo.environment[@"PASS_TINT_AMOUNT"]?:@"system",@"mutations":observations,@"frames":frames,@"scale":@(self.view.window.screen.scale),@"bounds":NSStringFromCGRect(self.view.bounds),@"os":NSProcessInfo.processInfo.operatingSystemVersionString};
            [[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil] writeToFile:[NSHomeDirectory() stringByAppendingPathComponent:@"Documents/pass.json"] atomically:YES];
        });
    });
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>@property(nonatomic,strong) UIWindow *window;@end
@implementation Scene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)scene];self.window.rootViewController=[Probe new];[self.window makeKeyAndVisible];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {UISceneConfiguration *configuration=[[UISceneConfiguration alloc]initWithName:@"Probe" sessionRole:session.role];configuration.delegateClass=Scene.class;return configuration;}
@end
int main(int argc,char **argv) {@autoreleasepool {
    NSString *amount=NSProcessInfo.processInfo.environment[@"PASS_TINT_AMOUNT"];
    NSUserDefaults *(*defaults)(void)=dlsym(RTLD_DEFAULT,"_UIKitUserDefaults");
    if(amount&&defaults) {NSMutableDictionary *domain=[[defaults() volatileDomainForName:NSArgumentDomain] mutableCopy]?:[NSMutableDictionary new];domain[@"UIViewGlassTintAmount"]=@([amount doubleValue]);[defaults() setVolatileDomain:domain forName:NSArgumentDomain];}
    return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));
}}
