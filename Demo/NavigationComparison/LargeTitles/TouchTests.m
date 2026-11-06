#import <XCTest/XCTest.h>
@interface LargeTitleTouchTests:XCTestCase@end
@implementation LargeTitleTouchTests
- (void)testCollapse {
 for(NSNumber *distance in @[@24,@40,@140]) {
  XCUIApplication*app=[[XCUIApplication alloc]initWithBundleIdentifier:@"org.nxkit.LargeTitles"];
  app.launchEnvironment=@{@"TOUCH_CASE":distance.stringValue};[app launch];[NSThread sleepForTimeInterval:1];
  XCUICoordinate*origin=[app coordinateWithNormalizedOffset:CGVectorMake(0,0)];
  [[origin coordinateWithOffset:CGVectorMake(200,500)] pressForDuration:.05 thenDragToCoordinate:[origin coordinateWithOffset:CGVectorMake(200,500-distance.doubleValue)] withVelocity:80 thenHoldForDuration:.5];
  [NSThread sleepForTimeInterval:9];[app terminate];
 }
}
@end
