#import "HUDController.hpp"
#import <MetalKit/MetalKit.h>
#include "MetalTextures.hpp"
#include "Engine.hpp"
#include "SettingsStore.hpp"
#include "imgui_impl_metal.h"
#include <memory>
#include <algorithm>
@interface HUDController()<MTKViewDelegate>{
 id<MTLDevice> _device;id<MTLCommandQueue> _queue;
 std::unique_ptr<MetalTextures> _textures;std::unique_ptr<ts::ESPRenderer> _renderer;Engine _engine;CFTimeInterval _lastFrame;
}
@end
@implementation HUDController
-(void)loadView{
 _device=MTLCreateSystemDefaultDevice();_queue=[_device newCommandQueue];
 MTKView* view=[[MTKView alloc] initWithFrame:UIScreen.mainScreen.bounds device:_device];
 view.backgroundColor=UIColor.clearColor;view.opaque=NO;view.clearColor=MTLClearColorMake(0,0,0,0);view.userInteractionEnabled=NO;
 view.delegate=self;view.preferredFramesPerSecond=120;self.view=view;
 IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::GetIO().IniFilename=nullptr;
 // Use the system's bundled Chinese font when available; do not download font code.
 const char* paths[]={"/System/Library/Fonts/LanguageSupport/PingFang.ttc","/System/Library/Fonts/Core/PingFang.ttc"};
 for(auto path:paths)if([[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithUTF8String:path]]){if(ImGui::GetIO().Fonts->AddFontFromFileTTF(path,24,nullptr,ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon()))break;}
 ImGui_ImplMetal_Init(_device);_textures=std::make_unique<MetalTextures>(_device);_renderer=std::make_unique<ts::ESPRenderer>(*_textures);_lastFrame=CACurrentMediaTime();_engine.start();
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
  auto settings=ts::loadSettings();view.preferredFramesPerSecond=std::clamp(settings.frameRate,30,120);
  ImGui_ImplMetal_NewFrame(descriptor);ImGui::NewFrame();
  auto frame=_engine.frame();_renderer->draw(ImGui::GetBackgroundDrawList(),frame,settings,{io.DisplaySize.x,io.DisplaySize.y});
  if(frame.timestamp==0)ImGui::GetBackgroundDrawList()->AddText({24,60},0xffffffff,_engine.status().c_str());
  ImGui::Render();auto command=[_queue commandBuffer];auto encoder=[command renderCommandEncoderWithDescriptor:descriptor];
  ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),command,encoder);[encoder endEncoding];[command presentDrawable:drawable];[command commit];
 }
}
@end
