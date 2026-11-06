// Native UIKit clone of the eight-arrangement navigation playground.
// Only public UIKit/QuartzCore APIs are invoked. Runtime names are observations.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

static NSArray *Names(void) { return @[@"Shared icons",@"Accent action",@"Fixed-space split",@"Three shared items",@"Text buttons",@"Leading items",@"Unshared item",@"Hidden glass"]; }
static NSArray *Titles(void) { return @[@"Shared",@"Accent",@"Separated",@"Three items",@"Text",@"Leading",@"Unshared",@"Hidden glass"]; }
static NSArray *RectValues(CGRect r) { return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)]; }
static UIImage *Symbol(NSString *name,CGFloat size,UIImageSymbolWeight weight) {
    return [UIImage systemImageNamed:name withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:size weight:weight scale:UIImageSymbolScaleMedium]];
}
@interface CycleScreen:UITableViewController
@property NSInteger arrangement;
@property NSString *status;
@property NSUInteger presses;
- (instancetype)initWithArrangement:(NSInteger)value;
@end
@implementation CycleScreen
- (UIBarButtonItem*)image:(NSString*)name identifier:(NSString*)identifier title:(NSString*)title {
    UIBarButtonItem *item=[[UIBarButtonItem alloc]initWithImage:Symbol(name,20,UIImageSymbolWeightRegular) style:UIBarButtonItemStylePlain target:self action:@selector(barTap:)];
    item.title=title;item.identifier=identifier;return item;
}
- (instancetype)initWithArrangement:(NSInteger)value {
    if(!(self=[super initWithStyle:UITableViewStyleInsetGrouped]))return nil;
    _arrangement=value;_status=@"Tap a navigation item to test its glass.";
    self.title=value<0?@"Home":Titles()[value];
    UIBarButtonItem *add=[self image:@"plus" identifier:@"demo.add" title:@"Add"],*more=[self image:@"ellipsis" identifier:@"demo.more" title:@"More"];
    NSArray *right=@[add,more];
    if(value<0) {
        UIBarButtonItem *close=[[UIBarButtonItem alloc]initWithImage:Symbol(@"chevron.left",20,UIImageSymbolWeightSemibold) style:UIBarButtonItemStylePlain target:self action:@selector(close:)];close.title=@"Back";self.navigationItem.leftBarButtonItem=close;
        add=[[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(barTap:)];add.identifier=@"demo.add";add.style=UIBarButtonItemStyleProminent;
        more.image=Symbol(@"ellipsis",17,UIImageSymbolWeightSemibold);right=@[add,more];
    }
    if(value==1)add.style=UIBarButtonItemStyleProminent;
    if(value==2)right=@[add,[UIBarButtonItem fixedSpaceItemOfWidth:0],more];
    if(value==3)right=@[add,more,[self image:@"trash" identifier:@"demo.delete" title:@"Delete"]];
    if(value==4) {
        add=[[UIBarButtonItem alloc]initWithTitle:@"Edit" style:UIBarButtonItemStylePlain target:self action:@selector(barTap:)];add.identifier=@"demo.add";
        more=[[UIBarButtonItem alloc]initWithTitle:@"Save" style:UIBarButtonItemStylePlain target:self action:@selector(barTap:)];more.identifier=@"demo.more";right=@[add,more];
    }
    if(value==5){self.navigationItem.leftItemsSupplementBackButton=YES;self.navigationItem.leftBarButtonItem=[self image:@"star" identifier:@"demo.favorite" title:@"Favorite"];}
    if(value==6)add.sharesBackground=NO;
    if(value==7)add.hidesSharedBackground=YES;
    self.navigationItem.rightBarButtonItems=right;
    return self;
}
- (void)viewDidLoad { [super viewDidLoad];self.tableView.rowHeight=52;self.view.backgroundColor=UIColor.systemGroupedBackgroundColor; }
- (void)close:(id)sender { [self barTap:sender]; }
- (void)barTap:(UIBarButtonItem*)item { self.status=[NSString stringWithFormat:@"%@ tapped · %lu",item.title?:@"Action",(unsigned long)++self.presses];[self.tableView reloadData]; }
- (NSInteger)numberOfSectionsInTableView:(UITableView*)table { return self.arrangement<0?2:3; }
- (NSInteger)tableView:(UITableView*)table numberOfRowsInSection:(NSInteger)section {
    if(self.arrangement<0)return section==0?3:1;
    return section==0?1:(section==1?2:8);
}
- (NSString*)tableView:(UITableView*)table titleForHeaderInSection:(NSInteger)section {
    if(self.arrangement<0)return section==0?@"Home collection":@"Navigation";
    return section==0?[NSString stringWithFormat:@"%ld of 8 · %@",(long)self.arrangement+1,Names()[self.arrangement]]:(section==2?@"Push any arrangement":nil);
}
- (NSString*)tableView:(UITableView*)table titleForFooterInSection:(NSInteger)section {
    if(self.arrangement>=0&&section==2)return @"Use Back to reverse each morph. Tabs retain their own navigation stacks.";
    return nil;
}
- (UITableViewCell*)tableView:(UITableView*)table cellForRowAtIndexPath:(NSIndexPath*)path {
    UITableViewCell *cell=[[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];NSString *text;
    if(self.arrangement<0)text=path.section==0?[NSString stringWithFormat:@"Item %ld",(long)path.row+1]:@"Navigation morphing";
    else if(path.section==0)text=self.status;
    else if(path.section==1)text=path.row==0?[NSString stringWithFormat:@"Push next: %@",Names()[(self.arrangement+1)%8]]:@"Pop to collection";
    else text=Names()[path.row];
    cell.textLabel.text=text;cell.textLabel.numberOfLines=0;cell.textLabel.font=[UIFont systemFontOfSize:self.arrangement>=0&&path.section==0?13:17];
    cell.textLabel.textColor=self.arrangement>=0&&path.section==0?UIColor.secondaryLabelColor:UIColor.systemBlueColor;
    cell.accessoryType=path.section>0?UITableViewCellAccessoryDisclosureIndicator:UITableViewCellAccessoryNone;return cell;
}
- (void)tableView:(UITableView*)table didSelectRowAtIndexPath:(NSIndexPath*)path {
    [table deselectRowAtIndexPath:path animated:YES];
    if(self.arrangement<0){if(path.section==1)[self.navigationController pushViewController:[[CycleScreen alloc]initWithArrangement:0] animated:YES];return;}
    if(path.section==0)return;
    if(path.section==1&&path.row==1){[self.navigationController popToRootViewControllerAnimated:YES];return;}
    NSInteger next=path.section==1?(self.arrangement+1)%8:path.row;
    [self.navigationController pushViewController:[[CycleScreen alloc]initWithArrangement:next] animated:YES];
}
@end

static UITabBarController *Demo(void) {
    NSMutableArray *controllers=[NSMutableArray new];NSArray *names=@[@"Home",@"Favorites",@"Library",@"Settings"],*icons=@[@"house.fill",@"star.fill",@"books.vertical.fill",@"slider.horizontal.3"];
    for(NSUInteger i=0;i<4;++i){CycleScreen *root=[[CycleScreen alloc]initWithArrangement:-1];root.title=names[i];UINavigationController *nav=[[UINavigationController alloc]initWithRootViewController:root];nav.navigationBar.tintColor=UIColor.systemBlueColor;nav.tabBarItem=[[UITabBarItem alloc]initWithTitle:names[i] image:Symbol(icons[i],28,UIImageSymbolWeightRegular) tag:i];[controllers addObject:nav];}
    UITabBarController *tabs=[UITabBarController new];tabs.viewControllers=controllers;tabs.mode=UITabBarControllerModeTabBar;return tabs;
}
@interface CycleProbe:NSObject
@property UIWindow *window;
@property UITabBarController *tabs;
@property UINavigationController *nav;
@property CADisplayLink *link;
@property NSArray *cases;
@property NSMutableArray *clips,*frames;
@property NSDictionary *current;
@property NSUInteger index,checkpoint;
@property CFTimeInterval origin,lastSample;
@property CycleScreen *source,*target;
@property dispatch_queue_t writer;
@property dispatch_group_t pending;
@end
@implementation CycleProbe
- (NSURL*)directory { NSURL *url=[[[NSFileManager defaultManager]URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:@"cycle"];[NSFileManager.defaultManager createDirectoryAtURL:url withIntermediateDirectories:YES attributes:nil error:nil];return url; }
- (void)later:(double)seconds run:(void(^)(void))run { dispatch_after(dispatch_time(DISPATCH_TIME_NOW,seconds*NSEC_PER_SEC),dispatch_get_main_queue(),run); }
- (void)views:(UIView*)view path:(NSString*)path opacity:(CGFloat)alpha into:(NSMutableArray*)rows {
    if(view.hidden)return;CALayer *layer=view.layer.presentationLayer?:view.layer;alpha*=layer.opacity;if(alpha<.001)return;
    NSString *name=NSStringFromClass(view.class);
    if([name containsString:@"Glass"]||[name containsString:@"Button"]||[name containsString:@"Title"]||[view isKindOfClass:UILabel.class]){
        CALayer *bar=self.nav.navigationBar.layer.presentationLayer?:self.nav.navigationBar.layer;
        NSMutableDictionary *row=[@{@"id":[NSString stringWithFormat:@"%p",view],@"path":path,@"class":name,@"frame":RectValues([layer convertRect:layer.bounds toLayer:bar]),@"alpha":@(alpha)}mutableCopy];
        row[@"label"]=view.accessibilityLabel?:@"";row[@"identifier"]=view.accessibilityIdentifier?:@"";
        if([view isKindOfClass:UILabel.class])row[@"text"]=((UILabel*)view).text?:@"";
        [rows addObject:row];
    }
    NSUInteger i=0;for(UIView *child in view.subviews)[self views:child path:[path stringByAppendingFormat:@"/%lu",(unsigned long)i++] opacity:alpha into:rows];
}
- (NSArray*)contentFrame:(UIViewController*)controller {
    if(!controller.isViewLoaded||!controller.view.window)return @[];
    CALayer *p=controller.view.layer.presentationLayer?:controller.view.layer,*w=self.window.layer.presentationLayer?:self.window.layer;return RectValues([p convertRect:p.bounds toLayer:w]);
}
- (void)sample:(double)t picture:(BOOL)picture {
    NSMutableArray *views=[NSMutableArray new];[self views:self.nav.navigationBar path:@"bar" opacity:1 into:views];
    NSMutableDictionary *frame=[@{@"t":@(t),@"views":views,@"sourceContent":[self contentFrame:self.source],@"targetContent":[self contentFrame:self.target]}mutableCopy];
    if(picture){
        NSString *file=[NSString stringWithFormat:@"%@/frame-%03lu.png",self.current[@"name"],(unsigned long)self.checkpoint++];
        UIGraphicsImageRendererFormat *format=UIGraphicsImageRendererFormat.defaultFormat;format.scale=3;
        CGRect crop=CGRectMake(0,self.nav.navigationBar.frame.origin.y-12,self.window.bounds.size.width,80);
        UIImage *image=[[[UIGraphicsImageRenderer alloc]initWithSize:crop.size format:format]imageWithActions:^(UIGraphicsImageRendererContext *c){CGContextTranslateCTM(c.CGContext,0,-crop.origin.y);[self.window drawViewHierarchyInRect:self.window.bounds afterScreenUpdates:NO];}];
        frame[@"image"]=file;NSURL *url=[[self directory]URLByAppendingPathComponent:file];dispatch_group_async(self.pending,self.writer,^{[UIImagePNGRepresentation(image)writeToURL:url atomically:YES];});
    }
    [self.frames addObject:frame];
}
- (void)tick:(CADisplayLink*)link {
    if(!self.current)return;double t=CACurrentMediaTime()-self.origin;
    if(t-self.lastSample<1./125.)return;self.lastSample=t;
    const double times[]={0,.05,.1,.15,.2,.25,.3,.4,.6,.75};BOOL picture=[NSProcessInfo.processInfo.arguments containsObject:@"--every-frame"]||(self.checkpoint<10&&t>=times[self.checkpoint]);[self sample:t picture:picture];
    if(t>=.75){self.current=nil;self.index++;[self later:.23 run:^{[self prepare];}];}
}
- (void)prepare {
    if(self.index>=self.cases.count){[self.link invalidate];dispatch_group_notify(self.pending,dispatch_get_main_queue(),^{NSDictionary *report=@{@"os":UIDevice.currentDevice.systemVersion,@"scale":@3,@"imageBarOriginY":@12,@"cases":self.cases,@"clips":self.clips};[[NSJSONSerialization dataWithJSONObject:report options:0 error:nil]writeToURL:[[self directory]URLByAppendingPathComponent:@"cycle.json"]atomically:YES];self.window.rootViewController=Demo();});return;}
    NSDictionary *c=self.cases[self.index];self.window.overrideUserInterfaceStyle=[c[@"dark"]boolValue]?UIUserInterfaceStyleDark:UIUserInterfaceStyleLight;
    // A reused UIKit bar can animate material after a nonanimated stack reset.
    // Isolate cases with fresh tab/navigation containers before attachment.
    self.tabs=Demo();self.nav=(UINavigationController*)self.tabs.viewControllers.firstObject;
    CycleScreen *root=(CycleScreen*)self.nav.viewControllers.firstObject;self.source=[c[@"source"]integerValue]<0?root:[[CycleScreen alloc]initWithArrangement:[c[@"source"]integerValue]];self.target=[c[@"target"]integerValue]<0?root:[[CycleScreen alloc]initWithArrangement:[c[@"target"]integerValue]];
    NSMutableArray *stack=[NSMutableArray arrayWithObject:root];
    if([c[@"pop"]boolValue]){if(self.target!=root)[stack addObject:self.target];[stack addObject:self.source];}else if(self.source!=root)[stack addObject:self.source];
    [self.nav setViewControllers:stack animated:NO];self.window.rootViewController=self.tabs;[self.window layoutIfNeeded];
    [self later:.3 run:^{
        self.frames=[NSMutableArray new];self.checkpoint=0;self.current=c;[self.clips addObject:@{@"name":c[@"name"],@"source":c[@"source"],@"target":c[@"target"],@"pop":c[@"pop"],@"dark":c[@"dark"],@"frames":self.frames}];
        [NSFileManager.defaultManager createDirectoryAtURL:[[self directory]URLByAppendingPathComponent:c[@"name"]]withIntermediateDirectories:YES attributes:nil error:nil];
        [self sample:0 picture:YES];self.origin=CACurrentMediaTime();self.lastSample=0;
        if([c[@"pop"]boolValue])[self.nav popViewControllerAnimated:YES];else [self.nav pushViewController:self.target animated:YES];
    }];
}
- (void)start {
    self.tabs=Demo();self.window.rootViewController=self.tabs;[self.window makeKeyAndVisible];self.nav=(UINavigationController*)self.tabs.viewControllers.firstObject;
    NSArray *args=NSProcessInfo.processInfo.arguments;NSUInteger selection=[args indexOfObject:@"--case"];
    if(![args containsObject:@"--capture"]&&selection==NSNotFound)return;
    NSData *data=[NSData dataWithContentsOfFile:[NSBundle.mainBundle pathForResource:@"CycleCases" ofType:@"json"]];NSArray *cases=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if(selection!=NSNotFound){NSString *prefix=args[selection+1];cases=[cases filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(NSDictionary *c,NSDictionary *bindings){return [c[@"name"]hasPrefix:prefix];}]];}
    if([args containsObject:@"--sequence"])cases=[cases filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(NSDictionary *c,NSDictionary *bindings){return [c[@"target"]integerValue]==[c[@"source"]integerValue]+([c[@"pop"]boolValue]?-1:1);}]];
    self.cases=cases;self.clips=[NSMutableArray new];self.writer=dispatch_queue_create("native.cycle.png",DISPATCH_QUEUE_SERIAL);self.pending=dispatch_group_create();
    [NSFileManager.defaultManager removeItemAtURL:[[self directory]URLByAppendingPathComponent:@"cycle.json"]error:nil];
    self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];self.link.preferredFrameRateRange=CAFrameRateRangeMake(60,120,120);[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    [self later:.3 run:^{[self.nav pushViewController:[[CycleScreen alloc]initWithArrangement:5] animated:YES];[self later:1 run:^{[self.nav popViewControllerAnimated:YES];[self later:1 run:^{[self prepare];}];}];}];
}
@end
@interface Scene:UIResponder<UIWindowSceneDelegate>@property UIWindow *window;@property CycleProbe *probe;@end
@implementation Scene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {self.window=[[UIWindow alloc]initWithWindowScene:(UIWindowScene*)scene];self.probe=[CycleProbe new];self.probe.window=self.window;[self.probe start];}
@end
@interface App:UIResponder<UIApplicationDelegate>@end
@implementation App
- (UISceneConfiguration*)application:(UIApplication*)app configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {UISceneConfiguration *configuration=[[UISceneConfiguration alloc]initWithName:@"Cycle" sessionRole:session.role];configuration.delegateClass=Scene.class;return configuration;}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(App.class));}}
