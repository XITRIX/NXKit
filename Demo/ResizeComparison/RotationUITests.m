#import <XCTest/XCTest.h>
@interface RotationUITests : XCTestCase @end
@implementation RotationUITests
- (void)testNativeRotationGeometry {
    XCUIDevice.sharedDevice.orientation=UIDeviceOrientationPortrait;
    XCUIApplication* app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.RotationProbe"];
    [app launch];
    XCUIElement* status=app.staticTexts[@"rotation-status"];
    XCTAssertTrue([status waitForExistenceWithTimeout:10]);
    int completed=0;
    for(NSNumber* orientation in @[@(UIDeviceOrientationLandscapeLeft),@(UIDeviceOrientationPortrait),
                                   @(UIDeviceOrientationLandscapeRight),@(UIDeviceOrientationPortrait)]) {
        XCUIDevice.sharedDevice.orientation=orientation.integerValue;
        const int expected=++completed;
        NSPredicate* predicate=[NSPredicate predicateWithBlock:^BOOL(id object,NSDictionary* bindings) {
            NSDictionary* state=[NSJSONSerialization JSONObjectWithData:[status.label dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
            return [state[@"completed"] intValue]>=expected;
        }];
        XCTNSPredicateExpectation* transition=[[XCTNSPredicateExpectation alloc] initWithPredicate:predicate object:status];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[transition] timeout:8],XCTWaiterResultCompleted);
    }
    NSDictionary* summary=[NSJSONSerialization JSONObjectWithData:[status.label dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
    XCTAssertGreaterThanOrEqual([summary[@"completed"] intValue],4);
    XCTAssertGreaterThan([summary[@"intermediate"] intValue],8);
    XCTAssertLessThan([summary[@"maxReferenceError"] doubleValue],0.25);
    XCTAssertEqual([summary[@"scaleErrors"] intValue],0);
    XCTAssertEqual([summary[@"drawableErrors"] intValue],0);
    XCTAssertGreaterThan([summary[@"drawables"] intValue],8);
    NSLog(@"NXKIT_ROTATION_RESULT %@",summary);
    [app terminate];
}
@end
