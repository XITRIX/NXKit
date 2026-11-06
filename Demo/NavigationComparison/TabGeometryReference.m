// Standalone public UITabBar measurements across explicit widths.
#import <UIKit/UIKit.h>
static NSArray *RectValues(CGRect r){return @[@(r.origin.x),@(r.origin.y),@(r.size.width),@(r.size.height)];}
static void Collect(UIView *view,UITabBar *bar,NSMutableArray *rows){
 if(view.hidden||view.alpha<.01)return;
 [rows addObject:@{@"class":NSStringFromClass(view.class),@"frame":RectValues([view convertRect:view.bounds toView:bar])}];
 for(UIView *child in view.subviews)Collect(child,bar,rows);
}
int main(){@autoreleasepool{
 NSMutableArray *records=[NSMutableArray new];
 NSArray *titles=@[@"Home",@"Favorites",@"Library",@"Settings",@"Search"],*symbols=@[@"house.fill",@"star.fill",@"books.vertical.fill",@"slider.horizontal.3",@"magnifyingglass"];
 for(NSNumber *width in @[@320,@360,@390,@402,@430,@600])for(NSNumber *count in @[@2,@3,@4,@5]){
  UITabBar *bar=[[UITabBar alloc] initWithFrame:CGRectMake(0,0,width.doubleValue,83)];
  bar.traitOverrides.horizontalSizeClass=UIUserInterfaceSizeClassCompact;
  NSMutableArray *items=[NSMutableArray new],*rows=[NSMutableArray new];
  for(NSUInteger i=0;i<count.unsignedIntegerValue;i++)[items addObject:[[UITabBarItem alloc] initWithTitle:titles[i] image:[UIImage systemImageNamed:symbols[i]] tag:i]];
  bar.items=items;bar.selectedItem=items.firstObject;[bar layoutIfNeeded];Collect(bar,bar,rows);
  [records addObject:@{@"width":width,@"count":count,@"views":rows}];
 }
 NSMutableArray *widths=[NSMutableArray new];for(NSString *title in titles)[widths addObject:@{@"title":title,@"bodyRegular":@([title sizeWithAttributes:@{NSFontAttributeName:[UIFont systemFontOfSize:17]}].width),@"bodyMedium":@([title sizeWithAttributes:@{NSFontAttributeName:[UIFont systemFontOfSize:17 weight:UIFontWeightMedium]}].width)}];
 NSData *data=[NSJSONSerialization dataWithJSONObject:@{@"os":UIDevice.currentDevice.systemVersion,@"records":records,@"titleWidths":widths} options:NSJSONWritingPrettyPrinted error:nil];
 puts([[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding].UTF8String);
}return 0;}
