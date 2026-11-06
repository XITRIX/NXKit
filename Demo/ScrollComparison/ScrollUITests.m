#import <XCTest/XCTest.h>
#import <objc/message.h>
#import <objc/runtime.h>
@interface ScrollUITests:XCTestCase@end
@implementation ScrollUITests
- (void)sendPaths:(NSArray *)paths origin:(CGPoint)origin {
 Class pathClass=NSClassFromString(@"XCPointerEventPath"),recordClass=NSClassFromString(@"XCSynthesizedEventRecord");
 SEL initPath=NSSelectorFromString(@"initForTouchAtPoint:offset:"),move=NSSelectorFromString(@"moveToPoint:atOffset:"),lift=NSSelectorFromString(@"liftUpAtOffset:"),add=NSSelectorFromString(@"addPointerEventPath:"),synthesize=NSSelectorFromString(@"synthesizeEvent:completion:");
 XCTAssertNotEqual(class_getInstanceMethod(pathClass,initPath),NULL);XCTAssertNotEqual(class_getInstanceMethod(pathClass,move),NULL);XCTAssertNotEqual(class_getInstanceMethod(pathClass,lift),NULL);
 id record=((id(*)(id,SEL,id,NSInteger))objc_msgSend)([recordClass alloc],NSSelectorFromString(@"initWithName:interfaceOrientation:"),@"NXKit native scroll comparison",UIInterfaceOrientationPortrait);
 // XCTest 27's retirement mapping is stable when lower indices outlive
 // higher ones. A chronological insertion corrupted FIFO handoff coordinates.
 NSArray *ordered=[paths sortedArrayUsingComparator:^NSComparisonResult(NSDictionary *a,NSDictionary *b){return [b[@"lift"] compare:a[@"lift"]];}];
 for(NSDictionary *configuration in ordered){NSArray *points=configuration[@"points"],*first=points.firstObject;
  id path=((id(*)(id,SEL,CGPoint,double))objc_msgSend)([pathClass alloc],initPath,CGPointMake(origin.x+[first[1] doubleValue],origin.y+[first[2] doubleValue]),[first[0] doubleValue]);
  NSLog(@"SCROLL_PATH_BEFORE %@",[path valueForKey:@"index"]);
  for(NSUInteger i=1;i<points.count;i++){NSArray *p=points[i];((void(*)(id,SEL,CGPoint,double))objc_msgSend)(path,move,CGPointMake(origin.x+[p[1] doubleValue],origin.y+[p[2] doubleValue]),[p[0] doubleValue]);}
  ((void(*)(id,SEL,double))objc_msgSend)(path,lift,[configuration[@"lift"] doubleValue]);((void(*)(id,SEL,id))objc_msgSend)(record,add,path);NSLog(@"SCROLL_PATH_AFTER %@",[path valueForKey:@"index"]);
 }
 SEL getter=NSSelectorFromString(@"eventSynthesizer");XCTAssertTrue([XCUIDevice.sharedDevice respondsToSelector:getter]);id synthesizer=((id(*)(id,SEL))objc_msgSend)(XCUIDevice.sharedDevice,getter);XCTAssertNotEqual(class_getInstanceMethod([synthesizer class],synthesize),NULL);
 XCTestExpectation *done=[self expectationWithDescription:@"native OS touch sequence"];((void(*)(id,SEL,id,id))objc_msgSend)(synthesizer,synthesize,record,^(BOOL success,NSError *error){XCTAssertTrue(success);XCTAssertNil(error);[done fulfill];});[self waitForExpectations:@[done] timeout:20];
}
- (void)testScrollCases {
 self.continueAfterFailure=NO;XCUIDevice.sharedDevice.orientation=UIDeviceOrientationPortrait;
 NSArray *cases=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:[[NSBundle bundleForClass:self.class] URLForResource:@"Cases" withExtension:@"json"]] options:0 error:nil];
 for(NSDictionary *c in cases){XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.ScrollReference"];app.launchEnvironment=@{@"SCROLL_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:c options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];
  XCUIElement *target=[app descendantsMatchingType:XCUIElementTypeAny][@"target"];
  XCTAssertTrue([target waitForExistenceWithTimeout:10]);XCTNSPredicateExpectation *ready=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'ready'"] object:app.staticTexts[@"status"]];XCTAssertEqual([XCTWaiter waitForExpectations:@[ready] timeout:10],XCTWaiterResultCompleted);
  [self sendPaths:c[@"paths"] origin:[target coordinateWithNormalizedOffset:CGVectorMake(0,0)].screenPoint];
  if([c[@"rotateAfter"] boolValue])XCUIDevice.sharedDevice.orientation=UIDeviceOrientationLandscapeLeft;
  XCTNSPredicateExpectation *finished=[[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithFormat:@"label == 'complete'"] object:app.staticTexts[@"status"]];XCTAssertEqual([XCTWaiter waitForExpectations:@[finished] timeout:12],XCTWaiterResultCompleted);NSLog(@"SCROLL_COMPLETE %@",c[@"id"]);[app terminate];XCUIDevice.sharedDevice.orientation=UIDeviceOrientationPortrait;
 }
}
@end
