#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import <objc/message.h>

@interface LensMotionUITests : XCTestCase
@end
@implementation LensMotionUITests
- (void)sendPath:(NSArray *)points from:(XCUICoordinate *)start {
    // Independent native test plumbing; selector names are research evidence,
    // not an implementation copied from WebDriverAgent or production code.
    Class pathClass=NSClassFromString(@"XCPointerEventPath"),recordClass=NSClassFromString(@"XCSynthesizedEventRecord"),sessionClass=NSClassFromString(@"XCTRunnerDaemonSession");
    SEL initPath=NSSelectorFromString(@"initForTouchAtPoint:offset:"),move=NSSelectorFromString(@"moveToPoint:atOffset:"),lift=NSSelectorFromString(@"liftUpAtOffset:"),initRecord=NSSelectorFromString(@"initWithName:interfaceOrientation:"),add=NSSelectorFromString(@"addPointerEventPath:"),shared=NSSelectorFromString(@"sharedSession"),synthesize=NSSelectorFromString(@"synthesizeEvent:completion:");
    for (NSArray *pair in @[@[pathClass?:NSObject.class,NSStringFromSelector(initPath)],@[pathClass?:NSObject.class,NSStringFromSelector(move)],@[pathClass?:NSObject.class,NSStringFromSelector(lift)],@[recordClass?:NSObject.class,NSStringFromSelector(initRecord)],@[recordClass?:NSObject.class,NSStringFromSelector(add)],@[sessionClass?:NSObject.class,NSStringFromSelector(synthesize)]]) {
        Method method=class_getInstanceMethod(pair[0],NSSelectorFromString(pair[1]));
        XCTAssertNotEqual(method,NULL); NSLog(@"LENS_EVENT_ABI %@ %@ %s",NSStringFromClass(pair[0]),pair[1],method?method_getTypeEncoding(method):"missing");
    }
    XCTAssertTrue([sessionClass respondsToSelector:shared]);
    CGPoint origin=start.screenPoint;
    id path=((id(*)(id,SEL,CGPoint,double))objc_msgSend)([pathClass alloc],initPath,origin,0);
    for (NSArray *point in points) ((void(*)(id,SEL,CGPoint,double))objc_msgSend)(path,move,CGPointMake(origin.x+[point[1] doubleValue],origin.y+[point[2] doubleValue]),[point[0] doubleValue]);
    ((void(*)(id,SEL,double))objc_msgSend)(path,lift,[points.lastObject[0] doubleValue]+.2);
    id record=((id(*)(id,SEL,id,NSInteger))objc_msgSend)([recordClass alloc],initRecord,@"NXKit native lens reversal",UIInterfaceOrientationPortrait);
    ((void(*)(id,SEL,id))objc_msgSend)(record,add,path);
    // The current XCTest event synthesizer reports (BOOL,NSError *).
    // Treating its BOOL result as an object corrupts the completion callback.
    SEL synthesizerGetter=NSSelectorFromString(@"eventSynthesizer");
    XCTAssertTrue([XCUIDevice.sharedDevice respondsToSelector:synthesizerGetter]);
    id synthesizer=((id(*)(id,SEL))objc_msgSend)(XCUIDevice.sharedDevice,synthesizerGetter);
    Method method=class_getInstanceMethod([synthesizer class],synthesize);
    XCTAssertNotEqual(method,NULL);
    NSLog(@"LENS_EVENT_ABI %@ %@ %s",NSStringFromClass([synthesizer class]),NSStringFromSelector(synthesize),method?method_getTypeEncoding(method):"missing");
    XCTestExpectation *done=[self expectationWithDescription:@"OS event sequence completed"];
    ((void(*)(id,SEL,id,id))objc_msgSend)(synthesizer,synthesize,record,^(BOOL success,NSError *error){ XCTAssertTrue(success); XCTAssertNil(error); [done fulfill]; });
    [self waitForExpectations:@[done] timeout:20];
}
- (void)testNativeLensTouchTraces {
    self.continueAfterFailure=NO;
    NSURL *url=[[NSBundle bundleForClass:self.class] URLForResource:@"Cases" withExtension:@"json"];
    NSArray *cases=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:url] options:0 error:nil];
    XCTAssertGreaterThan(cases.count,0);
    for (NSDictionary *configuration in cases) {
        XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.LensMotionReference"];
        app.launchEnvironment=@{@"LENS_MOTION_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};
        [app launch];
        NSString *kind=configuration[@"target"]?:@"tabs";
        XCUIElement *target=[kind isEqual:@"tabs"]?app.tabBars[@"target"]:([kind isEqual:@"switch"]?app.switches[@"target"]:app.sliders[@"target"]);
        XCTAssertTrue([target waitForExistenceWithTimeout:10]);
        XCTNSPredicateExpectation *ready=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'ready'"] object:app.staticTexts[@"status"]];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[ready] timeout:8],XCTWaiterResultCompleted);
        NSArray *point=configuration[@"startNormalized"]?:@[@.5,@.5];
        XCUICoordinate *start=[target coordinateWithNormalizedOffset:CGVectorMake([point[0] doubleValue],[point[1] doubleValue])];
        XCUICoordinate *end=[start coordinateWithOffset:CGVectorMake([configuration[@"dx"] doubleValue],[configuration[@"dy"] doubleValue])];
        if ([configuration[@"tap"] boolValue]) [start tap];
        else if (configuration[@"path"]) [self sendPath:configuration[@"path"] from:start];
        else [start pressForDuration:configuration[@"holdBefore"]?[configuration[@"holdBefore"] doubleValue]:.4 thenDragToCoordinate:end withVelocity:configuration[@"velocity"]?[configuration[@"velocity"] doubleValue]:200 thenHoldForDuration:configuration[@"holdAfter"]?[configuration[@"holdAfter"] doubleValue]:.3];
        XCTNSPredicateExpectation *finished=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'complete'"] object:app.staticTexts[@"status"]];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[finished] timeout:10],XCTWaiterResultCompleted);
        NSLog(@"LENS_MOTION_COMPLETE %@",configuration[@"id"]);
        [app terminate];
    }
}
@end
