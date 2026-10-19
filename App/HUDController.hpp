#pragma once
#import <UIKit/UIKit.h>
@interface HUDController:UIViewController
-(BOOL)capturePoint:(CGPoint)point;
-(void)globalTouch:(CGPoint)point phase:(NSUInteger)phase identity:(NSUInteger)identity;
@end
