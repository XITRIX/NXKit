#pragma once
// Native research fixtures only. Read applied layer/filter state as data.
#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <objc/message.h>
#import <dlfcn.h>
#import <math.h>
static NSString*S(const char*p){return p?([NSString stringWithUTF8String:p]?:@"<non-UTF8>"):@"";}
static id Value(id o,int depth);
static NSDictionary *Metadata(Class c) {
 if(!c)return @{@"name":@"<nil>"};
 NSMutableArray *methods=[NSMutableArray new],*properties=[NSMutableArray new],*ivars=[NSMutableArray new];unsigned n=0;
 Method *m=class_copyMethodList(c,&n);for(unsigned i=0;i<n;i++)[methods addObject:@{@"name":NSStringFromSelector(method_getName(m[i])),@"type":S(method_getTypeEncoding(m[i]))}];free(m);
 objc_property_t *p=class_copyPropertyList(c,&n);for(unsigned i=0;i<n;i++)[properties addObject:@{@"name":S(property_getName(p[i])),@"type":S(property_getAttributes(p[i]))}];free(p);
 Ivar *v=class_copyIvarList(c,&n);for(unsigned i=0;i<n;i++)[ivars addObject:@{@"name":S(ivar_getName(v[i])),@"type":S(ivar_getTypeEncoding(v[i])),@"offset":@(ivar_getOffset(v[i]))}];free(v);
 return @{@"name":NSStringFromClass(c),@"super":NSStringFromClass(class_getSuperclass(c))?:@"",@"image":@(class_getImageName(c)?:""),@"methods":methods,@"properties":properties,@"ivars":ivars};
}
static id Value(id o,int depth) {
 if(!o)return NSNull.null;if([o isKindOfClass:NSString.class])return o;if([o isKindOfClass:NSNumber.class])return isfinite([o doubleValue])?o:[o description];
 if([o isKindOfClass:UIColor.class]) {CGFloat r=0,g=0,b=0,a=0;if([o getRed:&r green:&g blue:&b alpha:&a])return @{@"class":NSStringFromClass([o class]),@"rgba":@[@(r),@(g),@(b),@(a)]};}
 if([o isKindOfClass:NSValue.class])return @{@"class":NSStringFromClass([o class]),@"encoding":@([o objCType]),@"description":[o description]};
 if([NSStringFromClass([o class])isEqual:@"__NSCFType"]&&CFGetTypeID((__bridge CFTypeRef)o)==CGColorGetTypeID()){CGColorRef color=(__bridge CGColorRef)o;NSMutableArray*c=[NSMutableArray new];for(size_t i=0;i<CGColorGetNumberOfComponents(color);i++)[c addObject:@(CGColorGetComponents(color)[i])];return @{@"class":@"CGColor",@"components":c};}
 if(depth<=0)return @{@"class":NSStringFromClass([o class])};
 if([o isKindOfClass:NSArray.class]){NSMutableArray*a=[NSMutableArray new];for(id x in o)if(a.count<64)[a addObject:Value(x,depth-1)];return a;}
 if([o isKindOfClass:NSDictionary.class]){NSMutableDictionary*d=[NSMutableDictionary new];for(id k in o)if([k isKindOfClass:NSString.class])d[k]=Value(o[k],depth-1);return d;}
 NSMutableDictionary *d=[@{@"class":NSStringFromClass([o class])} mutableCopy];
 if([NSStringFromClass([o class])isEqual:@"CAFilter"]){@try{NSData*archive=[NSKeyedArchiver archivedDataWithRootObject:o requiringSecureCoding:NO error:nil];if(archive)d[@"archiveBase64"]=[archive base64EncodedStringWithOptions:0];d[@"name"]=[o valueForKey:@"name"]?:NSNull.null;NSArray*keys=[o valueForKey:@"inputKeys"];for(NSString*k in keys)d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){d[@"error"]=e.reason?:@"";}}

 if([NSStringFromClass([o class])containsString:@"SwiftValue"]||[NSStringFromClass([o class])containsString:@"EquatableBox"])d[@"description"]=[o description];
 if([o isKindOfClass:CALayer.class])for(NSString*k in @[@"filters",@"backgroundFilters",@"compositingFilter"]){@try{d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){}}

 for(Class c=[o class];c&&c!=NSObject.class&&c!=UIView.class&&c!=CALayer.class;c=class_getSuperclass(c)){
  unsigned n=0;objc_property_t*p=class_copyPropertyList(c,&n);for(unsigned i=0;i<n;i++){@try{NSString*k=S(property_getName(p[i]));if(![k hasPrefix:@"debug"]&&![k isEqual:@"description"]&&![k isEqual:@"parent"]&&![k isEqual:@"delegate"]&&![k isEqual:@"hash"]&&![k isEqual:@"superclass"]&&![k isEqual:@"class"])d[k]=Value([o valueForKey:k],depth-1);}@catch(NSException*e){}}free(p);
 }
 return d;
}
static void TestMutation(CALayer*l,NSMutableArray*results){for(id f in l.filters)if([[f valueForKey:@"type"]isEqual:@"glassBackground"]){id old=[f valueForKey:@"inputBlurRadius"];if([old isKindOfClass:NSNumber.class]){@try{[f setValue:@([old doubleValue]+1)forKey:@"inputBlurRadius"];id changed=[f valueForKey:@"inputBlurRadius"];[f setValue:old forKey:@"inputBlurRadius"];[results addObject:@{@"key":@"inputBlurRadius",@"before":old,@"after":changed?:NSNull.null,@"restored":[f valueForKey:@"inputBlurRadius"]?:NSNull.null}];}@catch(NSException*e){[results addObject:@{@"error":e.reason?:@""}];}}}for(CALayer*c in l.sublayers)TestMutation(c,results);}
// Explicit read-only identity map: object identity resolves a portal's live
// source to a stable path in the same window layer tree, without archiving or
// retaining private layers, calling setters, or dereferencing private ivars.
static void MapLayerPaths(CALayer *layer,NSString *path,NSMapTable *paths,int depth) {
 if(!layer||depth<0||[paths objectForKey:layer])return;
 [paths setObject:path forKey:layer];NSUInteger index=0;
 for(CALayer *child in layer.sublayers)MapLayerPaths(child,[path stringByAppendingFormat:@".%lu",(unsigned long)index++],paths,depth-1);
}
static id ReadOnlyCGPath(CGPathRef path) {
 if(!path)return NSNull.null;
 NSMutableArray *elements=[NSMutableArray new];
 CGPathApplyWithBlock(path,^(const CGPathElement *element){
  NSUInteger count=0;switch(element->type){case kCGPathElementMoveToPoint:case kCGPathElementAddLineToPoint:count=1;break;case kCGPathElementAddQuadCurveToPoint:count=2;break;case kCGPathElementAddCurveToPoint:count=3;break;case kCGPathElementCloseSubpath:break;}
  NSMutableArray *points=[NSMutableArray new];for(NSUInteger i=0;i<count;i++)[points addObject:@[@(element->points[i].x),@(element->points[i].y)]];
  [elements addObject:@{@"type":@(element->type),@"points":points}];
 });
 return @{@"bounds":NSStringFromCGRect(CGPathGetBoundingBox(path)),@"empty":@(CGPathIsEmpty(path)),@"elements":elements};
}
static NSDictionary*LayerTreeWithPaths(CALayer*l,int depth,NSMapTable *paths){NSMutableDictionary*d=[@{@"value":Value(l,5),@"path":[paths objectForKey:l]?:@"<unmapped>",@"bounds":NSStringFromCGRect(l.bounds),@"cornerRadius":@(l.cornerRadius),@"opacity":@(l.opacity),@"name":l.name?:@"",@"hidden":@(l.hidden),@"contentsScale":@(l.contentsScale),@"borderWidth":@(l.borderWidth),@"borderColor":Value((__bridge id)l.borderColor,1),@"shadowColor":Value((__bridge id)l.shadowColor,1),@"shadowOpacity":@(l.shadowOpacity),@"shadowRadius":@(l.shadowRadius),@"shadowOffset":@[@(l.shadowOffset.width),@(l.shadowOffset.height)],@"backgroundColor":Value((__bridge id)l.backgroundColor,1)}mutableCopy];
 d[@"masksToBounds"]=@(l.masksToBounds);d[@"hasContents"]=@(l.contents!=nil);
 if([l.name hasSuffix:@"innerShadowView"]){d[@"shadowPath"]=ReadOnlyCGPath(l.shadowPath);d[@"maskLayerPath"]=l.mask?([paths objectForKey:l.mask]?:@"<mask-not-sublayer>"):(id)NSNull.null;d[@"maskLayer"]=Value(l.mask,3);}
 if([NSStringFromClass(l.class)containsString:@"PortalLayer"]&&[l respondsToSelector:NSSelectorFromString(@"sourceLayer")]) {
  @try { CALayer *source=[l valueForKey:@"sourceLayer"];
   d[@"portalSourceLayerPath"]=source?([paths objectForKey:source]?:@"<outside-window-tree>"):(id)NSNull.null;
   d[@"portalSourceLayerName"]=source?(source.name?:@""):(id)NSNull.null;
  } @catch(NSException *exception) {d[@"portalSourceLayerError"]=exception.name;}
 }
 NSMutableArray*a=[NSMutableArray new];if(depth>0)for(CALayer*c in l.sublayers)[a addObject:LayerTreeWithPaths(c,depth-1,paths)];d[@"sublayers"]=a;return d;}
static NSDictionary*LayerTree(CALayer*l,int depth){
 CALayer *root=l;while(root.superlayer)root=root.superlayer;
 NSMapTable *paths=[NSMapTable weakToStrongObjectsMapTable];MapLayerPaths(root,@"window",paths,64);
 return LayerTreeWithPaths(l,depth,paths);
}
static NSDictionary *Tree(UIView*v,int depth){NSMutableDictionary*d=[@{@"class":NSStringFromClass(v.class),@"frame":NSStringFromCGRect(v.frame),@"layer":Value(v.layer,2)}mutableCopy];NSMutableDictionary*refs=[NSMutableDictionary new];
 for(Class c=v.class;c&&c!=NSObject.class;c=class_getSuperclass(c)){unsigned n=0;Ivar*iv=class_copyIvarList(c,&n);for(unsigned i=0;i<n;i++){NSString*k=S(ivar_getName(iv[i]));if(ivar_getTypeEncoding(iv[i])&&ivar_getTypeEncoding(iv[i])[0]=='@'&&([k.lowercaseString containsString:@"setting"]||[k.lowercaseString containsString:@"effect"]||[k.lowercaseString containsString:@"spec"]||[k.lowercaseString containsString:@"recipe"]||[k.lowercaseString containsString:@"glass"]))@try{refs[k]=Value(object_getIvar(v,iv[i]),4);}@catch(NSException*e){}}free(iv);}for(NSString*k in @[@"_glass",@"glass",@"_glassBackgroundStyle"]){SEL sel=NSSelectorFromString(k);if([v respondsToSelector:sel])@try{refs[k]=Value([v valueForKey:k],5);}@catch(NSException*e){}}d[@"references"]=refs;
 NSMutableArray*children=[NSMutableArray new];if(depth>0)for(UIView*child in v.subviews)[children addObject:Tree(child,depth-1)];d[@"children"]=children;return d;}
