#import <XCTest/XCTest.h>
@interface GroupingUITests:XCTestCase@end
@implementation GroupingUITests
- (void)testNativeGroupContact {
 self.continueAfterFailure=NO;
 for(NSString *name in @[@"pair",@"prominent",@"fixed-0",@"unshared"]){
  XCUIApplication *app=[[XCUIApplication alloc]initWithBundleIdentifier:@"org.nxkit.NavigationGroupingReference"];app.launchEnvironment=@{@"HOLD_CASE":name};[app launch];
  XCUIElement *status=app.staticTexts[@"status"];XCTAssertTrue([status waitForExistenceWithTimeout:10]);
  XCUICoordinate *start=[[app coordinateWithNormalizedOffset:CGVectorMake(0,0)]coordinateWithOffset:CGVectorMake(364,84)];
  [start pressForDuration:.7 thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake(-6,4)] withVelocity:60 thenHoldForDuration:.4];
  XCTNSPredicateExpectation *done=[[XCTNSPredicateExpectation alloc]initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'complete'"]object:status];
  XCTAssertEqual([XCTWaiter waitForExpectations:@[done]timeout:5],XCTWaiterResultCompleted);
  [NSThread sleepForTimeInterval:1];[app terminate];
 }
}
@end
