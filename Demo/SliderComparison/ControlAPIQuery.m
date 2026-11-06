// Native command-line public UIKit API oracle; no window, touch or pixel claims.
#import <UIKit/UIKit.h>
int main(void) { @autoreleasepool {
    NSMutableDictionary *result=[NSMutableDictionary new];UIControl *c=[[UIControl alloc] initWithFrame:CGRectMake(0,0,100,44)];c.selected=YES;c.highlighted=YES;c.enabled=NO;
    result[@"disabledState"]=@{@"selected":@(c.selected),@"highlighted":@(c.highlighted),@"enabled":@(c.enabled),@"state":@(c.state),@"hitTestSelf":@([c hitTest:CGPointMake(50,22) withEvent:nil]==c)};
    __block NSInteger calls=0;[c addAction:[UIAction actionWithHandler:^(__kindof UIAction *a){calls++;}] forControlEvents:UIControlEventPrimaryActionTriggered];[c performPrimaryAction];result[@"disabledPrimaryActionCalls"]=@(calls);
    UISlider *s=[[UISlider alloc] initWithFrame:CGRectMake(0,0,300,50)];s.enabled=NO;[s layoutIfNeeded];result[@"disabledSliderHitTestClass"]=NSStringFromClass([s hitTest:CGPointMake(150,25) withEvent:nil].class)?:@"nil";
    UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(30,40)];UIImage *image=[renderer imageWithActions:^(UIGraphicsImageRendererContext *context){[UIColor.redColor setFill];UIRectFill(CGRectMake(0,0,30,40));}];[s setThumbImage:image forState:UIControlStateNormal];
    result[@"thumbImageNormalIdentity"]=@([s thumbImageForState:UIControlStateNormal]==image);result[@"thumbImageHighlightedIdentity"]=@([s thumbImageForState:UIControlStateHighlighted]==image);result[@"thumbImageDisabledIdentity"]=@([s thumbImageForState:UIControlStateDisabled]==image);result[@"currentThumbImageIdentity"]=@(s.currentThumbImage==image);
    NSData *data=[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil];fwrite(data.bytes,1,data.length,stdout);fputc('\n',stdout);
}return 0;}
