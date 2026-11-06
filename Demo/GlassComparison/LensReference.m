// Independent research fixture. Private selectors stay in this native-only app.
// No private UIKit implementation is linked into NXKit.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
#import <dlfcn.h>
#import "SettingsProbe/RuntimeInspection.h"

@protocol ResearchLiquidLens
- (instancetype)initWithRestingBackground:(UIView *)background;
- (void)setLiftedContainerView:(UIView *)view;
- (void)setLiftedContentView:(UIView *)view;
- (void)setOverridePunchoutView:(UIView *)view;
- (void)setLiftedContentMode:(NSInteger)mode;
- (void)setStyle:(NSInteger)style;
- (void)setWarpsContentBelow:(BOOL)enabled;
- (void)setRestingBackgroundColor:(UIColor *)color;
- (void)setLifted:(BOOL)lifted animated:(BOOL)animated
    alongsideAnimations:(void (^)(void))animations completion:(void (^)(BOOL,BOOL))completion;
@end

// The initializer takes caller-owned resting appearance, not an effect preset.
@interface LensRestingBackground : UIView
@end
@implementation LensRestingBackground
- (void)layoutSubviews { [super layoutSubviews]; self.layer.cornerRadius=MIN(self.bounds.size.width,self.bounds.size.height)/2; }
@end

// Rich, independently drawn source with64pt overscan around the original bar.
// It intentionally crosses every lens edge; centered glyphs cannot test that.
@interface ProbeContentView : UIView
@property BOOL selectedPalette;
@property BOOL coordinateMap;
@property BOOL sparseMarkers;
@property NSArray *markerPoints;
@property CGFloat markerRadius;
@property NSArray *markerColor;
@property NSString *scanAxis;
@property CGFloat scanCoordinate;
@end
@implementation ProbeContentView
- (instancetype)initWithFrame:(CGRect)frame { if ((self=[super initWithFrame:frame])) { self.opaque=NO; self.backgroundColor=UIColor.clearColor; self.contentMode=UIViewContentModeRedraw; } return self; }
- (void)drawRect:(CGRect)rect {
    if (self.coordinateMap) { [[UIImage imageWithContentsOfFile:[NSBundle.mainBundle.bundlePath stringByAppendingPathComponent:@"probe-map.png"]] drawInRect:self.bounds]; return; }
    CGContextRef c=UIGraphicsGetCurrentContext(); CGContextTranslateCTM(c,64,64);
    if (self.scanAxis) {
        UIColor *ink=self.traitCollection.userInterfaceStyle==UIUserInterfaceStyleDark?UIColor.whiteColor:UIColor.blackColor;
        CGContextSetFillColorWithColor(c,ink.CGColor);
        CGRect band=[self.scanAxis isEqual:@"x"]?CGRectMake(self.scanCoordinate,-64,1,204):CGRectMake(-64,self.scanCoordinate,768,1);
        CGContextFillRect(c,band); return;
    }
    if (self.sparseMarkers) {
        NSArray *colors=@[UIColor.redColor,UIColor.greenColor,UIColor.blueColor,UIColor.magentaColor,UIColor.cyanColor,UIColor.yellowColor,[UIColor colorWithRed:1 green:.4 blue:0 alpha:1],[UIColor colorWithRed:.5 green:0 blue:1 alpha:1]];
        NSArray *points=self.markerPoints?:@[@[@260,@38],@[@280,@12],@[@300,@64],@[@320,@-8],@[@340,@84],@[@360,@12],@[@380,@64],@[@400,@38]];
        CGFloat r=self.markerRadius>0?self.markerRadius:3;
        for (NSUInteger i=0;i<points.count;i++) { CGFloat x=[points[i][0] doubleValue],y=[points[i][1] doubleValue]; UIColor *ink=self.markerColor.count==3?[UIColor colorWithRed:[self.markerColor[0] doubleValue]/255 green:[self.markerColor[1] doubleValue]/255 blue:[self.markerColor[2] doubleValue]/255 alpha:1]:colors[i%colors.count]; CGContextSetFillColorWithColor(c,ink.CGColor); CGContextFillEllipseInRect(c,CGRectMake(x-r,y-r,r*2,r*2)); }
        return;
    }
    BOOL dark=self.traitCollection.userInterfaceStyle==UIUserInterfaceStyleDark;
    UIColor *grid=dark?[UIColor colorWithWhite:1 alpha:.75]:[UIColor colorWithWhite:0 alpha:.75];
    CGContextSetFillColorWithColor(c,grid.CGColor);
    for (NSInteger x=-64;x<=704;x+=16) CGContextFillRect(c,CGRectMake(x,-64,1,204));
    for (NSInteger y=-64;y<=140;y+=16) CGContextFillRect(c,CGRectMake(-64,y,768,1));
    NSArray<UIColor *> *palette=self.selectedPalette?
        @[[UIColor colorWithRed:0 green:.48 blue:1 alpha:1],[UIColor colorWithRed:1 green:0 blue:.65 alpha:1],[UIColor colorWithRed:0 green:.8 blue:.85 alpha:1]]:
        @[[UIColor colorWithRed:1 green:.15 blue:.05 alpha:1],[UIColor colorWithRed:0 green:.65 blue:.15 alpha:1],[UIColor colorWithRed:.6 green:.15 blue:1 alpha:1]];
    for (NSInteger row=-2;row<6;row++) {
        CGFloat y=12+26*row;
        UIColor *color=palette[(row+6)%3]; CGContextSetStrokeColorWithColor(c,color.CGColor); CGContextSetFillColorWithColor(c,color.CGColor); CGContextSetLineWidth(c,3);
        for (NSInteger col=-2;col<19;col++) {
            CGFloat x=20+40*col;
            CGContextMoveToPoint(c,x-10,y); CGContextAddLineToPoint(c,x+10,y);
            CGContextMoveToPoint(c,x,y-10); CGContextAddLineToPoint(c,x,y+10);
            CGContextStrokePath(c); CGContextFillRect(c,CGRectMake(x-3,y-3,6,6));
        }
    }
    NSDictionary *attributes=@{NSFontAttributeName:[UIFont monospacedSystemFontOfSize:12 weight:UIFontWeightBold],NSForegroundColorAttributeName:dark?UIColor.whiteColor:UIColor.blackColor};
    for (NSInteger row=-2;row<6;row++) for (NSInteger col=-1;col<7;col++) {
        NSString *label=[NSString stringWithFormat:@"E%ld+27",(long)(col+1)];
        [label drawAtPoint:CGPointMake(7+112*col,26*row) withAttributes:attributes];
    }
}
@end

static NSArray *RectArray(CGRect r) {
    if (!isfinite(r.origin.x) || !isfinite(r.origin.y) || !isfinite(r.size.width) || !isfinite(r.size.height)) return @[];
    return @[@(r.origin.x), @(r.origin.y), @(r.size.width), @(r.size.height)];
}
static NSArray *ColorArray(UIColor *color, UITraitCollection *traits) {
    if (!color) return @[];
    CGFloat r=0,g=0,b=0,a=0;
    if (![[color resolvedColorWithTraitCollection:traits] getRed:&r green:&g blue:&b alpha:&a]) return @[];
    return @[@(r),@(g),@(b),@(a)];
}
static id FiniteJSON(id value) {
    if ([value isKindOfClass:NSNumber.class]) return isfinite([value doubleValue])?value:NSNull.null;
    if ([value isKindOfClass:NSArray.class]) { NSMutableArray *result=[NSMutableArray new]; for (id child in value) [result addObject:FiniteJSON(child)]; return result; }
    if ([value isKindOfClass:NSDictionary.class]) { NSMutableDictionary *result=[NSMutableDictionary new]; for (id key in value) result[key]=FiniteJSON(value[key]); return result; }
    return value;
}
static NSDictionary *RuntimeClass(Class cls) {
    if (!cls) return @{ @"available":@NO };
    unsigned count=0; Method *methods=class_copyMethodList(cls,&count);
    NSMutableArray *rows=[NSMutableArray new];
    for (unsigned i=0;i<count;i++) [rows addObject:@{ @"selector":NSStringFromSelector(method_getName(methods[i])), @"encoding":@(method_getTypeEncoding(methods[i])) }];
    free(methods);
    [rows sortUsingComparator:^NSComparisonResult(NSDictionary *a,NSDictionary *b) { return [a[@"selector"] compare:b[@"selector"]]; }];
    NSMutableArray *ivarRows=[NSMutableArray new],*propertyRows=[NSMutableArray new];
    Ivar *ivars=class_copyIvarList(cls,&count);
    for (unsigned i=0;i<count;i++) [ivarRows addObject:@{@"name":@(ivar_getName(ivars[i])),@"type":@(ivar_getTypeEncoding(ivars[i])),@"offset":@(ivar_getOffset(ivars[i]))}];
    free(ivars);
    objc_property_t *properties=class_copyPropertyList(cls,&count);
    for (unsigned i=0;i<count;i++) [propertyRows addObject:@{@"name":@(property_getName(properties[i])),@"attributes":@(property_getAttributes(properties[i]))}];
    free(properties);
    return @{ @"available":@YES, @"class":NSStringFromClass(cls), @"superclass":NSStringFromClass(class_getSuperclass(cls))?:@"", @"methods":rows,@"ivars":ivarRows,@"properties":propertyRows };
}
static NSDictionary *SafeLensGetters(id lens) {
    NSMutableDictionary *values=[NSMutableDictionary new];
    for (NSString *name in @[@"isLifted",@"lifted",@"style",@"liftedContentMode",@"warpsContentBelow",@"restingBackgroundColor",@"restingBackground",@"restingBackgroundView",@"liftedContainerView",@"liftedContentView",@"overridePunchoutView",@"contentWrapper",@"liftPortal",@"glass",@"spec",@"flexInteraction"]) {
        SEL selector=NSSelectorFromString(name);
        if (![lens respondsToSelector:selector]) continue;
        NSMethodSignature *signature=[lens methodSignatureForSelector:selector];
        if (signature.numberOfArguments!=2) continue;
        NSInvocation *call=[NSInvocation invocationWithMethodSignature:signature];
        call.target=lens; call.selector=selector;
        @try {
            [call invoke];
            if (signature.methodReturnType[0]=='@') {
                __unsafe_unretained id result=nil; [call getReturnValue:&result];
                if ([result isKindOfClass:UIColor.class]) values[name]=ColorArray(result,[lens traitCollection]);
                else if ([result isKindOfClass:UIView.class]) {
                    UIView *view=result; NSMutableArray *children=[NSMutableArray new]; for (UIView *child in view.subviews) [children addObject:NSStringFromClass(child.class)];
                    values[name]=@{ @"class":NSStringFromClass(view.class), @"frameInLens":RectArray([view convertRect:view.bounds toView:lens]), @"bounds":RectArray(view.bounds), @"zPosition":@(view.layer.zPosition), @"superclass":view.superview?NSStringFromClass(view.superview.class):@"", @"children":children };
                } else if (result) values[name]=@{ @"class":NSStringFromClass([result class]) };
            } else if (signature.methodReturnLength<=8) {
                unsigned long long value=0; [call getReturnValue:&value]; values[name]=@(value);
            }
        } @catch(NSException *exception) { values[name]=exception.name; }
    }
    return values;
}

// Only observed, zero-argument getters with runtime-checked return encodings.
// These diagnostics never dereference private ivars or invoke native setters.
static id ObservedGetter(id target,NSString *name) {
    SEL selector=NSSelectorFromString(name);
    NSMethodSignature *signature=[target methodSignatureForSelector:selector];
    if (!signature || signature.numberOfArguments!=2) return NSNull.null;
    NSInvocation *call=[NSInvocation invocationWithMethodSignature:signature];call.target=target;call.selector=selector;
    @try {
        [call invoke];const char *type=signature.methodReturnType;
        if (strcmp(type,@encode(double))==0) { double value;[call getReturnValue:&value];return isfinite(value)?@(value):NSNull.null; }
        if (strcmp(type,@encode(BOOL))==0) { BOOL value;[call getReturnValue:&value];return @(value); }
        if (strcmp(type,@encode(NSInteger))==0) { NSInteger value;[call getReturnValue:&value];return @(value); }
        if (strcmp(type,@encode(NSUInteger))==0) { NSUInteger value;[call getReturnValue:&value];return @(value); }
        if (strcmp(type,@encode(CGPoint))==0 || strcmp(type,@encode(CGVector))==0) { CGPoint value;[call getReturnValue:&value];return @[@(value.x),@(value.y)]; }
        if (type[0]=='@') { __unsafe_unretained id value=nil;[call getReturnValue:&value];return value?:NSNull.null; }
    } @catch(NSException *exception) { return @{ @"exception":exception.name }; }
    return NSNull.null;
}
// PTSettings exposes a data-only archive of the applied spec. Invoke this
// observed getter explicitly; never enumerate/invoke settings methods (which
// include reset/default mutators alongside getters).
static NSDictionary *LensSpecValues(id object) {
    if (!object || object==NSNull.null) return @{};
    id values=ObservedGetter(object,@"archiveDictionary");
    return [values isKindOfClass:NSDictionary.class]?Value(values,6):@{};
}
static void CollectLensSettings(UIView *view, NSString *path, NSMutableArray *rows) {
    if([NSStringFromClass(view.class) isEqual:@"_UILiquidLensView"]) {
        id spec=ObservedGetter(view,@"spec"),glass=ObservedGetter(view,@"glass");
        NSMutableDictionary *row=[@{@"path":path,@"identifier":view.accessibilityIdentifier?:@"",@"getters":SafeLensGetters(view),@"spec":Value(spec,6),@"specGetters":LensSpecValues(spec),@"specClass":Metadata([spec class]),@"layerTree":LayerTree(view.layer,16),@"viewTree":Tree(view,12)} mutableCopy];
        if(glass!=NSNull.null) {row[@"glass"]=Value(glass,6);row[@"glassClass"]=Metadata([glass class]);}
        [rows addObject:row];
    }
    NSUInteger index=0;
    for(UIView *child in view.subviews)CollectLensSettings(child,[path stringByAppendingFormat:@".%lu",(unsigned long)index++],rows);
}
static NSDictionary *ObservedFlexSnapshot(UIView *view) {
    id flex=ObservedGetter(view,@"flexInteraction");
    if (flex==NSNull.null || ![NSStringFromClass([flex class]) isEqual:@"_UIFlexInteraction"]) return @{};
    NSMutableDictionary *result=[NSMutableDictionary new];
    for (NSString *name in @[@"preferredActivationMode",@"preferredFlexSources",@"preferredVariant",@"resolvedActivationMode",@"shouldUsePresentationLayers",@"isMatchingRadii",@"translation",@"internalTranslation",@"initialTouchLocation"]) result[name]=ObservedGetter(flex,name);
    id integrator=ObservedGetter(flex,@"velocityIntegrator");NSMutableDictionary *values=[NSMutableDictionary new];
    if (integrator!=NSNull.null) for (NSString *name in @[@"minimumRequiredMovement",@"hysteresisTimeInterval",@"maxNumberOfSamples",@"prefersDirectionlessAcceleration",@"useSampleWindowForDifferentiation",@"hasVelocity",@"position",@"velocity",@"acceleration",@"offset"]) values[name]=ObservedGetter(integrator,name);
    result[@"velocityIntegrator"]=values;
    for (NSString *name in @[@"scaleX",@"scaleY",@"driftX",@"driftY",@"translationY"]) { id value=ObservedGetter(flex,name);result[name]=value==NSNull.null?NSNull.null:ObservedGetter(value,@"value"); }
    return result;
}


@interface LensTouchWindow : UIWindow
@property(nonatomic,copy) void (^observeTouch)(UITouch *touch);
@end
@implementation LensTouchWindow
- (void)sendEvent:(UIEvent *)event {
    for (UITouch *touch in event.allTouches) if (self.observeTouch) self.observeTouch(touch);
    [super sendEvent:event];
}
@end

@interface LensController : UIViewController<UITabBarDelegate>
@property UIView *stage;
@property UIImageView *background;
@property UITabBar *nativeTabs;
@property UISwitch *nativeSwitch;
@property UISlider *nativeSlider;
@property UIVisualEffectView *glass;
@property UIView *normalContent;
@property UIView *selectedContent;
@property UIView *selectedSourceHost;
@property UIView<ResearchLiquidLens> *lens;
@property NSMutableArray<UIView<ResearchLiquidLens> *> *samples;
@property UILabel *status;
@property NSTimer *timer;
@property CADisplayLink *displayLink;
@property NSDictionary *request;
@property NSString *pending;
@property NSMutableArray *frames;
@property CFTimeInterval traceStart;
@property BOOL moving;
@property CFTimeInterval motionStart;
@property CGPoint motionFrom;
@property CGPoint motionTo;
@property CGSize restingSize;
@property CGSize liftedSize;
@property NSDictionary *sourceAssets;
@property NSArray *lensFilterOverrideReport;
@property NSDictionary *motionConfiguration;
@property NSMutableArray *touchEvents, *motionFrames, *motionSchema;
@property NSMutableDictionary *motionLayerIDs;
@property CFTimeInterval motionTouchStart, motionTouchEnded;
@property BOOL motionCompleted;
@property BOOL motionInterrupted;
@property NSMutableArray *motionActions, *motionSnapshots;
@property NSMutableSet *motionCaptureNames;
@property NSMutableDictionary *motionAnimationSamples;
@property NSMutableArray *motionFlexFrames;
@end

@implementation LensController
- (BOOL)prefersStatusBarHidden { return YES; }
- (NSURL *)documents { return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject; }
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor=UIColor.blackColor;
    self.stage=[[UIView alloc] initWithFrame:CGRectMake(0,0,720,560)]; self.stage.clipsToBounds=YES;
    [self.view addSubview:self.stage];
    self.background=[[UIImageView alloc] initWithFrame:self.stage.bounds];
    self.background.layer.magnificationFilter=kCAFilterNearest; self.background.layer.minificationFilter=kCAFilterNearest;
    [self.stage addSubview:self.background];
    self.nativeTabs=[[UITabBar alloc] initWithFrame:CGRectMake(40,20,640,80)]; self.nativeTabs.delegate=self;
    self.nativeTabs.accessibilityIdentifier=@"native-tabs";
    self.nativeTabs.items=@[
        [[UITabBarItem alloc] initWithTitle:@"Home" image:[UIImage systemImageNamed:@"house.fill"] tag:0],
        [[UITabBarItem alloc] initWithTitle:@"Favorites" image:[UIImage systemImageNamed:@"star.fill"] tag:1],
        [[UITabBarItem alloc] initWithTitle:@"Settings" image:[UIImage systemImageNamed:@"gearshape.fill"] tag:2]];
    self.nativeTabs.selectedItem=self.nativeTabs.items.firstObject; [self.stage addSubview:self.nativeTabs];
    self.nativeSwitch=[[UISwitch alloc] initWithFrame:CGRectMake(60,140,0,0)]; self.nativeSwitch.accessibilityIdentifier=@"native-switch";
    [self.stage addSubview:self.nativeSwitch];
    self.nativeSlider=[[UISlider alloc] initWithFrame:CGRectMake(180,132,470,50)]; self.nativeSlider.value=.25; self.nativeSlider.accessibilityIdentifier=@"native-slider";
    [self.stage addSubview:self.nativeSlider];
    UIGlassEffect *effect=[UIGlassEffect effectWithStyle:UIGlassEffectStyleRegular]; effect.interactive=YES;
    self.glass=[[UIVisualEffectView alloc] initWithEffect:effect]; self.glass.frame=CGRectMake(40,250,640,76);
    self.glass.cornerConfiguration=[UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:38]];
    self.glass.layer.zPosition=1;
    [self.stage addSubview:self.glass];
    self.normalContent=[[UIView alloc] initWithFrame:self.glass.bounds]; self.selectedContent=[[UIView alloc] initWithFrame:self.glass.bounds];
    [self populate:self.normalContent selected:NO]; [self populate:self.selectedContent selected:YES];
    [self.glass.contentView addSubview:self.selectedContent];
    self.lens=[self makeLensIn:self.glass.contentView content:self.selectedContent punchout:self.normalContent];
    if (self.lens) { self.lens.accessibilityIdentifier=@"manual-lens"; self.lens.bounds=CGRectMake(0,0,116,56); self.lens.center=CGPointMake(640.0/6,38); [self.glass.contentView addSubview:self.lens]; }
    [self.glass.contentView addSubview:self.normalContent];
    self.samples=[NSMutableArray new];
    NSArray *sizes=@[@[@48,@48],@[@80,@80],@[@120,@64]];
    for (NSUInteger i=0;i<sizes.count;i++) {
        UIView<ResearchLiquidLens> *lens=[self makeLensIn:self.stage content:nil punchout:nil];
        if (!lens) continue;
        NSArray *size=sizes[i]; lens.bounds=CGRectMake(0,0,[size[0] doubleValue],[size[1] doubleValue]); lens.center=CGPointMake(120+240*i,425);
        [self.stage addSubview:lens]; [self.samples addObject:lens];
    }
    self.status=[[UILabel alloc] initWithFrame:CGRectMake(20,520,680,25)]; self.status.font=[UIFont monospacedSystemFontOfSize:11 weight:UIFontWeightRegular]; self.status.accessibilityIdentifier=@"status";
    [self.stage addSubview:self.status];
    [NSFileManager.defaultManager removeItemAtURL:[[self documents] URLByAppendingPathComponent:@"request.json"] error:nil];
    self.displayLink=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)]; self.displayLink.preferredFrameRateRange=CAFrameRateRangeMake(30,60,60); [self.displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    self.frames=[NSMutableArray new];
    __weak LensController *weak=self;
    self.timer=[NSTimer scheduledTimerWithTimeInterval:.05 repeats:YES block:^(NSTimer *timer){[weak poll];}];
}
- (void)populate:(UIView *)view selected:(BOOL)selected {
    NSArray *icons=@[@"house.fill",@"star.fill",@"gearshape.fill"],*titles=@[@"Home",@"Favorites",@"Settings"];
    for (NSUInteger i=0;i<3;i++) {
        CGFloat x=640.0/3*i;
        UIImageView *icon=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:icons[i]]]; icon.contentMode=UIViewContentModeScaleAspectFit; icon.frame=CGRectMake(x+87,13,40,25); icon.tintColor=selected?[UIColor colorWithRed:0 green:122./255 blue:1 alpha:1]:UIColor.labelColor;
        UILabel *label=[[UILabel alloc] initWithFrame:CGRectMake(x,44,640.0/3,18)]; label.text=titles[i]; label.textAlignment=NSTextAlignmentCenter; label.font=[UIFont systemFontOfSize:11 weight:UIFontWeightSemibold]; label.textColor=icon.tintColor;
        [view addSubview:icon]; [view addSubview:label];
    }
}
- (UIView<ResearchLiquidLens> *)makeLensIn:(UIView *)container content:(UIView *)content punchout:(UIView *)punchout {
    Class cls=NSClassFromString(@"_UILiquidLensView");
    if (!cls || ![cls instancesRespondToSelector:@selector(initWithRestingBackground:)]) return nil;
    UIView *resting=[LensRestingBackground new]; resting.backgroundColor=[UIColor colorWithWhite:0 alpha:.1];
    UIView<ResearchLiquidLens> *lens=(UIView<ResearchLiquidLens> *)[(id<ResearchLiquidLens>)[cls alloc] initWithRestingBackground:resting];
    lens.userInteractionEnabled=NO; lens.layer.zPosition=10;
    if ([lens respondsToSelector:@selector(setLiftedContainerView:)]) [lens setLiftedContainerView:container];
    if (content && [lens respondsToSelector:@selector(setLiftedContentView:)]) [lens setLiftedContentView:content];
    if (punchout && [lens respondsToSelector:@selector(setOverridePunchoutView:)]) [lens setOverridePunchoutView:punchout];
    if ([lens respondsToSelector:@selector(setLiftedContentMode:)]) [lens setLiftedContentMode:1];
    if ([lens respondsToSelector:@selector(setStyle:)]) [lens setStyle:1];
    if ([lens respondsToSelector:@selector(setWarpsContentBelow:)]) [lens setWarpsContentBelow:YES];
    if ([lens respondsToSelector:@selector(setRestingBackgroundColor:)]) [lens setRestingBackgroundColor:[UIColor colorWithWhite:0 alpha:.1]];
    return lens;
}
- (ProbeContentView *)probeRowSelected:(BOOL)selected {
    ProbeContentView *row=[[ProbeContentView alloc] initWithFrame:CGRectMake(-64,-64,768,204)];
    row.selectedPalette=selected && ![self.request[@"samePalette"] boolValue];
    row.coordinateMap=[self.request[@"probePattern"] isEqual:@"map"];
    row.sparseMarkers=[self.request[@"probePattern"] isEqual:@"markers"];
    row.markerPoints=self.request[@"markerPoints"]; row.markerRadius=[self.request[@"markerRadius"] doubleValue];
    row.markerColor=self.request[@"markerColor"];
    if ([self.request[@"probePattern"] hasPrefix:@"scan-"]) { row.scanAxis=[self.request[@"probePattern"] substringFromIndex:5]; row.scanCoordinate=[self.request[@"scanCoordinate"] doubleValue]; }
    row.overrideUserInterfaceStyle=self.overrideUserInterfaceStyle;
    row.accessibilityIdentifier=selected?@"probe-selected-source":@"probe-normal-source";
    return row;
}
- (void)configureProbe {
    [self.lens removeFromSuperview]; self.lens=nil;
    [self.normalContent removeFromSuperview]; [self.selectedContent removeFromSuperview]; [self.selectedSourceHost removeFromSuperview]; self.selectedSourceHost=nil;
    NSString *base=self.request[@"base"]?:@"regular";
    if ([base isEqual:@"none"]) self.glass.effect=nil;
    else { UIGlassEffect *effect=[UIGlassEffect effectWithStyle:[base isEqual:@"clear"]?UIGlassEffectStyleClear:UIGlassEffectStyleRegular]; effect.interactive=NO; self.glass.effect=effect; }
    self.glass.clipsToBounds=NO; self.glass.contentView.clipsToBounds=NO;
    self.normalContent=[self probeRowSelected:NO]; self.selectedContent=[self probeRowSelected:YES];
    NSString *setup=self.request[@"contentSetup"]?:@"portal";
    BOOL portal=[setup isEqual:@"portal"]||[setup isEqual:@"portal-only"];
    self.normalContent.hidden=[setup isEqual:@"portal-only"]||[setup isEqual:@"none"]||[setup isEqual:@"source-selected"];
    self.selectedContent.hidden=!(portal||[setup isEqual:@"source-selected"]);
    if (self.request[@"sourceHostAlpha"]) {
        self.selectedSourceHost=[[UIView alloc] initWithFrame:self.glass.bounds]; self.selectedSourceHost.alpha=[self.request[@"sourceHostAlpha"] doubleValue];
        [self.selectedSourceHost addSubview:self.selectedContent]; [self.glass.contentView addSubview:self.selectedSourceHost];
    } else [self.glass.contentView addSubview:self.selectedContent];
    UIView *container=self.glass.contentView;
    if ([self.request[@"liftContainer"] isEqual:@"glass"]) container=self.glass;
    else if ([self.request[@"liftContainer"] isEqual:@"stage"]) container=self.stage;
    self.lens=[self makeLensIn:container content:portal?self.selectedContent:nil punchout:[setup isEqual:@"portal"]?self.normalContent:nil];
    self.lens.accessibilityIdentifier=@"manual-lens";
    [self.glass.contentView addSubview:self.lens]; [self.glass.contentView addSubview:self.normalContent];
    [self.lens setLiftedContentMode:self.request[@"contentMode"]?[self.request[@"contentMode"] integerValue]:1];
    self.lens.hidden=[self.request[@"hideLens"] boolValue]||[setup hasPrefix:@"source-"];
    self.nativeTabs.hidden=YES; self.nativeSwitch.hidden=YES; self.nativeSlider.hidden=YES;
    for (UIView *sample in self.samples) sample.hidden=YES;
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.stage.frame=CGRectMake(floor((self.view.bounds.size.width-720)/2),floor((self.view.bounds.size.height-560)/2),720,560);
}
- (NSDictionary *)viewRow:(UIView *)view path:(NSString *)path {
    NSMutableDictionary *row=[@{ @"path":path, @"class":NSStringFromClass(view.class), @"frame":RectArray([view convertRect:view.bounds toView:self.stage]), @"bounds":RectArray(view.bounds), @"alpha":@(view.alpha), @"hidden":@(view.hidden), @"clipsToBounds":@(view.clipsToBounds), @"identifier":view.accessibilityIdentifier?:@"", @"background":ColorArray(view.backgroundColor,view.traitCollection), @"layerClass":NSStringFromClass(view.layer.class), @"cornerRadius":@(view.layer.cornerRadius), @"zPosition":@(view.layer.zPosition) } mutableCopy];
    CALayer *p=view.layer.presentationLayer;
    if (p) { row[@"presentationBounds"]=RectArray(p.bounds); row[@"presentationTransform"]=@[@(p.transform.m11),@(p.transform.m12),@(p.transform.m21),@(p.transform.m22),@(p.transform.m41),@(p.transform.m42)]; }
    if ([NSStringFromClass(view.class) containsString:@"Lens"]) row[@"getters"]=SafeLensGetters(view);
    return row;
}
- (void)walk:(UIView *)view path:(NSString *)path into:(NSMutableArray *)rows lensesOnly:(BOOL)only insideLens:(BOOL)inside {
    BOOL lens=inside || [NSStringFromClass(view.class) containsString:@"Lens"];
    if (!only || lens) [rows addObject:[self viewRow:view path:path]];
    NSUInteger index=0;
    for (UIView *child in view.subviews) { [self walk:child path:[path stringByAppendingFormat:@".%lu",(unsigned long)index++] into:rows lensesOnly:only insideLens:lens]; }
}
- (void)tick:(CADisplayLink *)link {
    if (self.motionConfiguration) { [self motionTick]; return; }
    if (!self.request) return;
    if (self.moving) {
        CGFloat duration=self.request[@"duration"]?[self.request[@"duration"] doubleValue]:.35;
        CGFloat t=MIN(1,MAX(0,(CACurrentMediaTime()-self.motionStart)/MAX(.05,duration)));
        CGFloat u=.5-.5*cos(t*M_PI);
        self.lens.center=CGPointMake(self.motionFrom.x+(self.motionTo.x-self.motionFrom.x)*u,self.motionFrom.y+(self.motionTo.y-self.motionFrom.y)*u);
        if (t>=1) {
            self.moving=NO;
            if ([self.lens respondsToSelector:@selector(setLifted:animated:alongsideAnimations:completion:)]) {
                __weak LensController *weak=self;
                [self.lens setLifted:NO animated:YES alongsideAnimations:^{weak.lens.bounds=(CGRect){CGPointZero,weak.restingSize};} completion:nil];
            }
        }
    }
    if (![self.request[@"settings"] boolValue] && self.frames.count<360) {
        NSMutableArray *rows=[NSMutableArray new]; [self walk:self.stage path:@"stage" into:rows lensesOnly:YES insideLens:NO];
        [self.frames addObject:@{ @"t":@(CACurrentMediaTime()-self.traceStart), @"lenses":rows }];
    }
}
- (void)poll {
    if (self.motionConfiguration) return;
    NSData *data=[NSData dataWithContentsOfURL:[[self documents] URLByAppendingPathComponent:@"request.json"]];
    if (!data) return;
    NSDictionary *request=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if (![request isKindOfClass:NSDictionary.class] || [request isEqual:self.request]) return;
    self.pending=request[@"captureNonce"]?:request[@"id"]; self.request=request; [self.frames removeAllObjects]; self.traceStart=CACurrentMediaTime();
    self.overrideUserInterfaceStyle=[request[@"dark"] boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
    NSString *name=request[@"background"]?:@"gray";
    self.background.image=[UIImage imageWithContentsOfFile:[NSBundle.mainBundle.bundlePath stringByAppendingPathComponent:[name stringByAppendingPathExtension:@"png"]]];
    if ([request[@"probe"] isEqual:@"edges"]) [self configureProbe];
    NSInteger selected=MAX(0,MIN(2,[request[@"selected"] integerValue])); self.nativeTabs.selectedItem=self.nativeTabs.items[selected];
    [self.nativeSwitch setOn:[request[@"on"] boolValue] animated:NO]; self.nativeSlider.value=request[@"value"]?[request[@"value"] floatValue]:.25;
    self.restingSize=CGSizeMake(request[@"width"]?[request[@"width"] doubleValue]:116,request[@"height"]?[request[@"height"] doubleValue]:56);
    CGFloat lift=request[@"liftInset"]?[request[@"liftInset"] doubleValue]:4;
    self.liftedSize=CGSizeMake(self.restingSize.width+lift*2,self.restingSize.height+lift*2);
    CGFloat position=request[@"position"]?[request[@"position"] doubleValue]:selected;
    CGFloat centerX=request[@"centerX"]?[request[@"centerX"] doubleValue]:(position+.5)*640/3;
    CGFloat centerY=request[@"centerY"]?[request[@"centerY"] doubleValue]:38;
    self.lens.bounds=(CGRect){CGPointZero,self.restingSize}; self.lens.center=CGPointMake(centerX,centerY);
    BOOL lifted=[request[@"lifted"] boolValue];
    NSInteger style=request[@"style"]?[request[@"style"] integerValue]:1;
    for (UIView<ResearchLiquidLens> *lens in self.samples) {
        if ([lens respondsToSelector:@selector(setStyle:)]) [lens setStyle:style];
        if ([lens respondsToSelector:@selector(setLifted:animated:alongsideAnimations:completion:)]) [lens setLifted:lifted animated:NO alongsideAnimations:^{} completion:nil];
    }
    if ([self.lens respondsToSelector:@selector(setWarpsContentBelow:)]) [self.lens setWarpsContentBelow:request[@"warps"]?[request[@"warps"] boolValue]:YES];
    if ([self.lens respondsToSelector:@selector(setStyle:)]) [self.lens setStyle:request[@"style"]?[request[@"style"] integerValue]:1];
    if ([self.lens respondsToSelector:@selector(setLiftedContentMode:)]) [self.lens setLiftedContentMode:request[@"contentMode"]?[request[@"contentMode"] integerValue]:1];
    BOOL motion=[request[@"action"] isEqual:@"motion"];
    if ([self.lens respondsToSelector:@selector(setLifted:animated:alongsideAnimations:completion:)]) {
        __weak LensController *weak=self;
        [self.lens setLifted:lifted||motion animated:motion alongsideAnimations:^{ weak.lens.bounds=(CGRect){CGPointZero,(lifted||motion)?weak.liftedSize:weak.restingSize}; } completion:nil];
    }
    if ([request[@"probe"] isEqual:@"edges"]) for (UIView *sample in self.samples) sample.hidden=YES;
    self.moving=motion;
    if (motion) { self.motionStart=CACurrentMediaTime()+.25; self.motionFrom=self.lens.center; self.motionTo=CGPointMake((MAX(0,MIN(2,[request[@"to"] integerValue]))+.5)*640/3,38); }
    self.status.text=@"recording";
    NSString *pending=self.pending;
    self.lensFilterOverrideReport=nil;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)((motion?2.5:1.3)*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        if (![pending isEqual:self.pending]) return;
        if (!self.request[@"lensFilterOverrides"]) { [self finish]; return; }
        [self applyLensFilterOverrides];
        [CATransaction flush];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
            if ([pending isEqual:self.pending]) [self finish];
        });
    });
}
- (void)applyLensFilterOverrides {
    // Diagnostic ablation only. No selector enumeration, arbitrary key paths,
    // or portable implementation dependency: these routes are explicit.
    NSDictionary *routes=@{
        @"backdrop":@[@".0.0",@"displacementMap",@"inputAmount"],
        @"wrapper":@[@".0.2.0.0",@"displacementMap",@"inputAmount"],
        @"source":@[@".0.2.1.0.0.1",@"displacementMap",@"inputAmount"],
        @"foreground":@[@".0.2.1.0.0.2",@"glassForeground",@"inputAberrationAmount"],
        @"inner":@[@".0.2.1.0.0.0",@"glassBackground",@"inputInnerRefractionAmount"],
        @"foregroundMargin":@[@".0.2.1.0.0.2",@"glassForeground",@"marginWidth"]};
    id requested=self.request[@"lensFilterOverrides"];
    NSMutableArray *report=[NSMutableArray new];
    if (![requested isKindOfClass:NSDictionary.class]) {
        self.lensFilterOverrideReport=@[@{@"error":@"lensFilterOverrides must be a dictionary"}]; return;
    }
    for (NSString *name in [[requested allKeys] sortedArrayUsingSelector:@selector(compare:)]) {
        NSArray *route=routes[name]; id number=requested[name];
        NSMutableDictionary *row=[@{@"name":name,@"requested":number?:NSNull.null} mutableCopy];
        [report addObject:row];
        if (!route || ![number isKindOfClass:NSNumber.class] || !isfinite([number doubleValue])) {
            row[@"error"]=@"unrecognized override or non-finite scalar"; continue;
        }
        BOOL margin=[name isEqual:@"foregroundMargin"];
        if (margin && ([number doubleValue]<0 || [number doubleValue]>300)) {
            row[@"error"]=@"foregroundMargin must be within0...300";continue;
        }
        row[@"targetObject"]=margin?@"layer":@"filter";
        row[@"path"]=route[0];row[@"filterType"]=route[1];row[@"key"]=route[2];
        CALayer *layer=self.lens.layer;
        for (NSString *component in [route[0] componentsSeparatedByString:@"."]) {
            if (!component.length) continue;
            NSUInteger index=component.integerValue;
            if (index>=layer.sublayers.count) { layer=nil;break; }
            layer=layer.sublayers[index];
        }
        if (!layer) { row[@"error"]=@"layer path absent";continue; }
        NSMutableArray *matches=[NSMutableArray new];
        @try {
            for (id filter in layer.filters)
                if ([[filter valueForKey:@"type"] isEqual:route[1]]) [matches addObject:filter];
            if (matches.count!=1) { row[@"error"]=@"expected exactly one matching filter";continue; }
            id original=matches.firstObject;
            id filter=[original copy];
            row[@"copiedFilter"] = @(filter!=original);
            id target=margin?layer:filter;
            row[@"before"]=[target valueForKey:route[2]]?:NSNull.null;
            [target setValue:number forKey:route[2]];
            NSMutableArray *replacements=[layer.filters mutableCopy];
            [replacements replaceObjectAtIndex:[replacements indexOfObjectIdenticalTo:original] withObject:filter];
            // A direct CAFilter mutation can change model readback without
            // changing the rendered node. Replace the filter identity and
            // explicitly clear the old array for this diagnostic ablation.
            layer.filters=nil;
            layer.filters=replacements;
            id applied=[target valueForKey:route[2]];
            row[@"applied"]=applied?:NSNull.null;
            row[@"verified"]=@([applied isKindOfClass:NSNumber.class] && fabs([applied doubleValue]-[number doubleValue])<1e-9);
        } @catch (NSException *exception) { row[@"error"]=exception.reason?:@"native filter override exception"; }
    }
    self.lensFilterOverrideReport=report;
}
- (void)finish {
    if ([self.request[@"probe"] isEqual:@"edges"]) {
        NSMutableDictionary *assets=[NSMutableDictionary new];
        for (NSNumber *selected in @[@NO,@YES]) {
            UIView *row=[self probeRowSelected:selected.boolValue]; row.frame=(CGRect){CGPointZero,row.bounds.size}; [self.stage addSubview:row]; [row layoutIfNeeded];
            UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat preferredFormat]; format.scale=2; format.opaque=NO; format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
            UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithSize:row.bounds.size format:format] imageWithActions:^(UIGraphicsImageRendererContext *context) { [row drawViewHierarchyInRect:row.bounds afterScreenUpdates:YES]; }];
            NSString *kind=selected.boolValue?@"selected":@"normal";
            NSString *filename=[NSString stringWithFormat:@"probe-%@-%@.png",self.pending,kind];
            [UIImagePNGRepresentation(image) writeToURL:[[self documents] URLByAppendingPathComponent:filename] atomically:YES]; assets[kind]=filename; [row removeFromSuperview];
        }
        self.sourceAssets=assets;
    }
    if ([self.request[@"exportContent"] boolValue]) {
        // Export fresh public UIKit rows, independent of private punchout and
        // portal state. Comparison assets only; not resources for the Demo.
        for (NSNumber *selected in @[@NO,@YES]) {
            UIView *row=[[UIView alloc] initWithFrame:CGRectMake(0,0,640,76)];
            row.overrideUserInterfaceStyle=self.overrideUserInterfaceStyle;
            [self populate:row selected:selected.boolValue]; [self.stage addSubview:row];
            [row layoutIfNeeded];
            UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat preferredFormat];
            format.scale=2; format.opaque=NO; format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
            UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithSize:row.bounds.size format:format] imageWithActions:^(UIGraphicsImageRendererContext *context) {
                [row drawViewHierarchyInRect:row.bounds afterScreenUpdates:YES];
            }];
            NSString *name=[NSString stringWithFormat:@"content-dark%d-%@.png",[self.request[@"dark"] boolValue],selected.boolValue?@"selected":@"normal"];
            [UIImagePNGRepresentation(image) writeToURL:[[self documents] URLByAppendingPathComponent:name] atomically:YES];
            [row removeFromSuperview];
        }
    }
    NSMutableArray *hierarchy=[NSMutableArray new]; [self walk:self.stage path:@"stage" into:hierarchy lensesOnly:NO insideLens:NO];
    NSMutableDictionary *classes=[NSMutableDictionary new];
    for (NSString *name in @[@"_UILiquidLensView",@"_UILensView",@"_UIFluidLensView",@"_UIFlexInteraction",@"_UIVelocityIntegrator",@"_TtC5UIKit28_UIDebouncingAnimatableFloat",@"_UIFlexInteractionPanGestureRecognizer"]) classes[name]=RuntimeClass(NSClassFromString(name));
    NSDictionary *metadata=@{ @"request":self.request, @"x":@(self.stage.frame.origin.x), @"y":@(self.stage.frame.origin.y), @"width":@720, @"height":@560, @"scale":@(self.view.window.screen.scale), @"os":UIDevice.currentDevice.systemVersion, @"osBuild":NSProcessInfo.processInfo.operatingSystemVersionString, @"classes":classes, @"hierarchy":hierarchy, @"manualLens":self.lens?SafeLensGetters(self.lens):@{}, @"manualBounds":self.lens?RectArray(self.lens.bounds):@[], @"trace":self.frames, @"sourceAssets":self.sourceAssets?:@{}, @"sourceFrameInBar":@[@-64,@-64,@768,@204], @"sourceOriginInStage":@[@-24,@186], @"featureGeometry":@{@"coordinates":@"bar points; source asset point=(bar point +64,+64)",@"gridStep":@16,@"gridWidth":@1,@"crossCenters":@"(20+40*column,12+26*row)",@"crossHalfExtent":@10,@"crossStroke":@3,@"textOrigin":@"(7+112*column,26*row)",@"textFont":@"12pt bold monospaced system",@"overscan":@64,@"markerPoints":self.request[@"markerPoints"]?:@[@[@260,@38],@[@280,@12],@[@300,@64],@[@320,@-8],@[@340,@84],@[@360,@12],@[@380,@64],@[@400,@38]],@"markerRadius":self.request[@"markerRadius"]?:@3,@"markerRGB":@[@[@255,@0,@0],@[@0,@255,@0],@[@0,@0,@255],@[@255,@0,@255],@[@0,@255,@255],@[@255,@255,@0],@[@255,@102,@0],@[@128,@0,@255]]}, @"geometryNote":@"Manual +liftInset bounds are caller supplied, not measured automatic inflation." };
    if (self.lensFilterOverrideReport) {
        NSMutableDictionary *extended=[metadata mutableCopy];extended[@"lensFilterOverridesApplied"]=self.lensFilterOverrideReport;metadata=extended;
    }
    if ([self.request[@"settings"] boolValue]) {
        NSMutableArray *settings=[NSMutableArray new];CollectLensSettings(self.stage,@"stage",settings);
        NSMutableDictionary *extended=[metadata mutableCopy];extended[@"lensSettings"]=settings;
        NSNumber*(*getTint)(void)=dlsym(RTLD_DEFAULT,"UIViewGlassGetTintAmount");
        extended[@"effectiveTint"]=getTint?(getTint()?:NSNull.null):NSNull.null;metadata=extended;
    }
    NSData *data=[NSJSONSerialization dataWithJSONObject:FiniteJSON(metadata) options:NSJSONWritingPrettyPrinted error:nil];
    if (data) [data writeToURL:[[self documents] URLByAppendingPathComponent:@"capture.json"] atomically:YES];
    self.status.text=data?@"complete":@"json-error";
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    NSString *encoded=NSProcessInfo.processInfo.environment[@"LENS_MOTION_CASE"];
    if (!encoded || self.motionConfiguration) return;
    NSDictionary *configuration=[NSJSONSerialization JSONObjectWithData:[encoded dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
    if (![configuration isKindOfClass:NSDictionary.class]) return;
    self.motionConfiguration=configuration;
    self.overrideUserInterfaceStyle=[configuration[@"dark"] boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
    if ([configuration[@"phoneTabs"] boolValue]) {
        CGFloat width=self.view.bounds.size.width;
        self.nativeTabs.frame=CGRectMake((720-width)/2,120,width,83);
        self.nativeTabs.items=@[
            [[UITabBarItem alloc] initWithTitle:@"Home" image:[UIImage systemImageNamed:@"house.fill"] tag:0],
            [[UITabBarItem alloc] initWithTitle:@"Favorites" image:[UIImage systemImageNamed:@"star.fill"] tag:1],
            [[UITabBarItem alloc] initWithTitle:@"Library" image:[UIImage systemImageNamed:@"books.vertical.fill"] tag:2],
            [[UITabBarItem alloc] initWithTitle:@"Settings" image:[UIImage systemImageNamed:@"slider.horizontal.3"] tag:3]];
    }
    NSArray *background=configuration[@"backgroundRGB"];
    if (background.count==3) self.background.backgroundColor=[UIColor colorWithRed:[background[0] doubleValue]/255 green:[background[1] doubleValue]/255 blue:[background[2] doubleValue]/255 alpha:1];
    [self.glass removeFromSuperview];
    for (UIView *sample in self.samples) [sample removeFromSuperview];
    self.nativeTabs.selectedItem=self.nativeTabs.items[MAX(0,MIN(self.nativeTabs.items.count-1,[configuration[@"selected"] integerValue]))];
    self.nativeSwitch.on=[configuration[@"on"] boolValue];
    self.nativeSlider.value=configuration[@"value"]?[configuration[@"value"] floatValue]:.25;
    NSString *target=configuration[@"target"]?:@"tabs";
    self.nativeTabs.hidden=![target isEqual:@"tabs"];
    self.nativeSwitch.hidden=![target isEqual:@"switch"];
    self.nativeSlider.hidden=![target isEqual:@"slider"];
    if ([target isEqual:@"tabs"]) self.nativeTabs.accessibilityIdentifier=@"target";
    if ([target isEqual:@"switch"]) self.nativeSwitch.accessibilityIdentifier=@"target";
    if ([target isEqual:@"slider"]) self.nativeSlider.accessibilityIdentifier=@"target";
    self.motionFrames=[NSMutableArray new]; self.touchEvents=[NSMutableArray new]; self.motionSchema=[NSMutableArray new]; self.motionLayerIDs=[NSMutableDictionary new]; self.motionActions=[NSMutableArray new]; self.motionSnapshots=[NSMutableArray new]; self.motionCaptureNames=[NSMutableSet new];
    self.motionAnimationSamples=[NSMutableDictionary new];
    self.motionFlexFrames=[NSMutableArray new];
    self.displayLink.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);
    __weak LensController *weak=self;
    ((LensTouchWindow *)self.view.window).observeTouch=^(UITouch *touch){ [weak motionObserve:touch]; };
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,.3*NSEC_PER_SEC),dispatch_get_main_queue(),^{
        if ([configuration[@"snapshots"] boolValue]) [weak motionSnapshot:@"rest"];
        weak.status.text=@"ready";
    });
}
- (void)motionObserve:(UITouch *)touch {
    if (self.motionCompleted || touch.phase==UITouchPhaseStationary) return;
    if (!self.motionTouchStart) { if (touch.phase!=UITouchPhaseBegan) return; self.motionTouchStart=touch.timestamp; NSMutableArray *initial=[NSMutableArray new]; [self motionCollect:self.stage.layer path:@"stage" rows:initial]; [self.motionFrames addObject:@{@"t":@0,@"layers":initial}]; }
    CGPoint point=[touch locationInView:self.stage];
    [self.touchEvents addObject:@{@"t":@(touch.timestamp-self.motionTouchStart),@"dispatchT":@(CACurrentMediaTime()-self.motionTouchStart),@"phase":@(touch.phase),@"point":@[@(point.x),@(point.y)],@"targetClass":NSStringFromClass(touch.view.class)?:@"",@"targetIdentifier":touch.view.accessibilityIdentifier?:@""}];
    if (touch.phase==UITouchPhaseEnded || touch.phase==UITouchPhaseCancelled) self.motionTouchEnded=CACurrentMediaTime();
}
- (void)motionCollect:(CALayer *)model path:(NSString *)path rows:(NSMutableArray *)rows {
    NSValue *identity=[NSValue valueWithNonretainedObject:model]; NSNumber *identifier=self.motionLayerIDs[identity];
    if (!identifier) {
        identifier=@(self.motionSchema.count); self.motionLayerIDs[identity]=identifier;
        UIView *view=[model.delegate isKindOfClass:UIView.class]?(UIView *)model.delegate:nil;
        NSMutableArray *interactions=[NSMutableArray new]; for (id interaction in view.interactions) [interactions addObject:NSStringFromClass([interaction class])];
        [self.motionSchema addObject:@{@"id":identifier,@"path":path,@"class":NSStringFromClass(model.class),@"name":model.name?:@"",@"viewClass":view?NSStringFromClass(view.class):@"",@"viewIdentifier":view.accessibilityIdentifier?:@"",@"interactions":interactions}];
    }
    CALayer *layer=model.presentationLayer?:model;
    CALayer *root=model.presentationLayer?(self.stage.layer.presentationLayer?:self.stage.layer):self.stage.layer;
    CGRect frame=[root convertRect:layer.bounds fromLayer:layer]; CATransform3D t=layer.transform,m=model.transform;
    if ([self.motionConfiguration[@"flexDiagnostics"] boolValue] && [model.delegate isKindOfClass:UIView.class] && [NSStringFromClass([model.delegate class]) isEqual:@"_UILiquidLensView"]) {
        [self.motionFlexFrames addObject:@{@"t":@(CACurrentMediaTime()-self.motionTouchStart),@"layerID":identifier,@"values":ObservedFlexSnapshot(model.delegate)}];
    }
    if ([self.motionConfiguration[@"animationDiagnostics"] boolValue] && !self.motionAnimationSamples[identifier.stringValue] && (fabs(t.m11-1)>.001 || fabs(t.m22-1)>.001)) {
        NSMutableArray *animations=[NSMutableArray new];
        for (NSString *key in model.animationKeys) {
            CAAnimation *animation=[model animationForKey:key];
            NSMutableDictionary *data=[@{@"key":key,@"class":NSStringFromClass(animation.class),@"duration":@(animation.duration),@"beginTime":@(animation.beginTime),@"speed":@(animation.speed),@"timeOffset":@(animation.timeOffset),@"removedOnCompletion":@(animation.removedOnCompletion),@"fillMode":animation.fillMode?:@""} mutableCopy];
            if ([animation isKindOfClass:CAPropertyAnimation.class]) { CAPropertyAnimation *property=(id)animation;data[@"keyPath"]=property.keyPath?:@"";data[@"additive"]=@(property.additive);data[@"cumulative"]=@(property.cumulative); }
            if ([animation isKindOfClass:CABasicAnimation.class]) { CABasicAnimation *basic=(id)animation;data[@"fromValue"]=basic.fromValue?[basic.fromValue description]:@"nil";data[@"toValue"]=basic.toValue?[basic.toValue description]:@"nil";data[@"byValue"]=basic.byValue?[basic.byValue description]:@"nil"; }
            if ([animation isKindOfClass:CASpringAnimation.class]) { CASpringAnimation *spring=(id)animation;data[@"mass"]=@(spring.mass);data[@"stiffness"]=@(spring.stiffness);data[@"damping"]=@(spring.damping);data[@"initialVelocity"]=@(spring.initialVelocity);data[@"settlingDuration"]=@(spring.settlingDuration); }
            [animations addObject:data];
        }
        self.motionAnimationSamples[identifier.stringValue]=@{@"t":@(CACurrentMediaTime()-self.motionTouchStart),@"path":path,@"animations":animations};
    }
    [rows addObject:@[identifier,@(frame.origin.x),@(frame.origin.y),@(frame.size.width),@(frame.size.height),@(layer.bounds.origin.x),@(layer.bounds.origin.y),@(layer.bounds.size.width),@(layer.bounds.size.height),@(t.m11),@(t.m12),@(t.m21),@(t.m22),@(t.m41),@(t.m42),@(model.bounds.size.width),@(model.bounds.size.height),@(m.m11),@(m.m12),@(m.m21),@(m.m22),@(m.m41),@(m.m42),@(layer.position.x),@(layer.position.y),@(model.position.x),@(model.position.y)]];
    NSArray *children=model.sublayers; for (NSUInteger i=0;i<children.count;i++) [self motionCollect:children[i] path:[path stringByAppendingFormat:@".%lu",(unsigned long)i] rows:rows];
}
- (void)motionSnapshot:(NSString *)name {
    if ([self.motionCaptureNames containsObject:name]) return;
    [self.motionCaptureNames addObject:name];
    CFTimeInterval start=self.motionTouchStart?CACurrentMediaTime()-self.motionTouchStart:0;
    UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat preferredFormat]; format.scale=2; format.opaque=NO; format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
    __block BOOL success=NO;
    UIImage *image=[[[UIGraphicsImageRenderer alloc] initWithSize:self.stage.bounds.size format:format] imageWithActions:^(UIGraphicsImageRendererContext *context){ success=[self.stage drawViewHierarchyInRect:self.stage.bounds afterScreenUpdates:NO]; }];
    NSURL *directory=[[self documents] URLByAppendingPathComponent:[@"motion-" stringByAppendingString:self.motionConfiguration[@"id"]]]; [[NSFileManager defaultManager] createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil];
    [UIImagePNGRepresentation(image) writeToURL:[directory URLByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
    [self.motionSnapshots addObject:@{@"name":name,@"startedT":@(start),@"completedT":@(self.motionTouchStart?CACurrentMediaTime()-self.motionTouchStart:0),@"touchActive":@(self.motionTouchStart&&!self.motionTouchEnded),@"success":@(success)}];
}
- (void)motionTick {
    if (!self.motionTouchStart || self.motionCompleted) return;
    CFTimeInterval time=CACurrentMediaTime()-self.motionTouchStart;
    if (!self.motionInterrupted && self.motionConfiguration[@"cancelAt"] && time>=[self.motionConfiguration[@"cancelAt"] doubleValue]) {
        self.motionInterrupted=YES;
        NSString *kind=self.motionConfiguration[@"target"]?:@"tabs";
        UIView *target=[kind isEqual:@"tabs"]?self.nativeTabs:([kind isEqual:@"switch"]?self.nativeSwitch:self.nativeSlider);
        target.userInteractionEnabled=NO;
        [self.motionActions addObject:@{@"t":@(time),@"action":@"Disable target userInteractionEnabled; UIKit owns cancellation response"}];
    }
    if ([self.motionConfiguration[@"snapshots"] boolValue]) {
        NSArray *times=self.motionConfiguration[@"snapshotTimes"]?:@[@.25,@.65,@1.0,@1.4,@1.8];
        for (NSUInteger i=0;i<times.count;i++) if (time>=[times[i] doubleValue]) [self motionSnapshot:[NSString stringWithFormat:@"contact-%lu",(unsigned long)i]];
        if (self.motionTouchEnded) {
            const double release[]={.016,.066,.15,.3,.6,1.0};
            for (NSUInteger i=0;i<6;i++) if (CACurrentMediaTime()-self.motionTouchEnded>=release[i]) [self motionSnapshot:[NSString stringWithFormat:@"release-%lu",(unsigned long)i]];
        }
    }
    NSMutableArray *rows=[NSMutableArray new]; [self motionCollect:self.stage.layer path:@"stage" rows:rows];
    [self.motionFrames addObject:@{@"t":@(CACurrentMediaTime()-self.motionTouchStart),@"layers":rows}];
    if (self.motionTouchEnded && CACurrentMediaTime()-self.motionTouchEnded>1.5) {
        self.motionCompleted=YES;
        NSMutableArray *hierarchy=[NSMutableArray new]; [self walk:self.stage path:@"stage" into:hierarchy lensesOnly:NO insideLens:NO];
        NSDictionary *trace=@{@"configuration":self.motionConfiguration,@"evidenceType":@"Actual UIKit controls responding to XCTest-delivered OS touch events; no authored lens geometry after setup",@"os":UIDevice.currentDevice.systemVersion,@"osBuild":NSProcessInfo.processInfo.operatingSystemVersionString,@"scale":@(self.view.window.screen.scale),@"stageInWindow":RectArray([self.stage convertRect:self.stage.bounds toView:self.view.window]),@"clockOrigin":@(self.motionTouchStart),@"touchEvents":self.touchEvents,@"frames":self.motionFrames,@"nativeActions":self.motionActions,@"snapshots":self.motionSnapshots,@"schema":self.motionSchema,@"hierarchy":hierarchy,@"columns":@[@"layerID",@"presentationStageX",@"presentationStageY",@"presentationStageWidth",@"presentationStageHeight",@"presentationBoundsX",@"presentationBoundsY",@"presentationBoundsWidth",@"presentationBoundsHeight",@"presentationM11",@"presentationM12",@"presentationM21",@"presentationM22",@"presentationM41",@"presentationM42",@"modelBoundsWidth",@"modelBoundsHeight",@"modelM11",@"modelM12",@"modelM21",@"modelM22",@"modelM41",@"modelM42",@"presentationPositionX",@"presentationPositionY",@"modelPositionX",@"modelPositionY"]};
        NSURL *directory=[[self documents] URLByAppendingPathComponent:[@"motion-" stringByAppendingString:self.motionConfiguration[@"id"]]]; [[NSFileManager defaultManager] createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil];
        NSMutableDictionary *withDiagnostics=[trace mutableCopy];withDiagnostics[@"animationDiagnostics"]=self.motionAnimationSamples?:@{};
        withDiagnostics[@"flexDiagnostics"]=self.motionFlexFrames?:@[];
        NSData *data=[NSJSONSerialization dataWithJSONObject:FiniteJSON(withDiagnostics) options:0 error:nil]; [data writeToURL:[directory URLByAppendingPathComponent:@"trace.json"] atomically:YES]; self.status.text=@"complete";
    }
}
- (void)tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item { self.status.text=[NSString stringWithFormat:@"native-tab-%ld",(long)item.tag]; }
@end

@interface LensScene : UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation LensScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window=[[LensTouchWindow alloc] initWithWindowScene:(UIWindowScene *)scene]; self.window.rootViewController=[LensController new]; [self.window makeKeyAndVisible];
}
@end
@interface LensApp : UIResponder<UIApplicationDelegate>
@end
@implementation LensApp
- (UISceneConfiguration *)application:(UIApplication *)app configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options { UISceneConfiguration *c=[[UISceneConfiguration alloc] initWithName:@"Lens" sessionRole:session.role]; c.delegateClass=LensScene.class; return c; }
@end
int main(int argc,char **argv) { @autoreleasepool {
    // Research-only, process-local preference; never change persistent settings.
    NSString *amount=NSProcessInfo.processInfo.environment[@"LENS_TINT_AMOUNT"];
    NSUserDefaults*(*defaults)(void)=dlsym(RTLD_DEFAULT,"_UIKitUserDefaults");
    if(amount&&defaults) {
        NSMutableDictionary *domain=[[defaults() volatileDomainForName:NSArgumentDomain] mutableCopy]?:[NSMutableDictionary new];
        domain[@"UIViewGlassTintAmount"]=@([amount doubleValue]);
        [defaults() setVolatileDomain:domain forName:NSArgumentDomain];
    }
    return UIApplicationMain(argc,argv,nil,NSStringFromClass(LensApp.class));
} }
