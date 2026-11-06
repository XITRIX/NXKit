// Native UIKit transition test of the production geometry bridge, without a
// simulator Skia archive. No private UIKit or XCTest interfaces are used.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#include "../../UIKitCore/lib/platforms/apple/ios/IOSWindowGeometry.hpp"
#include <algorithm>
#include <cmath>

@interface RotationProbeMetalView : UIView @end
@implementation RotationProbeMetalView
+ (Class)layerClass {return CAMetalLayer.class;}
@end

@interface RotationProbeController : UIViewController {
    std::unique_ptr<NXKit::Internal::IOSWindowGeometry> _geometry;
    CADisplayLink* _link;
    RotationProbeMetalView* _metalView;
    id<MTLCommandQueue> _queue;
    int _drawableErrors, _drawables;
    UIView* _reference;
    UILabel* _status;
    NSMutableArray* _samples;
    CGSize _from, _target;
    int _completed, _intermediate, _scaleErrors;
    double _maximumError;
}
@end
@implementation RotationProbeController
- (void)viewDidLoad {
    [super viewDidLoad];self.view.backgroundColor=UIColor.systemBlueColor;
    _samples=[NSMutableArray array];
    _metalView=[[RotationProbeMetalView alloc] initWithFrame:self.view.bounds];
    _metalView.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    CAMetalLayer* layer=(CAMetalLayer*)_metalView.layer;
    layer.device=MTLCreateSystemDefaultDevice();layer.pixelFormat=MTLPixelFormatBGRA8Unorm;
    layer.contentsGravity=kCAGravityResize;_queue=[layer.device newCommandQueue];
    [self.view addSubview:_metalView];
    _reference=[[UIView alloc] initWithFrame:CGRectZero];_reference.userInteractionEnabled=NO;[self.view addSubview:_reference];
    _status=[[UILabel alloc] initWithFrame:CGRectMake(20,100,350,220)];_status.numberOfLines=0;
    _status.textColor=UIColor.whiteColor;_status.accessibilityIdentifier=@"rotation-status";[self.view addSubview:_status];
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    if(_geometry)return;
    _geometry=std::make_unique<NXKit::Internal::IOSWindowGeometry>((__bridge void*)self.view.window);
    _link=[CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];
    [_link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];[self updateReport];
}
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    if(!_geometry)return;
    auto frame=_geometry->sample();_from=CGSizeMake(frame.width,frame.height);_target=size;
    [UIView performWithoutAnimation:^{self->_reference.bounds=CGRectMake(0,0,self->_from.width,self->_from.height);}];
    [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        self->_reference.bounds=CGRectMake(0,0,size.width,size.height);
    } completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        ++self->_completed;[self updateReport];
    }];
}
- (void)tick:(CADisplayLink*)link {
    if(!_geometry)return;
    const auto frame=_geometry->sample();
    const auto native=self.view.bounds.size;
    const auto presentedReference=_reference.layer.presentationLayer;
    const auto reference=presentedReference && presentedReference.bounds.size.width>0 && presentedReference.bounds.size.height>0
        ? presentedReference.bounds.size : _from;
    if(frame.transitioning) {
        const auto error=std::max(std::abs(double(frame.width)-reference.width),std::abs(double(frame.height)-reference.height));
        _maximumError=std::max(_maximumError,error);
        if(std::abs(frame.width-_from.width)>1 && std::abs(frame.width-_target.width)>1)++_intermediate;
    }
    if(std::abs(frame.scale-self.view.window.screen.nativeScale)>.0001)++_scaleErrors;
    CAMetalLayer* layer=(CAMetalLayer*)_metalView.layer;
    const int width=int(std::lround(frame.width*frame.scale)),height=int(std::lround(frame.height*frame.scale));
    [CATransaction begin];[CATransaction setDisableActions:YES];
    layer.contentsScale=frame.scale;layer.drawableSize=CGSizeMake(width,height);[CATransaction commit];
    id<CAMetalDrawable> drawable=[layer nextDrawable];
    if(!drawable || drawable.texture.width!=width || drawable.texture.height!=height)++_drawableErrors;
    if(drawable && _queue) {
        ++_drawables;
        MTLRenderPassDescriptor* pass=[MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].texture=drawable.texture;
        pass.colorAttachments[0].loadAction=MTLLoadActionClear;pass.colorAttachments[0].storeAction=MTLStoreActionStore;
        pass.colorAttachments[0].clearColor=MTLClearColorMake(.05,.2,.45,1);
        id<MTLCommandBuffer> command=[_queue commandBuffer];
        [[command renderCommandEncoderWithDescriptor:pass] endEncoding];[command presentDrawable:drawable];[command commit];
    }
    [_samples addObject:@{@"time":@(link.timestamp),@"width":@(frame.width),@"height":@(frame.height),
        @"modelWidth":@(native.width),@"modelHeight":@(native.height),@"referenceWidth":@(reference.width),
        @"referenceHeight":@(reference.height),@"scale":@(frame.scale),@"transitioning":@(frame.transitioning),
        @"top":@(frame.top),@"left":@(frame.left),@"bottom":@(frame.bottom),@"right":@(frame.right)}];
}
- (void)updateReport {
    NSDictionary* summary=@{@"completed":@(_completed),@"intermediate":@(_intermediate),
        @"maxReferenceError":@(_maximumError),@"scaleErrors":@(_scaleErrors),@"drawableErrors":@(_drawableErrors),@"drawables":@(_drawables)};
    _status.text=[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:summary options:0 error:nil] encoding:NSUTF8StringEncoding];
    NSString* directory=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
    [[NSJSONSerialization dataWithJSONObject:@{@"summary":summary,@"samples":_samples} options:NSJSONWritingPrettyPrinted error:nil]
        writeToFile:[directory stringByAppendingPathComponent:@"rotation.json"] atomically:YES];
}
@end
@interface RotationProbeScene : UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow* window;
@end
@implementation RotationProbeScene
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
    self.window.rootViewController=[[RotationProbeController alloc] init];[self.window makeKeyAndVisible];
}
@end
@interface RotationProbeDelegate : UIResponder<UIApplicationDelegate> @end
@implementation RotationProbeDelegate
- (UISceneConfiguration*)application:(UIApplication*)application configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    UISceneConfiguration* configuration=[[UISceneConfiguration alloc] initWithName:@"Rotation" sessionRole:session.role];
    configuration.delegateClass=RotationProbeScene.class;return configuration;
}
@end
int main(int argc,char**argv) {@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(RotationProbeDelegate.class));}}
