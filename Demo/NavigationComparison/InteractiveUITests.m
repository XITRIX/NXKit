#import <XCTest/XCTest.h>
@interface InteractiveUITests:XCTestCase@end
@implementation InteractiveUITests
- (void)testLeadingCancellationRepeat {
 for(NSInteger attempt=0;attempt<2;++attempt){
  XCUIApplication *app=[[XCUIApplication alloc]initWithBundleIdentifier:@"org.nxkit.NavigationInteractiveReference"];
  app.launchEnvironment=@{@"SOURCE":@"6",@"TARGET":@"5",@"DARK":@"1",@"CASE":[NSString stringWithFormat:@"leading-repeat-%ld-dark-cancel",(long)attempt]};[app launch];[NSThread sleepForTimeInterval:.6];
  XCUICoordinate *origin=[app coordinateWithNormalizedOffset:CGVectorMake(0,0)],*start=[origin coordinateWithOffset:CGVectorMake(2,300)];
  [start pressForDuration:.05 thenDragToCoordinate:[origin coordinateWithOffset:CGVectorMake(app.frame.size.width*.28,300)] withVelocity:350 thenHoldForDuration:.25];[NSThread sleepForTimeInterval:2.3];[app terminate];
 }
}
- (void)testCancellationLifecycle {
 for(NSNumber *finish in @[@NO,@YES]) {
  XCUIApplication *app=[[XCUIApplication alloc]initWithBundleIdentifier:@"org.nxkit.NavigationInteractiveReference"];
  app.launchEnvironment=@{@"SOURCE":@"5",@"TARGET":@"4",@"DARK":@"1",@"CASE":finish.boolValue?@"lifecycle-dark-finish":@"lifecycle-dark-cancel"};[app launch];[NSThread sleepForTimeInterval:.6];
  XCUICoordinate *origin=[app coordinateWithNormalizedOffset:CGVectorMake(0,0)],*start=[origin coordinateWithOffset:CGVectorMake(2,300)];
  [start pressForDuration:.05 thenDragToCoordinate:[origin coordinateWithOffset:CGVectorMake(finish.boolValue?320:114,300)] withVelocity:350 thenHoldForDuration:.25];[NSThread sleepForTimeInterval:2.3];[app terminate];
 }
}

- (void)testEdgePop {
    self.continueAfterFailure=NO;
    for(NSInteger target=0;target<8;++target)for(NSNumber *dark in @[@NO,@YES])for(NSNumber *finish in @[@NO,@YES]) {
        NSInteger source=(target+1)%8;
        NSString *name=[NSString stringWithFormat:@"%ld-%ld-%@-%@",(long)source,(long)target,dark.boolValue?@"dark":@"light",finish.boolValue?@"finish":@"cancel"];
        XCUIApplication *app=[[XCUIApplication alloc]initWithBundleIdentifier:@"org.nxkit.NavigationInteractiveReference"];
        app.launchEnvironment=@{@"SOURCE":@(source).stringValue,@"TARGET":@(target).stringValue,@"DARK":dark.stringValue,@"CASE":name};[app launch];
        [NSThread sleepForTimeInterval:.6];
        XCUICoordinate *origin=[app coordinateWithNormalizedOffset:CGVectorMake(0,0)];
        XCUICoordinate *start=[origin coordinateWithOffset:CGVectorMake(2,300)];
        [start pressForDuration:.05 thenDragToCoordinate:[origin coordinateWithOffset:CGVectorMake(app.frame.size.width*(finish.boolValue?.8:.28),300)] withVelocity:350 thenHoldForDuration:.25];
        [NSThread sleepForTimeInterval:2.1];[app terminate];
    }
}
@end
