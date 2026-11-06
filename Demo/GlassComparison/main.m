// Apple's UIKit, deliberately built separately from NXKit's UIKitCore.
#import <UIKit/UIKit.h>
#import "Scene.h"

@interface GlassReferenceController : UIViewController
@property(nonatomic, strong) UIView *scene;
@property(nonatomic, strong) NSMutableArray<UIVisualEffectView *> *glasses;
@property(nonatomic, strong) NSTimer *requestTimer;
@property(nonatomic, copy) NSString *tintName;
@property(nonatomic, copy) NSString *settledID;
@property(nonatomic, copy) NSString *pendingID;
@property(nonatomic) CGFloat sizeFraction;
@property(nonatomic, strong) UIImageView *background;
@property(nonatomic, strong) UIImage *artwork;
@property(nonatomic, copy) NSString *backgroundName;
@property(nonatomic) BOOL showsLabels;
@end

@implementation GlassReferenceController
- (BOOL)prefersStatusBarHidden { return YES; }
- (void)viewDidLoad {
    [super viewDidLoad];
    BOOL dark = [NSProcessInfo.processInfo.arguments containsObject:@"--dark"];
    self.tintName = @"none";
    self.sizeFraction = 1;
    self.settledID = @"initial";
    self.showsLabels = [NSProcessInfo.processInfo.arguments containsObject:@"--labels"];
    self.backgroundName = @"artwork";
    for (NSString *argument in NSProcessInfo.processInfo.arguments) {
        if ([argument hasPrefix:@"--tint="]) self.tintName = [argument substringFromIndex:7];
        if ([argument hasPrefix:@"--size="]) self.sizeFraction = [[argument substringFromIndex:7] doubleValue];
    }
    self.overrideUserInterfaceStyle = dark ? UIUserInterfaceStyleDark : UIUserInterfaceStyleLight;
    self.view.backgroundColor = UIColor.blackColor;
    self.scene = [[UIView alloc] initWithFrame:CGRectMake(0, 0, GlassSceneWidth, GlassSceneHeight)];
    self.scene.clipsToBounds = YES;
    [self.view addSubview:self.scene];
    UIImageView *background = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"scene.png"]];
    self.background = background;
    self.artwork = background.image;
    background.frame = self.scene.bounds;
    background.contentMode = UIViewContentModeScaleToFill;
    background.layer.magnificationFilter = kCAFilterNearest;
    background.layer.minificationFilter = kCAFilterNearest;
    [self.scene addSubview:background];
    for (NSString *name in @[@"black", @"white", @"gray"]) {
        if ([NSProcessInfo.processInfo.arguments containsObject:[@"--" stringByAppendingString:name]]) {
            self.backgroundName = name;
        }
    }
    [self applyBackground:self.backgroundName];
    if ([NSProcessInfo.processInfo.arguments containsObject:@"--no-glass"]) return;
    self.glasses = [NSMutableArray new];
    for (unsigned i = 0; i < sizeof(GlassSceneElements) / sizeof(GlassSceneElements[0]); ++i) {
        GlassSceneElement element = GlassElementAtSize(i, self.sizeFraction);
        UIGlassEffect *effect = [UIGlassEffect effectWithStyle:element.clear ? UIGlassEffectStyleClear : UIGlassEffectStyleRegular];
        effect.interactive = YES;
        if ([self comparisonTint]) effect.tintColor = [self comparisonTint];
        UIVisualEffectView *glass = [[UIVisualEffectView alloc] initWithEffect:effect];
        glass.frame = CGRectMake(element.x, element.y, element.width, element.height);
        glass.cornerConfiguration = [UICornerConfiguration configurationWithUniformRadius:[UICornerRadius fixedRadius:element.radius]];
        [self.scene addSubview:glass];
        [self.glasses addObject:glass];
        if (self.showsLabels) {
            UILabel *label = [[UILabel alloc] initWithFrame:glass.bounds];
            label.text = @"Glass";
            label.textAlignment = NSTextAlignmentCenter;
            label.font = [UIFont systemFontOfSize:18 weight:UIFontWeightSemibold];
            label.textColor = UIColor.labelColor;
            label.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [glass.contentView addSubview:label];
        }
    }
    NSURL *requestURL = [[self documentsURL] URLByAppendingPathComponent:@"request.json"];
    [NSFileManager.defaultManager removeItemAtURL:requestURL error:nil];
    __weak GlassReferenceController *weakSelf = self;
    self.requestTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 repeats:YES block:^(NSTimer *timer) {
        [weakSelf readComparisonRequest];
    }];
}
- (NSURL *)documentsURL {
    return [NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
}
- (UIColor *)comparisonTint {
    if ([self.tintName isEqualToString:@"blue"]) return [UIColor colorWithRed:0 green:122.0/255 blue:1 alpha:1];
    if ([self.tintName isEqualToString:@"coral"]) return [UIColor colorWithRed:1 green:69.0/255 blue:58.0/255 alpha:1];
    return nil;
}
- (void)applyBackground:(NSString *)name {
    self.backgroundName = name;
    self.background.hidden = ![@[@"artwork", @"checker"] containsObject:name];
    if ([name isEqualToString:@"artwork"]) self.background.image = self.artwork;
    else if ([name isEqualToString:@"checker"]) {
        UIGraphicsImageRendererFormat *format = [UIGraphicsImageRendererFormat defaultFormat];
        format.scale = 1;
        self.background.image = [[[UIGraphicsImageRenderer alloc] initWithSize:self.scene.bounds.size format:format]
            imageWithActions:^(UIGraphicsImageRendererContext *context) {
                for (int y = 0; y < GlassSceneHeight; y += 20)
                    for (int x = 0; x < GlassSceneWidth; x += 20) {
                        [((x / 20 + y / 20) % 2 ? UIColor.blackColor : UIColor.whiteColor) setFill];
                        UIRectFill(CGRectMake(x, y, 20, 20));
                    }
            }];
    } else self.scene.backgroundColor = [name isEqualToString:@"black"] ? UIColor.blackColor
        : [name isEqualToString:@"white"] ? UIColor.whiteColor : [UIColor colorWithWhite:0.5 alpha:1];
}
- (void)readComparisonRequest {
    NSData *data = [NSData dataWithContentsOfURL:[[self documentsURL] URLByAppendingPathComponent:@"request.json"]];
    if (!data) return;
    NSDictionary *request = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if (![request isKindOfClass:NSDictionary.class]) return;
    NSString *identifier = request[@"id"];
    if (![identifier isKindOfClass:NSString.class] || [identifier isEqual:self.pendingID]) return;
    if (![request[@"size"] isKindOfClass:NSNumber.class]) return;
    CGFloat size = [request[@"size"] doubleValue];
    if (!isfinite(size) || size < 0 || size > 1) return;
    NSString *tint = request[@"tint"];
    if (![@[@"none", @"blue", @"coral"] containsObject:tint]) return;
    self.pendingID = identifier;
    NSString *backgroundName = request[@"background"];
    if ([@[@"artwork", @"black", @"white", @"gray", @"checker"] containsObject:backgroundName])
        [self applyBackground:backgroundName];
    self.sizeFraction = size;
    if (![self.tintName isEqual:tint]) {
        self.tintName = tint;
        for (unsigned i = 0; i < self.glasses.count; ++i) {
            UIGlassEffect *effect = [UIGlassEffect effectWithStyle:GlassSceneElements[i].clear ? UIGlassEffectStyleClear : UIGlassEffectStyleRegular];
            effect.interactive = YES;
            if ([self comparisonTint]) effect.tintColor = [self comparisonTint];
            self.glasses[i].effect = effect;
        }
    }
    // Resize these same effect views, rather than recreating one per sample.
    void (^resize)(void) = ^{
        for (unsigned i = 0; i < self.glasses.count; ++i) {
            GlassSceneElement element = GlassElementAtSize(i, size);
            self.glasses[i].frame = CGRectMake(element.x, element.y, element.width, element.height);
        }
    };
    if (request[@"animate"] && ![request[@"animate"] boolValue]) resize();
    else [UIView animateWithDuration:0.3 delay:0 options:UIViewAnimationOptionBeginFromCurrentState | UIViewAnimationOptionAllowUserInteraction animations:resize completion:nil];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.8 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if ([self.pendingID isEqual:identifier]) {
            self.settledID = identifier;
            [self writeCaptureMetadata];
        }
    });
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    // An unscaled 720 x 440 point scene, independent of the simulator's screen.
    self.scene.frame = CGRectMake(floor((self.view.bounds.size.width - GlassSceneWidth) / 2),
                                  floor((self.view.bounds.size.height - GlassSceneHeight) / 2),
                                  GlassSceneWidth, GlassSceneHeight);
    NSLog(@"GLASS_SCENE %@ scale %.1f OS %@", NSStringFromCGRect(self.scene.frame),
          self.view.window.screen.scale, UIDevice.currentDevice.systemVersion);
    [self writeCaptureMetadata];
}
- (void)writeCaptureMetadata {
    NSMutableArray *elements = [NSMutableArray new];
    for (unsigned i = 0; i < self.glasses.count; ++i) {
        GlassSceneElement e = GlassElementAtSize(i, self.sizeFraction);
        [elements addObject:@[@(e.x), @(e.y), @(e.width), @(e.height), @(e.radius), @(e.clear)]];
    }
    UIUserInterfaceStyle effectiveStyle = self.glasses.count > 1
        ? self.glasses[1].traitCollection.userInterfaceStyle : self.view.traitCollection.userInterfaceStyle;
    NSMutableArray *contentStyles = [NSMutableArray new];
    NSMutableArray *foregroundColors = [NSMutableArray new];
    for (UIVisualEffectView *glass in self.glasses) {
        [contentStyles addObject:glass.contentView.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark ? @"dark" : @"light"];
        UIColor *foreground = [UIColor.labelColor resolvedColorWithTraitCollection:glass.contentView.traitCollection];
        CGFloat r = 0, g = 0, b = 0, a = 0;
        [foreground getRed:&r green:&g blue:&b alpha:&a];
        [foregroundColors addObject:@[@(r), @(g), @(b), @(a)]];
    }
    NSDictionary *metadata = @{@"x": @(self.scene.frame.origin.x), @"y": @(self.scene.frame.origin.y),
        @"width": @(GlassSceneWidth), @"height": @(GlassSceneHeight), @"scale": @(self.view.window.screen.scale),
        @"os": UIDevice.currentDevice.systemVersion, @"arguments": NSProcessInfo.processInfo.arguments,
        @"osBuild": NSProcessInfo.processInfo.operatingSystemVersionString,
        @"appearance": self.overrideUserInterfaceStyle == UIUserInterfaceStyleDark ? @"dark" : @"light",
        @"effectiveAppearance": effectiveStyle == UIUserInterfaceStyleDark ? @"dark" : @"light",
        @"size": @(self.sizeFraction), @"tint": self.tintName, @"id": self.settledID,
        @"background": self.backgroundName, @"labels": @(self.showsLabels),
        @"contentAppearances": contentStyles, @"resolvedLabelColors": foregroundColors,
        @"elements": elements,
        @"reduceTransparency": @(UIAccessibilityIsReduceTransparencyEnabled()),
        @"darkerSystemColors": @(UIAccessibilityDarkerSystemColorsEnabled())};
    NSURL *directory = [self documentsURL];
    [[NSJSONSerialization dataWithJSONObject:metadata options:NSJSONWritingPrettyPrinted error:nil]
        writeToURL:[directory URLByAppendingPathComponent:@"capture.json"] atomically:YES];
}
@end

@interface GlassSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation GlassSceneDelegate
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [GlassReferenceController new];
    [self.window makeKeyAndVisible];
}
@end

@interface GlassAppDelegate : UIResponder <UIApplicationDelegate>
@end
@implementation GlassAppDelegate
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    UISceneConfiguration *configuration = [[UISceneConfiguration alloc] initWithName:@"Glass comparison" sessionRole:session.role];
    configuration.delegateClass = GlassSceneDelegate.class;
    return configuration;
}
@end

int main(int argc, char **argv) {
    @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(GlassAppDelegate.class)); }
}
