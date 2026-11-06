#import <XCTest/XCTest.h>

@interface InteractionUITests : XCTestCase
@end
@implementation InteractionUITests
- (void)testNativeTouchTraces {
    self.continueAfterFailure=NO;
    NSURL *url=[[NSBundle bundleForClass:self.class] URLForResource:@"Cases" withExtension:@"json"];
    NSArray *cases=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:url] options:0 error:nil];
    XCTAssertGreaterThan(cases.count,0);
    for (NSDictionary *configuration in cases) {
        XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.GlassInteractionReference"];
        app.launchEnvironment=@{@"GLASS_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};
        [app launch];
        XCUIElement *target=app.buttons[@"target"]; XCTAssertTrue([target waitForExistenceWithTimeout:10]);
        NSPredicate *ready=[NSPredicate predicateWithFormat:@"label == 'ready'"];
        XCTNSPredicateExpectation *prepared=[[XCTNSPredicateExpectation alloc] initWithPredicate:ready object:app.staticTexts[@"status"]];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[prepared] timeout:8],XCTWaiterResultCompleted);
        // UIKit animations are measured while they run; no synthetic UITouch or
        // private event injection is used by either the app or this runner.
        XCUICoordinate *start=[target coordinateWithNormalizedOffset:CGVectorMake(.5,.5)];
        XCUICoordinate *end=[start coordinateWithOffset:CGVectorMake([configuration[@"dx"] doubleValue],[configuration[@"dy"] doubleValue])];
        [start pressForDuration:configuration[@"holdBefore"] ? [configuration[@"holdBefore"] doubleValue] : .7
            thenDragToCoordinate:end withVelocity:configuration[@"velocity"] ? [configuration[@"velocity"] doubleValue] : 200
            thenHoldForDuration:configuration[@"holdAfter"] ? [configuration[@"holdAfter"] doubleValue] : .65];
        XCUIElement *status=app.staticTexts[@"status"];
        XCTNSPredicateExpectation *finished=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'complete'"] object:status];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[finished] timeout:8],XCTWaiterResultCompleted);
        NSLog(@"GLASS_CASE_COMPLETE %@",configuration[@"id"]);
        [app terminate];
    }
}
@end
