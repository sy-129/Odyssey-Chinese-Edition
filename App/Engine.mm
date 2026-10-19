#include "Engine.hpp"
#include "MachSession.hpp"
#import "TrollSpeedRebuild-Swift.h"
#include <chrono>
namespace {double now(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}}
void Engine::status(std::string s){std::lock_guard<std::mutex> guard(mutex_);status_=std::move(s);}
std::string Engine::status()const{std::lock_guard<std::mutex> guard(mutex_);return status_;}
ts::Frame Engine::frame()const{std::lock_guard<std::mutex> guard(mutex_);return frame_;}
void Engine::start(){if(running_.exchange(true))return;if(worker_.joinable())worker_.join();worker_=std::thread(&Engine::run,this);}
void Engine::stop(){running_=false;if(worker_.joinable())worker_.join();std::lock_guard<std::mutex> guard(mutex_);frame_={};}
void Engine::run(){
 @autoreleasepool {
  while(running_){
   if(![TSLicenseBridge hasValidLease]){status("授权无效或到期，请返回应用激活");break;}
   MachSession session;std::string error;pid_t pid=MachSession::findProcess("smoba",error);
   if(pid<1||!session.attach(pid,error)){status(error);for(int i=0;i<20&&running_;i++)std::this_thread::sleep_for(std::chrono::milliseconds(50));continue;}
   auto image=session.findGameImage(error);if(!image){status(error);std::this_thread::sleep_for(std::chrono::milliseconds(250));continue;}
   ts::Offsets offsets;ts::CoordinateAccess coordinates(session,offsets);ts::GameReader reader(session,offsets,image->address,pid);
   double nextCoordinates=0,nextAuthorization=0,nextRenewal=0;bool coordinateReady=false;
   while(running_){
    @autoreleasepool {
     double time=now();
     if(time>=nextRenewal){nextRenewal=time+30;[TSLicenseBridge renewIfNeeded];}
     if(time>=nextAuthorization){nextAuthorization=time+1;if(![TSLicenseBridge hasValidLease]){status("授权已到期，请返回应用续期");running_=false;break;}}
     if(time>=nextCoordinates){nextCoordinates=time+3;error.clear();auto regions=session.regions(error);
      if(!error.empty()){status(error);break;}
      auto result=coordinates.initialize(regions);coordinateReady=result.success;status(result.reason);
      if(!coordinateReady){std::lock_guard<std::mutex> guard(mutex_);frame_={};}
     }
     if(coordinateReady){auto capture=reader.capture(time,true);if(capture){std::lock_guard<std::mutex> guard(mutex_);frame_=std::move(*capture);status_="正在读取："+std::to_string(frame_.heroes.size())+" 位敌方英雄";}else{status("矩阵或数据根节点不可读；等待对局数据");std::lock_guard<std::mutex> guard(mutex_);frame_={};}}
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(8));
   }
   {std::lock_guard<std::mutex> guard(mutex_);frame_={};}
  }
  std::lock_guard<std::mutex> guard(mutex_);frame_={};running_=false;
 }
}
