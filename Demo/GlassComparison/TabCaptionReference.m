// Public UIKit measurement probe. Run with simctl spawn; no app launch or
// private selectors are needed to inspect the real UITabBar caption styles.
#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>

static void Collect(UIView *view, UITabBar *bar, NSMutableArray *rows) {
    if ([view isKindOfClass:UILabel.class]) {
        UILabel *label=(UILabel *)view;
        CGRect frame=[label convertRect:label.bounds toView:bar];
        NSMutableArray *glyphs=[NSMutableArray new];
        NSAttributedString *text=[[NSAttributedString alloc] initWithString:label.text?:@"" attributes:@{NSFontAttributeName:label.font}];
        CTLineRef line=CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)text);
        for (id raw in (__bridge NSArray *)CTLineGetGlyphRuns(line)) {
            CTRunRef run=(__bridge CTRunRef)raw;
            CFIndex count=CTRunGetGlyphCount(run);
            CGPoint positions[count]; CGGlyph identifiers[count];
            CTRunGetPositions(run,CFRangeMake(0,0),positions);
            CTRunGetGlyphs(run,CFRangeMake(0,0),identifiers);
            for (CFIndex i=0;i<count;i++) [glyphs addObject:@[@(identifiers[i]),@(positions[i].x)]];
        }
        [rows addObject:@{@"text":label.text?:@"",@"font":label.font.fontName,
            @"points":@(label.font.pointSize),@"ascender":@(label.font.ascender),
            @"lineHeight":@(label.font.lineHeight),@"alignment":@(label.textAlignment),
            @"frame":@[@(frame.origin.x),@(frame.origin.y),@(frame.size.width),@(frame.size.height)],
            @"advance":@(CTLineGetTypographicBounds(line,NULL,NULL,NULL)),@"glyphs":glyphs}];
        CFRelease(line);
    }
    for (UIView *child in view.subviews) Collect(child,bar,rows);
}
int main() { @autoreleasepool {
    UITabBar *bar=[[UITabBar alloc] initWithFrame:CGRectMake(0,0,402,83)];
    bar.traitOverrides.horizontalSizeClass=UIUserInterfaceSizeClassCompact;
    NSArray *titles=@[@"Home",@"Favorites",@"Library",@"Settings"];
    NSArray *symbols=@[@"house.fill",@"star.fill",@"books.vertical.fill",@"slider.horizontal.3"];
    NSMutableArray *items=[NSMutableArray new],*rows=[NSMutableArray new];
    for (NSUInteger i=0;i<titles.count;i++) [items addObject:[[UITabBarItem alloc] initWithTitle:titles[i] image:[UIImage systemImageNamed:symbols[i]] tag:i]];
    bar.items=items;bar.selectedItem=items[2];[bar layoutIfNeeded];Collect(bar,bar,rows);
    NSData *data=[NSJSONSerialization dataWithJSONObject:@{@"os":UIDevice.currentDevice.systemVersion,@"scale":@(bar.traitCollection.displayScale),@"labels":rows} options:NSJSONWritingPrettyPrinted error:nil];
    if(!data||rows.count!=8)return 1;
    puts([[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding].UTF8String);
}return 0;}
