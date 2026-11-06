#import <XCTest/XCTest.h>
@interface SliderUITests : XCTestCase @end
@implementation SliderUITests
- (void)testNativeSnapshots {
    for(NSNumber *dark in @[@NO,@YES]) {
    XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.SliderReference"];
    NSDictionary *configuration=@{@"id":dark.boolValue?@"thumb-snapshots-dark":@"thumb-snapshots",@"dark":dark,@"x":@.28,@"snapshots":@YES};
    app.launchEnvironment=@{@"SLIDER_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];XCUIElement *slider=app.sliders[@"slider"];XCTAssertTrue([slider waitForExistenceWithTimeout:10]);
    XCUICoordinate *start=[slider coordinateWithNormalizedOffset:CGVectorMake(.28,.5)];[start pressForDuration:1 thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake(80,0)] withVelocity:100 thenHoldForDuration:1];
    [NSThread sleepForTimeInterval:1.5];[app terminate];
    }
}
- (void)testTrackingCalibration {
    NSArray *cases=@[@{@"id":@"zero",@"x":@.28,@"dx":@0,@"velocity":@50},@{@"id":@"slow",@"x":@.28,@"dx":@80,@"velocity":@50},@{@"id":@"fast",@"x":@.28,@"dx":@80,@"velocity":@700},@{@"id":@"offset-slow",@"x":@.31,@"dx":@80,@"velocity":@50}];
    for(NSDictionary *configuration in cases) {
        XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.SliderReference"];
        app.launchEnvironment=@{@"SLIDER_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];
        XCUIElement *slider=app.sliders[@"slider"];XCTAssertTrue([slider waitForExistenceWithTimeout:10]);XCUICoordinate *start=[slider coordinateWithNormalizedOffset:CGVectorMake([configuration[@"x"] doubleValue],.5)];
        [start pressForDuration:1 thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake([configuration[@"dx"] doubleValue],0)] withVelocity:[configuration[@"velocity"] doubleValue] thenHoldForDuration:1];
        [NSThread sleepForTimeInterval:1.5];[app terminate];
    }
}
- (void)testVelocityAndCustom {
    NSArray *cases=@[@{@"id":@"velocity-200",@"x":@.28,@"dx":@80,@"velocity":@200},@{@"id":@"velocity-400",@"x":@.28,@"dx":@80,@"velocity":@400},@{@"id":@"custom-thumb",@"x":@.275,@"dx":@80,@"velocity":@200,@"customThumb":@YES,@"snapshots":@YES}];
    for(NSDictionary *configuration in cases) {XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.SliderReference"];app.launchEnvironment=@{@"SLIDER_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];XCUIElement *slider=app.sliders[@"slider"];XCTAssertTrue([slider waitForExistenceWithTimeout:10]);XCUICoordinate *start=[slider coordinateWithNormalizedOffset:CGVectorMake([configuration[@"x"] doubleValue],.5)];[start pressForDuration:1 thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake(80,0)] withVelocity:[configuration[@"velocity"] doubleValue] thenHoldForDuration:1];[NSThread sleepForTimeInterval:1.5];[app terminate];}
}
- (void)testLowVelocity {
    NSArray *cases=@[@{@"id":@"velocity-50",@"x":@.28,@"dx":@80,@"velocity":@50},@{@"id":@"velocity-100",@"x":@.28,@"dx":@80,@"velocity":@100},@{@"id":@"velocity-150",@"x":@.28,@"dx":@80,@"velocity":@150}];
    for(NSDictionary *configuration in cases) {XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.SliderReference"];app.launchEnvironment=@{@"SLIDER_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];XCUIElement *slider=app.sliders[@"slider"];XCTAssertTrue([slider waitForExistenceWithTimeout:10]);XCUICoordinate *start=[slider coordinateWithNormalizedOffset:CGVectorMake(.28,.5)];[start pressForDuration:1 thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake(80,0)] withVelocity:[configuration[@"velocity"] doubleValue] thenHoldForDuration:1];[NSThread sleepForTimeInterval:1.5];[app terminate];}
}
- (void)testNativeTracking {
    self.continueAfterFailure=NO;
    NSArray *cases=@[
        @{@"id":@"thumb-center",@"x":@.28,@"dx":@80},
        @{@"id":@"thumb-offset",@"x":@.31,@"dx":@80},
        @{@"id":@"track-right",@"x":@.8,@"dx":@0},
        @{@"id":@"track-right-drag",@"x":@.8,@"dx":@30},
        @{@"id":@"noncontinuous",@"x":@.28,@"dx":@80,@"continuous":@NO}
    ];
    for (NSDictionary *configuration in cases) {
        XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.SliderReference"];
        app.launchEnvironment=@{@"SLIDER_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:configuration options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];
        XCUIElement *slider=app.sliders[@"slider"];XCTAssertTrue([slider waitForExistenceWithTimeout:10]);
        XCUICoordinate *start=[slider coordinateWithNormalizedOffset:CGVectorMake([configuration[@"x"] doubleValue],.5)];
        XCUICoordinate *end=[start coordinateWithOffset:CGVectorMake([configuration[@"dx"] doubleValue],0)];
        [start pressForDuration:.35 thenDragToCoordinate:end withVelocity:200 thenHoldForDuration:.2];
        XCTNSPredicateExpectation *finished=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'complete'"] object:app.staticTexts[@"status"]];
        XCTAssertEqual([XCTWaiter waitForExpectations:@[finished] timeout:10],XCTWaiterResultCompleted);[app terminate];
    }
}
@end
