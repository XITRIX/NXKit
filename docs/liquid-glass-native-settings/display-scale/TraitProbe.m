#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <objc/message.h>
#import <dlfcn.h>
#import <math.h>
static NSString*S(const char*p){return p?([NSString stringWithUTF8String:p]?:@"<non-UTF8>"):@"";}
static id Value(id o,int depth);
static NSDictionary *Metadata(Class c) {
 if(!c)return @{@"name":@"<nil>"};
 NSMutableArray *methods=[NSMutableArray new],*properties=[NSMutableArray new],*ivars=[NSMutableArray new];unsigned n=0;
 Method *m=class_copyMethodList(c,&n);for(unsigned i=0;i<n;i++)[methods addObject:@{@"name":NSStringFromSelector(method_getName(m[i])),@"type":S(method_getTypeEncoding(m[i]))}];free(m);
 objc_property_t *p=class_copyPropertyList(c,&n);for(unsigned i=0;i<n;i++)[properties addObject:@{@"name":S(property_getName(p[i])),@"type":S(property_getAttributes(p[i]))}];free(p);
 Ivar *v=class_copyIvarList(c,&n);for(unsigned i=0;i<n;i++)[ivars addObject:@{@"name":S(ivar_getName(v[i])),@"type":S(ivar_getTypeEncoding(v[i])),@"offset":@(ivar_getOffset(v[i]))}];free(v);
 return @{@"name":NSStringFromClass(c),@"super":NSStringFromClass(class_getSuperclass(c))?:@"",@"image":@(class_getImageName(c)?:""),@"methods":methods,@"properties":properties,@"ivars":ivars};
}
static id Value(id o,int depth) {
 if(!o)return NSNull.null;if([o isKindOfClass:NSString.class])return o;if([o isKindOfClass:NSNumber.class])return isfinite([o doubleValue])?o:[o description];
 if([o isKindOfClass:UIColor.class]) {CGFloat r=0,g=0,b=0,a=0;if([o getRed:&r green:&g blue:&b alpha:&a])return @{@"class":NSStringFromClass([o class]),@"rgba":@[@(r),@(g),@(b),@(a)]};}
 if([o isKindOfClass:CAMediaTimingFunction.class]){NSMutableArray*p=[NSMutableArray new];for(size_t i=0;i<4;i++){float v[2]={0,0};[(CAMediaTimingFunction*)o getControlPointAtIndex:i values:v];[p addObject:@[@(v[0]),@(v[1])]];}return @{@"class":NSStringFromClass([o class]),@"controlPoints":@[p[1][0],p[1][1],p[2][0],p[2][1]]};}
 if([o isKindOfClass:NSValue.class])return @{@"class":NSStringFromClass([o class]),@"encoding":@([o objCType]),@"description":[o description]};
 if([NSStringFromClass([o class])isEqual:@"__NSCFType"]&&CFGetTypeID((__bridge CFTypeRef)o)==CGColorGetTypeID()){CGColorRef color=(__bridge CGColorRef)o;NSMutableArray*c=[NSMutableArray new];for(size_t i=0;i<CGColorGetNumberOfComponents(color);i++)[c addObject:@(CGColorGetComponents(color)[i])];return @{@"class":@"CGColor",@"components":c};}
 if(depth<=0)return @{@"class":NSStringFromClass([o class])};
 if([o isKindOfClass:NSArray.class]){NSMutableArray*a=[NSMutableArray new];for(id x in o)if(a.count<64)[a addObject:Value(x,depth-1)];return a;}
 if([o isKindOfClass:NSDictionary.class]){NSMutableDictionary*d=[NSMutableDictionary new];for(id k in o)if([k isKindOfClass:NSString.class])d[k]=Value(o[k],depth-1);return d;}
 NSMutableDictionary *d=[@{@"class":NSStringFromClass([o class])} mutableCopy];
 if([NSStringFromClass([o class])isEqual:@"CAFilter"]){@try{NSData*archive=[NSKeyedArchiver archivedDataWithRootObject:o requiringSecureCoding:NO error:nil];if(archive)d[@"archiveBase64"]=[archive base64EncodedStringWithOptions:0];d[@"name"]=[o valueForKey:@"name"]?:NSNull.null;NSArray*keys=[o valueForKey:@"inputKeys"];for(NSString*k in keys)d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){d[@"error"]=e.reason?:@"";}}

 if([NSStringFromClass([o class])containsString:@"SwiftValue"]||[NSStringFromClass([o class])containsString:@"EquatableBox"])d[@"description"]=[o description];
 if([o isKindOfClass:CALayer.class])for(NSString*k in @[@"filters",@"backgroundFilters",@"compositingFilter"]){@try{d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){}}

 for(Class c=[o class];c&&c!=NSObject.class&&c!=UIView.class&&c!=CALayer.class;c=class_getSuperclass(c)){
  unsigned n=0;objc_property_t*p=class_copyPropertyList(c,&n);for(unsigned i=0;i<n;i++){@try{NSString*k=S(property_getName(p[i]));if(![k hasPrefix:@"debug"]&&![k isEqual:@"description"]&&![k isEqual:@"parent"]&&![k isEqual:@"delegate"])d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){}}free(p);
 }
 return d;
}
static void TestMutation(CALayer*l,NSMutableArray*results){for(id f in l.filters)if([[f valueForKey:@"type"]isEqual:@"glassBackground"]){id old=[f valueForKey:@"inputBlurRadius"];if([old isKindOfClass:NSNumber.class]){@try{[f setValue:@([old doubleValue]+1)forKey:@"inputBlurRadius"];id changed=[f valueForKey:@"inputBlurRadius"];[f setValue:old forKey:@"inputBlurRadius"];[results addObject:@{@"key":@"inputBlurRadius",@"before":old,@"after":changed?:NSNull.null,@"restored":[f valueForKey:@"inputBlurRadius"]?:NSNull.null}];}@catch(NSException*e){[results addObject:@{@"error":e.reason?:@""}];}}}for(CALayer*c in l.sublayers)TestMutation(c,results);}
static NSDictionary*LayerTree(CALayer*l,int depth){NSMutableDictionary*d=[@{@"value":Value(l,5),@"bounds":NSStringFromCGRect(l.bounds),@"cornerRadius":@(l.cornerRadius)}mutableCopy];NSMutableArray*a=[NSMutableArray new];if(depth>0)for(CALayer*c in l.sublayers)[a addObject:LayerTree(c,depth-1)];d[@"sublayers"]=a;return d;}
static NSDictionary *Tree(UIView*v,int depth){NSMutableDictionary*d=[@{@"class":NSStringFromClass(v.class),@"frame":NSStringFromCGRect(v.frame),@"layer":Value(v.layer,2)}mutableCopy];NSMutableDictionary*refs=[NSMutableDictionary new];
 for(Class c=v.class;c&&c!=NSObject.class;c=class_getSuperclass(c)){unsigned n=0;Ivar*iv=class_copyIvarList(c,&n);for(unsigned i=0;i<n;i++){NSString*k=S(ivar_getName(iv[i]));if(ivar_getTypeEncoding(iv[i])&&ivar_getTypeEncoding(iv[i])[0]=='@'&&([k.lowercaseString containsString:@"setting"]||[k.lowercaseString containsString:@"effect"]||[k.lowercaseString containsString:@"spec"]||[k.lowercaseString containsString:@"recipe"]||[k.lowercaseString containsString:@"glass"]))@try{refs[k]=Value(object_getIvar(v,iv[i]),4);}@catch(NSException*e){}}free(iv);}for(NSString*k in @[@"_glass",@"glass",@"_glassBackgroundStyle"]){SEL sel=NSSelectorFromString(k);if([v respondsToSelector:sel])@try{refs[k]=Value([v valueForKey:k],5);}@catch(NSException*e){}}d[@"references"]=refs;
 NSMutableArray*children=[NSMutableArray new];if(depth>0)for(UIView*child in v.subviews)[children addObject:Tree(child,depth-1)];d[@"children"]=children;return d;}
@interface Probe:UIViewController @property NSMutableArray*surfaces;@end
@implementation Probe
- (void)viewDidLoad{[super viewDidLoad];self.view.backgroundColor=UIColor.systemBackgroundColor;self.overrideUserInterfaceStyle=NSProcessInfo.processInfo.environment[@"PROBE_LIGHT"]?UIUserInterfaceStyleLight:UIUserInterfaceStyleDark;self.surfaces=[NSMutableArray new];
 NSArray*heights=@[@44,@52,@62,@64,@80,@120,@160];NSMutableArray*sizes=[NSMutableArray new];for(NSNumber*h in heights)[sizes addObject:@[@280,h]];for(NSNumber*h in heights)[sizes addObject:@[h,h]];
 if(NSProcessInfo.processInfo.environment[@"PROBE_SINGLE_COLOR"])sizes=[@[@[@280,@80]]mutableCopy];BOOL allStyles=NSProcessInfo.processInfo.environment[@"PROBE_ALL_STYLES"]!=nil;
 for(NSArray*size in sizes)for(int style=0;style<(allStyles?4:3);style++){CGFloat w=[size[0]doubleValue],h=[size[1]doubleValue];UIGlassEffect*e=[UIGlassEffect effectWithStyle:(style==1||style==3)?UIGlassEffectStyleClear:UIGlassEffectStyleRegular];if(style>=2){NSString*rgb=NSProcessInfo.processInfo.environment[@"PROBE_TINT_RGB"];NSArray*a=[rgb componentsSeparatedByString:@","];e.tintColor=a.count==3?[UIColor colorWithRed:[a[0]doubleValue]green:[a[1]doubleValue]blue:[a[2]doubleValue]alpha:1]:UIColor.systemBlueColor;}e.interactive=NSProcessInfo.processInfo.environment[@"PROBE_INTERACTIVE"]!=nil;UIVisualEffectView*v=[[UIVisualEffectView alloc]initWithEffect:e];v.frame=CGRectMake(12,80+self.surfaces.count*22,w,h);v.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:MIN(w,h)/2]];[self.view addSubview:v];[self.surfaces addObject:@{@"view":v,@"width":@(w),@"height":@(h),@"style":@[@"regular",@"clear",@"tinted",@"clearTinted"][style]}];}
 dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC*2),dispatch_get_main_queue(),^{[self.view.window layoutIfNeeded];[CATransaction flush];NSMutableArray*surfaces=[NSMutableArray new];for(NSDictionary*s in self.surfaces){UIView*v=s[@"view"];[surfaces addObject:@{@"width":s[@"width"],@"height":s[@"height"],@"style":s[@"style"],@"resolvedSystemBlue":Value([UIColor.systemBlueColor resolvedColorWithTraitCollection:v.traitCollection],1),@"layerTree":LayerTree(v.layer,16)}];}NSNumber*(*getTint)(void)=dlsym(RTLD_DEFAULT,"UIViewGlassGetTintAmount");NSDictionary*d=@{@"runID":NSProcessInfo.processInfo.environment[@"PROBE_RUN_ID"]?:@"manual",@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"scale":@(self.view.window.screen.scale),@"traitScale":@(self.view.traitCollection.displayScale),@"traitIdiom":@(self.view.traitCollection.userInterfaceIdiom),@"effectiveTint":getTint?(getTint()?:NSNull.null):NSNull.null,@"surfaces":surfaces,@"layerTree":LayerTree(self.view.window.layer,20)};[[NSJSONSerialization dataWithJSONObject:d options:0 error:nil]writeToFile:[NSHomeDirectory()stringByAppendingPathComponent:@"Documents/sizes.json"]atomically:YES];});}
@end
@interface TraitContainer:UIViewController @end
@implementation TraitContainer
- (void)viewDidLoad{[super viewDidLoad]; Probe*p=[Probe new];[self addChildViewController:p];NSMutableArray*a=[NSMutableArray new];NSString*scale=NSProcessInfo.processInfo.environment[@"PROBE_OVERRIDE_SCALE"],*idiom=NSProcessInfo.processInfo.environment[@"PROBE_OVERRIDE_IDIOM"];if(scale)[a addObject:[UITraitCollection traitCollectionWithDisplayScale:scale.doubleValue]];if(idiom)[a addObject:[UITraitCollection traitCollectionWithUserInterfaceIdiom:idiom.integerValue]];if(a.count)[self setOverrideTraitCollection:[UITraitCollection traitCollectionWithTraitsFromCollections:a]forChildViewController:p];p.view.frame=self.view.bounds;p.view.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;[self.view addSubview:p.view];[p didMoveToParentViewController:self];}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>@property(nonatomic,strong) UIWindow*window;@end
@implementation Scene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options{self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)scene];self.window.rootViewController=[TraitContainer new];[self.window makeKeyAndVisible];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options{UISceneConfiguration*c=[[UISceneConfiguration alloc]initWithName:@"Probe" sessionRole:session.role];c.delegateClass=Scene.class;return c;}
@end
int main(int argc,char**argv){@autoreleasepool{NSString*amount=NSProcessInfo.processInfo.environment[@"PROBE_TINT_AMOUNT"];NSUserDefaults*(*defaults)(void)=dlsym(RTLD_DEFAULT,"_UIKitUserDefaults");if(amount&&defaults){NSMutableDictionary*d=[[defaults()volatileDomainForName:NSArgumentDomain]mutableCopy]?:[NSMutableDictionary new];d[@"UIViewGlassTintAmount"]=@([amount doubleValue]);[defaults()setVolatileDomain:d forName:NSArgumentDomain];}return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
