// Independent Apple UIKit adapter of the exact gallery scene definitions.
#import <UIKit/UIKit.h>
#import "../app/Screens/GlassDemoViewController/GlassDemoSpec.h"

@interface GalleryStage : UIView
@property(nonatomic) NSInteger demo, backdrop;
@property(nonatomic) CGFloat progress;
@property(nonatomic) BOOL reversedOrder;
@property(nonatomic,strong) UIImageView *artwork;
@property(nonatomic,strong) NSMutableArray<UIView *> *panels, *symbols;
@property(nonatomic,strong) NSMutableArray<UIVisualEffectView *> *items;
@property(nonatomic,strong) NSMutableArray<UIButton *> *buttons;
@property(nonatomic,strong) UIVisualEffectView *container;
- (instancetype)initWithDemo:(NSInteger)demo;
- (void)reset;
@end
@implementation GalleryStage
- (instancetype)initWithDemo:(NSInteger)demo {
    if (!(self=[super initWithFrame:CGRectMake(0,0,720,440)])) return nil;
    _demo=demo; self.clipsToBounds=YES;
    self.artwork=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"scene.png"]];
    self.artwork.frame=self.bounds; self.artwork.contentMode=UIViewContentModeScaleToFill; [self addSubview:self.artwork];
    self.panels=[NSMutableArray new]; self.items=[NSMutableArray new]; self.symbols=[NSMutableArray new]; self.buttons=[NSMutableArray new];
    for (int i=0;i<3;++i) { UIView *panel=[[UIView alloc] initWithFrame:CGRectMake(i*240,0,240,440)]; [self.panels addObject:panel]; [self addSubview:panel]; }
    UIView *host=self;
    if (GlassDemos[demo].grouped) {
        UIGlassContainerEffect *effect=[UIGlassContainerEffect new]; effect.spacing=GlassDemoContainerSpacing((int)demo);
        UIVisualEffectView *container=[[UIVisualEffectView alloc] initWithEffect:effect]; container.frame=self.bounds;
        [self addSubview:container]; self.container=container; host=container.contentView;
    }
    for (int i=0;i<GlassDemoElementCount((int)demo);++i) {
        GlassDemoElement spec=GlassDemoElementAt((int)demo,i,0);
        UIGlassEffect *effect=[UIGlassEffect effectWithStyle:GlassDemos[demo].clear ? UIGlassEffectStyleClear : UIGlassEffectStyleRegular];
        effect.interactive=GlassDemos[demo].interactive;
        if (spec.tint) { unsigned char r,g,b; GlassDemoTint(spec.tint,&r,&g,&b); effect.tintColor=[UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1]; }
        UIVisualEffectView *item=[[UIVisualEffectView alloc] initWithEffect:effect];
        item.layer.cornerCurve=spec.circular ? kCACornerCurveCircular : kCACornerCurveContinuous;
        [host addSubview:item]; [self.items addObject:item];
        UIView *symbol=[[UIView alloc] initWithFrame:CGRectMake(0,0,20,20)]; symbol.userInteractionEnabled=NO;
        if (GlassDemoHasButtons((int)demo)) {
            UIButton *button=[UIButton buttonWithType:UIButtonTypeCustom];
            button.accessibilityLabel=GlassDemoIsNavigationButtons((int)demo) && i==0 ? @"Back" : (i==1 ? @"More" : @"Add");
            __weak GalleryStage *weak=self;
            [button addAction:[UIAction actionWithHandler:^(UIAction *action) {
                GalleryStage *self=weak; if (!self) return;
                self.buttons[i].selected=!self.buttons[i].selected; [self updateSymbol:i];
            }] forControlEvents:UIControlEventPrimaryActionTriggered];
            [button addSubview:symbol]; [item.contentView addSubview:button]; [self.buttons addObject:button];
        } else {
            for (int axis=0;axis<2;++axis) {
                UIView *stroke=[[UIView alloc] initWithFrame:axis ? CGRectMake(9,0,2,20) : CGRectMake(0,9,20,2)];
                stroke.backgroundColor=UIColor.labelColor; [symbol addSubview:stroke];
            }
            [item.contentView addSubview:symbol];
        }
        [self.symbols addObject:symbol];
        if (GlassDemos[demo].movable) [item addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(pan:)]];
    }
    [self reset]; return self;
}
- (void)setBackdrop:(NSInteger)backdrop {
    _backdrop=backdrop; self.artwork.hidden=backdrop!=0;
    self.backgroundColor=backdrop==2 ? UIColor.blackColor : (backdrop==3 ? UIColor.whiteColor : [UIColor colorWithWhite:31/255.0 alpha:1]);
    for (int i=0;i<3;++i) { self.panels[i].hidden=backdrop!=4; self.panels[i].backgroundColor=i==0 ? UIColor.blackColor : (i==1 ? UIColor.whiteColor : [UIColor colorWithWhite:127/255.0 alpha:1]); }
}
- (void)setProgress:(CGFloat)progress {
    _progress=MIN(1,MAX(0,progress));
    for (int i=0;i<self.items.count;++i) {
        GlassDemoElement e=GlassDemoElementAtSize((int)self.demo,i,(float)_progress,self.bounds.size.width,self.bounds.size.height);
        self.items[i].frame=CGRectMake(e.x,e.y,e.width,e.height); self.items[i].layer.cornerRadius=e.radius;
        self.symbols[i].frame=CGRectMake((e.width-20)*.5,(e.height-20)*.5,20,20);
        if (self.buttons.count) self.buttons[i].frame=CGRectMake(0,0,e.width,e.height);
    }
}
- (void)setReversedOrder:(BOOL)reversedOrder {
    if (_reversedOrder==reversedOrder) return;
    _reversedOrder=reversedOrder;
    NSArray *ordered=reversedOrder ? self.items.reverseObjectEnumerator.allObjects : self.items;
    for (UIView *item in ordered) { UIView *host=item.superview; [item removeFromSuperview]; [host addSubview:item]; }
}
- (void)updateSymbol:(NSInteger)index {
    UIView *symbol=self.symbols[index];
    for (UIView *stroke in symbol.subviews) [stroke removeFromSuperview];
    int kind=self.buttons[index].selected ? GlassDemoCheck : GlassDemoSymbolForItem((int)self.demo,(int)index);
    GlassDemoElement spec=GlassDemoElementAt((int)self.demo,(int)index,0);
    for (int i=0;i<GlassDemoStrokeCount(kind);++i) {
        GlassDemoStroke line=GlassDemoStrokeAt(kind,i);
        UIView *stroke=[[UIView alloc] initWithFrame:CGRectMake(line.x,line.y,line.width,line.height)];
        stroke.layer.cornerRadius=line.radius; stroke.backgroundColor=spec.tint ? UIColor.whiteColor : UIColor.labelColor;
        stroke.transform=CGAffineTransformMakeRotation(line.angle*M_PI/180); [symbol addSubview:stroke];
    }
}
- (void)layoutSubviews {
    [super layoutSubviews];
    if (!GlassDemoHasButtons((int)self.demo)) return;
    self.artwork.frame=self.bounds;
    self.container.frame=self.bounds;
    for (int i=0;i<3;++i) self.panels[i].frame=CGRectMake(i*self.bounds.size.width/3,0,self.bounds.size.width/3,self.bounds.size.height);
    self.progress=_progress;
}
- (void)reset {
    self.reversedOrder=NO; for (UIView *item in self.items) [item.layer removeAllAnimations];
    for (NSInteger i=0;i<self.buttons.count;++i) { self.buttons[i].selected=NO; [self updateSymbol:i]; }
    self.progress=0; self.backdrop=self.demo==GlassDemoContrast26 ? 4 : 0;
}
- (void)pan:(UIPanGestureRecognizer *)pan {
    CGPoint delta=[pan translationInView:self]; CGRect frame=pan.view.frame;
    frame.origin.x=MIN(720-frame.size.width,MAX(0,frame.origin.x+delta.x));
    frame.origin.y=MIN(440-frame.size.height,MAX(0,frame.origin.y+delta.y));
    pan.view.frame=frame; [pan setTranslation:CGPointZero inView:self];
}
@end

@interface GallerySceneController : UIViewController
@property(nonatomic,strong) GalleryStage *stage;
@property(nonatomic,strong) UILabel *detail;
@property(nonatomic,strong) UIStackView *controls;
@property(nonatomic,copy) NSDictionary *request;
- (instancetype)initWithDemo:(NSInteger)demo;
- (void)publish;
@end
@implementation GallerySceneController
- (instancetype)initWithDemo:(NSInteger)demo { if ((self=[super init])) { self.title=@(GlassDemos[demo].title); self.stage=[[GalleryStage alloc] initWithDemo:demo]; } return self; }
- (void)viewDidLoad {
    [super viewDidLoad]; self.view.backgroundColor=UIColor.systemBackgroundColor;
    self.detail=[UILabel new]; self.detail.font=[UIFont systemFontOfSize:18]; self.detail.numberOfLines=0; self.detail.textColor=UIColor.secondaryLabelColor;
    self.detail.text=@(GlassDemos[self.stage.demo].detail); [self.view addSubview:self.detail]; [self.view addSubview:self.stage];
    self.controls=[UIStackView new]; self.controls.axis=UILayoutConstraintAxisHorizontal; self.controls.spacing=12; self.controls.distribution=UIStackViewDistributionFillEqually;
    __weak GallerySceneController *weak=self;
    NSMutableArray *names=[(GlassDemos[self.stage.demo].action[0] ? @[@"Backdrop",@"Reset",@(GlassDemos[self.stage.demo].action)] : @[@"Backdrop",@"Reset"]) mutableCopy];
    if (self.stage.demo==GlassDemoTwoTints) [names addObject:@"Swap order"];
    for (NSInteger i=0;i<names.count;++i) {
        UIButton *button=[UIButton buttonWithType:UIButtonTypeSystem]; button.configuration=UIButtonConfiguration.tintedButtonConfiguration; [button setTitle:names[i] forState:UIControlStateNormal];
        [button addAction:[UIAction actionWithHandler:^(UIAction *action) {
            GallerySceneController *self=weak; if (!self) return;
            if (i==0) self.stage.backdrop=(self.stage.backdrop+1)%5;
            else if (i==1) [self.stage reset];
            else if (i==3) self.stage.reversedOrder=!self.stage.reversedOrder;
            else [UIView animateWithDuration:.7 animations:^{ self.stage.progress=self.stage.progress<.5 ? 1 : 0; }];
        }] forControlEvents:UIControlEventPrimaryActionTriggered]; [self.controls addArrangedSubview:button];
    }
    [self.view addSubview:self.controls];
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews]; UIEdgeInsets safe=self.view.safeAreaInsets;
    CGFloat width=MIN(720,MAX(0,self.view.bounds.size.width-safe.left-safe.right-40)), x=(self.view.bounds.size.width-width)*.5, y=safe.top+24;
    self.detail.frame=CGRectMake(x,y,width,72); y+=88;
    self.stage.transform=CGAffineTransformIdentity;
    BOOL fixedButtons=GlassDemoHasButtons((int)self.stage.demo);
    self.stage.bounds=CGRectMake(0,0,fixedButtons ? width : 720,fixedButtons ? 440*width/720 : 440);
    if (!fixedButtons) self.stage.transform=CGAffineTransformMakeScale(width/720,width/720);
    self.stage.center=CGPointMake(x+width/2,y+440*width/720/2);
    self.controls.frame=CGRectMake(x,y+440*width/720+20,width,56); [self publish];
}
- (void)publish {
    if (!self.view.window) return;
    CGRect frame=[self.stage convertRect:self.stage.bounds toView:self.view.window];
    NSMutableArray *elements=[NSMutableArray new];
    for (UIView *item in self.stage.items) { CGRect f=item.frame; [elements addObject:@{@"x":@(f.origin.x),@"y":@(f.origin.y),@"width":@(f.size.width),@"height":@(f.size.height),@"radius":@(item.layer.cornerRadius)}]; }
    NSDictionary *metadata=@{@"x":@(frame.origin.x),@"y":@(frame.origin.y),@"width":@(frame.size.width),@"scale":@(self.view.window.screen.scale),@"os":UIDevice.currentDevice.systemVersion,@"request":self.request ?: @{},@"elements":elements};
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    [[NSJSONSerialization dataWithJSONObject:metadata options:NSJSONWritingPrettyPrinted error:nil] writeToURL:[documents URLByAppendingPathComponent:@"gallery.json"] atomically:YES];
}
@end

@interface GalleryMenu : UITableViewController
@property(nonatomic,strong) NSTimer *poller;
@property(nonatomic,copy) NSDictionary *request;
@property(nonatomic,strong) GallerySceneController *destination;
@end
@implementation GalleryMenu
- (void)viewDidLoad {
    [super viewDidLoad]; self.title=@"Liquid Glass"; self.tableView.rowHeight=64;
    __weak GalleryMenu *weak=self;
    self.poller=[NSTimer scheduledTimerWithTimeInterval:.1 repeats:YES block:^(NSTimer *timer) { [weak poll]; }];
}
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section { return GlassDemoCount; }
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil]; cell.textLabel.text=@(GlassDemos[indexPath.row].title); cell.accessoryType=UITableViewCellAccessoryDisclosureIndicator; return cell;
}
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath { [self.navigationController pushViewController:[[GallerySceneController alloc] initWithDemo:indexPath.row] animated:YES]; }
- (void)poll {
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSData *data=[NSData dataWithContentsOfURL:[documents URLByAppendingPathComponent:@"request.json"]]; if (!data) return;
    NSDictionary *request=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil]; if (!request || [request isEqual:self.request]) return;
    NSInteger demo=[request[@"case"] integerValue]; if (demo<0 || demo>=GlassDemoCount) return;
    self.request=request;
    if (!self.destination || self.destination.stage.demo!=demo) {
        self.destination=[[GallerySceneController alloc] initWithDemo:demo]; [self.navigationController setViewControllers:@[self,self.destination] animated:NO];
    }
    self.destination.request=request; self.destination.overrideUserInterfaceStyle=[request[@"dark"] boolValue] ? UIUserInterfaceStyleDark : UIUserInterfaceStyleLight;
    [UIView performWithoutAnimation:^{ self.destination.stage.reversedOrder=[request[@"reverse"] boolValue]; self.destination.stage.progress=[request[@"progress"] doubleValue]; self.destination.stage.backdrop=[request[@"backdrop"] integerValue]; }];
    [self.destination.view setNeedsLayout];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{ [self.destination publish]; });
}
@end
@interface GalleryScene : UIResponder <UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation GalleryScene
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController=[[UINavigationController alloc] initWithRootViewController:[GalleryMenu new]]; [self.window makeKeyAndVisible];
}
@end
@interface GalleryApp : UIResponder <UIApplicationDelegate>
@end
@implementation GalleryApp
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *config=[[UISceneConfiguration alloc] initWithName:@"Glass gallery" sessionRole:session.role]; config.delegateClass=GalleryScene.class; return config;
}
@end
int main(int argc,char** argv) { @autoreleasepool { return UIApplicationMain(argc,argv,nil,NSStringFromClass(GalleryApp.class)); } }
