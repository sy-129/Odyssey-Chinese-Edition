#import "HUDController.hpp"
#import <MetalKit/MetalKit.h>
#include "MetalTextures.hpp"
#include "Engine.hpp"
#include "SettingsStore.hpp"
#include "SettingsMenu.hpp"
#include "imgui_impl_metal.h"
#include <memory>
#include <algorithm>
@interface HUDController()<MTKViewDelegate>{
 id<MTLDevice> _device;id<MTLCommandQueue> _queue;
 std::unique_ptr<MetalTextures> _textures;std::unique_ptr<ts::ESPRenderer> _renderer;Engine _engine;CFTimeInterval _lastFrame,_lastSettings;ts::Settings _settings;ts::SettingsMenu _menu;NSInteger _activeTouch;
}
@end
@implementation HUDController
-(void)loadView{
 _device=MTLCreateSystemDefaultDevice();_queue=[_device newCommandQueue];
 MTKView* view=[[MTKView alloc] initWithFrame:UIScreen.mainScreen.bounds device:_device];
 view.backgroundColor=UIColor.clearColor;view.opaque=NO;view.clearColor=MTLClearColorMake(0,0,0,0);view.userInteractionEnabled=YES;
 view.delegate=self;view.preferredFramesPerSecond=120;self.view=view;
 IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::GetIO().IniFilename=nullptr;
 // Use the system's bundled Chinese font when available; do not download font code.
 const char* paths[]={"/System/Library/Fonts/LanguageSupport/PingFang.ttc","/System/Library/Fonts/Core/PingFang.ttc"};
 for(auto path:paths)if([[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithUTF8String:path]]){if(ImGui::GetIO().Fonts->AddFontFromFileTTF(path,24,nullptr,ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon()))break;}
 ImGui::GetStyle().ScaleAllSizes(2.4f);_settings=ts::loadSettings();_activeTouch=-1;
 ImGui_ImplMetal_Init(_device);_textures=std::make_unique<MetalTextures>(_device);_renderer=std::make_unique<ts::ESPRenderer>(*_textures);_lastFrame=CACurrentMediaTime();_engine.start();
}
-(BOOL)capturePoint:(CGPoint)point{
 CGFloat scale=self.view.window.screen.scale?:UIScreen.mainScreen.scale;
 return _menu.capture({float(point.x*scale),float(point.y*scale)});
}
-(void)globalTouch:(CGPoint)point phase:(NSUInteger)phase identity:(NSUInteger)identity{
 if(!ImGui::GetCurrentContext())return;
 CGPoint local=[self.view convertPoint:point fromCoordinateSpace:UIScreen.mainScreen.fixedCoordinateSpace];
 if(phase==UITouchPhaseBegan){if(_activeTouch>=0||![self capturePoint:local])return;_activeTouch=identity;}
 if(_activeTouch!=(NSInteger)identity)return;
 CGFloat scale=self.view.window.screen.scale?:UIScreen.mainScreen.scale;auto& io=ImGui::GetIO();
 io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);io.AddMousePosEvent(float(local.x*scale),float(local.y*scale));
 if(phase==UITouchPhaseBegan)io.AddMouseButtonEvent(0,true);
 if(phase==UITouchPhaseEnded||phase==UITouchPhaseCancelled){io.AddMouseButtonEvent(0,false);_activeTouch=-1;}
}
-(void)dealloc{_engine.stop();if(ImGui::GetCurrentContext()){ImGui_ImplMetal_Shutdown();ImGui::DestroyContext();}}
-(BOOL)prefersStatusBarHidden{return YES;}
-(UIInterfaceOrientationMask)supportedInterfaceOrientations{return UIInterfaceOrientationMaskAll;}
-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size{(void)view;(void)size;}
-(void)drawInMTKView:(MTKView*)view{
 @autoreleasepool {
  auto descriptor=view.currentRenderPassDescriptor;id<CAMetalDrawable> drawable=view.currentDrawable;if(!descriptor||!drawable)return;
  auto& io=ImGui::GetIO();CGFloat scale=view.window.screen.scale?:UIScreen.mainScreen.scale;
  // The recovered layout values use physical pixels, rather than UIKit points.
  io.DisplaySize={float(view.bounds.size.width*scale),float(view.bounds.size.height*scale)};io.DisplayFramebufferScale={1,1};
  CFTimeInterval time=CACurrentMediaTime();io.DeltaTime=float(std::max(time-_lastFrame,1./1000));_lastFrame=time;
  if(time-_lastSettings>.3&&!ImGui::IsAnyItemActive()){_settings=ts::loadSettings();_lastSettings=time;}
  view.preferredFramesPerSecond=std::clamp(_settings.frameRate,30,120);
  ImGui_ImplMetal_NewFrame(descriptor);ImGui::NewFrame();
  if(_menu.draw(_settings,{io.DisplaySize.x,io.DisplaySize.y},_textures->floating())){ts::saveSettings(_settings);}
  auto frame=_engine.frame();_renderer->draw(ImGui::GetBackgroundDrawList(),frame,_settings,{io.DisplaySize.x,io.DisplaySize.y});
  if(frame.timestamp==0)ImGui::GetBackgroundDrawList()->AddText({24,60},0xffffffff,_engine.status().c_str());
  ImGui::Render();auto command=[_queue commandBuffer];auto encoder=[command renderCommandEncoderWithDescriptor:descriptor];
  ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),command,encoder);[encoder endEncoding];[command presentDrawable:drawable];[command commit];
 }
}
@end
