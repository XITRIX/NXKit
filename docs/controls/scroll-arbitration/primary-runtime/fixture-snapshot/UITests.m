#import <XCTest/XCTest.h>
@interface ControlScrollTests:XCTestCase @end
@implementation ControlScrollTests
- (void)testNativeArbitration{
self.continueAfterFailure=NO;
NSArray *cases=@[
@{@"id":@"tap",@"hold":@.05,@"dx":@0,@"dy":@0},
@{@"id":@"horizontal",@"hold":@.01,@"dx":@70,@"dy":@0},
@{@"id":@"vertical",@"hold":@.01,@"dx":@0,@"dy":@-100},
@{@"id":@"hold-short",@"hold":@.12,@"dx":@0,@"dy":@0},
@{@"id":@"hold-long",@"hold":@.65,@"dx":@0,@"dy":@0},
@{@"id":@"held-vertical",@"hold":@.65,@"dx":@0,@"dy":@-100},
@{@"id":@"outside-hold",@"outside":@YES,@"hold":@.65,@"dx":@0,@"dy":@0},
@{@"id":@"no-delay-vertical",@"delays":@NO,@"hold":@.01,@"dx":@0,@"dy":@-100},
@{@"id":@"no-cancel-vertical",@"canCancel":@NO,@"hold":@.65,@"dx":@0,@"dy":@-100},
@{@"id":@"vertical-fast",@"hold":@.01,@"dx":@0,@"dy":@-100,@"velocity":@1000},
@{@"id":@"horizontal-fast",@"hold":@.01,@"dx":@70,@"dy":@0,@"velocity":@1000},
@{@"id":@"track-vertical-fast",@"hold":@.01,@"dx":@0,@"dy":@-100,@"velocity":@1000,@"track":@YES},
@{@"id":@"track-tap",@"hold":@.05,@"dx":@0,@"dy":@0,@"track":@YES},
@{@"id":@"horizontal-scroll",@"horizontalScroll":@YES,@"hold":@.01,@"dx":@-70,@"dy":@0}
];
NSString *filter=NSProcessInfo.processInfo.environment[@"CONTROL_SCROLL_FILTER"];
NSArray *selection=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfFile:[[NSBundle bundleForClass:self.class] pathForResource:@"Cases" ofType:@"json"]] options:0 error:nil];
NSSet *selected=[NSSet setWithArray:[selection valueForKey:@"id"]];
for(NSString *control in @[@"slider",@"switch"])for(NSDictionary *input in cases){NSMutableDictionary *c=[input mutableCopy];c[@"control"]=control;c[@"id"]=[NSString stringWithFormat:@"%@-%@",control,input[@"id"]];if(filter.length&&![c[@"id"] containsString:filter])continue;if(selected.count&&![selected containsObject:@"control-scroll"]&&![selected containsObject:c[@"id"]])continue;
XCUIApplication *app=[[XCUIApplication alloc] initWithBundleIdentifier:@"org.nxkit.ControlScrollReference"];app.launchEnvironment=@{@"CONTROL_SCROLL_CASE":[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:c options:0 error:nil] encoding:NSUTF8StringEncoding]};[app launch];XCUIElement *status=app.staticTexts[@"status"];XCTAssertTrue([status waitForExistenceWithTimeout:10]);
XCUICoordinate *start=[app coordinateWithNormalizedOffset:CGVectorMake(0,0)];start=[start coordinateWithOffset:[control isEqual:@"slider"]?CGVectorMake([c[@"track"] boolValue]?290:200,275):CGVectorMake([c[@"track"] boolValue]?128:98,264)];CGFloat dx=[c[@"dx"] doubleValue],dy=[c[@"dy"] doubleValue];if(dx==0&&dy==0)[start pressForDuration:[c[@"hold"] doubleValue]];else [start pressForDuration:[c[@"hold"] doubleValue] thenDragToCoordinate:[start coordinateWithOffset:CGVectorMake(dx,dy)] withVelocity:c[@"velocity"]?[c[@"velocity"] doubleValue]:100 thenHoldForDuration:.05];[NSThread sleepForTimeInterval:.7];[app terminate];
}
}
@end
