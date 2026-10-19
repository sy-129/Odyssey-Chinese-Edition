#import <UIKit/UIKit.h>
#import <objc/message.h>
#import <dlfcn.h>
#import <signal.h>
#import <unistd.h>

#import "HUDController.hpp"
#include "SettingsStore.hpp"
#include <cstring>
static NSString* const pidPath=@"/var/mobile/Library/Caches/local.trollspeed.rebuild.pid";
static NSString* const logPath=@"/var/mobile/Library/Caches/local.trollspeed.rebuild.log";
static void record(NSString* message){[message writeToFile:logPath atomically:YES encoding:NSUTF8StringEncoding error:nil];NSLog(@"%@",message);}
@interface MainController:UIViewController @end
@interface HUDApplication:UIApplication @end
@implementation HUDApplication
@end
@interface HUDWindow:UIWindow @end
@implementation HUDWindow
-(BOOL)_isSystemWindow{return YES;}
-(BOOL)_isWindowServerHostingManaged{return NO;}
-(BOOL)_ignoresHitTest{return NO;}
-(BOOL)_isSecure{return ts::loadSettings().hideAtSnapshot;}
-(BOOL)_shouldCreateContextAsSecure{return ts::loadSettings().hideAtSnapshot;}
-(UIView*)hitTest:(CGPoint)point withEvent:(UIEvent*)event{
 HUDController* controller=(HUDController*)self.rootViewController;
 if([controller capturePoint:[controller.view convertPoint:point fromView:self]])return [super hitTest:point withEvent:event];
 return nil;
}
@end
@interface MainDelegate:UIResponder<UIApplicationDelegate>
@property(nonatomic,strong)UIWindow* window;
@end
@implementation MainDelegate
-(BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options{
 (void)app;(void)options;self.window=[[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];self.window.rootViewController=[[UINavigationController alloc] initWithRootViewController:[MainController new]];[self.window makeKeyAndVisible];return YES;
}
@end
@interface HUDDelegate:UIResponder<UIApplicationDelegate>
@property(nonatomic,strong)UIWindow* window;
@property(nonatomic,strong)id hosting;
@end
@implementation HUDDelegate
-(BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options{
 (void)app;(void)options;self.window=[[HUDWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];self.window.backgroundColor=UIColor.clearColor;self.window.rootViewController=[HUDController new];self.window.windowLevel=UIWindowLevelStatusBar+1;[self.window makeKeyAndVisible];
 Class host=NSClassFromString(@"SBSAccessibilityWindowHostingController");SEL registerSelector=NSSelectorFromString(@"registerWindowWithContextID:atLevel:");SEL contextSelector=NSSelectorFromString(@"_contextId");
 self.hosting=[host new];if(!self.hosting||![self.hosting respondsToSelector:registerSelector]||![self.window respondsToSelector:contextSelector]){record(@"HUD 窗口托管接口不可用");exit(20);}
 uint32_t context=((uint32_t(*)(id,SEL))objc_msgSend)(self.window,contextSelector);double level=self.window.windowLevel;
 NSMethodSignature* signature=[self.hosting methodSignatureForSelector:registerSelector];NSInvocation* invocation=[NSInvocation invocationWithMethodSignature:signature];invocation.target=self.hosting;invocation.selector=registerSelector;[invocation setArgument:&context atIndex:2];[invocation setArgument:&level atIndex:3];[invocation invoke];record(@"HUD 窗口已注册，正在检查授权和游戏数据");return YES;
}
@end
// Original 0x10000b294 / 0x10000b570: AXEventRepresentation from the fourth HID callback argument.
static BOOL axFlag(id event,NSString* name){SEL selector=NSSelectorFromString(name);return [event respondsToSelector:selector]&&((BOOL(*)(id,SEL))objc_msgSend)(event,selector);}
static void hidCallback(void* target,void* refcon,void* service,CFTypeRef hid){
 (void)target;(void)refcon;(void)service;if(!hid)return;
 @autoreleasepool{
  Class representation=NSClassFromString(@"AXEventRepresentation");SEL factory=NSSelectorFromString(@"representationWithHIDEvent:hidStreamIdentifier:");
  if(![representation respondsToSelector:factory])return;
  id event=((id(*)(id,SEL,CFTypeRef,id))objc_msgSend)(representation,factory,hid,@"UIApplicationEvents");
  SEL location=NSSelectorFromString(@"location");if(![event respondsToSelector:location])return;
  CGPoint point=((CGPoint(*)(id,SEL))objc_msgSend)(event,location);
  NSUInteger phase=axFlag(event,@"isTouchDown")?UITouchPhaseBegan:axFlag(event,@"isMove")?UITouchPhaseMoved:axFlag(event,@"isCancel")?UITouchPhaseCancelled:UITouchPhaseEnded;
  id hand=[event valueForKey:@"handInfo"];NSArray* paths=[hand valueForKey:@"paths"];id path=paths.firstObject;NSUInteger identity=[[path valueForKey:@"pathIdentity"] unsignedIntegerValue];if(identity==0)return;identity=MIN(identity,98);
  dispatch_async(dispatch_get_main_queue(),^{UIWindow* window=UIApplication.sharedApplication.delegate.window;HUDController* controller=(HUDController*)window.rootViewController;[controller globalTouch:point phase:phase identity:identity];});
 }
}
static BOOL installHUDInput(){
 [[NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/AccessibilityUtilities.framework"] load];
 using Register=void(*)(void(*)(void*,void*,void*,CFTypeRef));auto install=reinterpret_cast<Register>(dlsym(RTLD_DEFAULT,"BKSHIDEventRegisterEventCallback"));
 if(!install||!NSClassFromString(@"AXEventRepresentation"))return NO;install(hidCallback);return YES;
}
static BOOL stopHUD(void){
 int pid=[[NSString stringWithContentsOfFile:pidPath encoding:NSUTF8StringEncoding error:nil] intValue];if(pid<=1)return NO;
 using ProcessPath=int(*)(int,void*,uint32_t);auto processPath=reinterpret_cast<ProcessPath>(dlsym(RTLD_DEFAULT,"proc_pidpath"));
 char other[4096]{},own[4096]{};
 if(!processPath||processPath(pid,other,sizeof other)<=0||processPath(getpid(),own,sizeof own)<=0||strcmp(other,own)!=0){record(@"PID 文件与应用路径不符，未发送停止信号");return NO;}
 BOOL success=kill(pid,SIGTERM)==0;if(success)[NSFileManager.defaultManager removeItemAtPath:pidPath error:nil];return success;
}
int main(int argc,char** argv){@autoreleasepool{
 if(argc<2)return UIApplicationMain(argc,argv,nil,NSStringFromClass(MainDelegate.class));
 if(strcmp(argv[1],"-exit")==0)return stopHUD()?0:1;
 if(strcmp(argv[1],"-hud")!=0)return 2;
 // Resolve the same private startup functions observed in original entry 0x10000b780.
 dlopen("/System/Library/PrivateFrameworks/GraphicsServices.framework/GraphicsServices",RTLD_NOW);
 dlopen("/System/Library/PrivateFrameworks/BackBoardServices.framework/BackBoardServices",RTLD_NOW);
 dlopen("/System/Library/PrivateFrameworks/SpringBoardServices.framework/SpringBoardServices",RTLD_NOW);
 using VoidFn=void(*)();auto gs=(VoidFn)dlsym(RTLD_DEFAULT,"GSInitialize"),display=(VoidFn)dlsym(RTLD_DEFAULT,"BKSDisplayServicesStart"),initialize=(VoidFn)dlsym(RTLD_DEFAULT,"UIApplicationInitialize");
 auto singleton=(void(*)(Class))dlsym(RTLD_DEFAULT,"UIApplicationInstantiateSingleton");
 if(!gs||!display||!initialize||!singleton){record(@"缺少 HUD 私有启动接口");return 10;}
 stopHUD();[[NSString stringWithFormat:@"%d",getpid()] writeToFile:pidPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
 [UIScreen class];CFRunLoopGetCurrent();gs();display();initialize();singleton(HUDApplication.class);
 static HUDDelegate* delegate;delegate=[HUDDelegate new];UIApplication* app=UIApplication.sharedApplication;app.delegate=delegate;
 SEL accessibility=NSSelectorFromString(@"_accessibilityInit");if([app respondsToSelector:accessibility])((void(*)(id,SEL))objc_msgSend)(app,accessibility);
 if(!installHUDInput())record(@"HID/AX 触摸接口不可用；可在主应用修改设置");
 SEL run=NSSelectorFromString(@"__completeAndRunAsPlugin");if(![app respondsToSelector:run]){record(@"缺少 HUD 插件运行接口");return 11;}
 ((void(*)(id,SEL))objc_msgSend)(app,run);CFRunLoopRun();return 0;
}}
