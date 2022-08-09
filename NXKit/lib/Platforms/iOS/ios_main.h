//
//  ios_video_layer.h
//  NXKit
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#include <Foundation/Foundation.h>
#import <OpenGLES/ES2/glext.h>
#import <GLKit/GLKit.h>

@interface IOSVideoLayer: NSObject

@property (strong, nonatomic) EAGLContext *context;

- (instancetype)initWithPath:(NSString*)path;

- (void)setScale:(float)width height:(float)height scale:(float)scale;

- (void)applicationLoop;

- (void)setKey:(int)key state:(bool)state;

@end
