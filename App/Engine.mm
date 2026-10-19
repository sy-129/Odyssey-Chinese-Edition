#include "Engine.hpp"
#include "MachSession.hpp"
#import "TrollSpeedRebuild-Swift.h"
#include <chrono>
namespace {double now(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}}
void Engine::status(std::string s){{std::lock_guard<std::mutex> guard(mutex_);status_=std::move(s);}publish();}
void Engine::publish(){
 double time=now();if(time-lastReport_<1)return;lastReport_=time;
 ts::Frame snapshot;std::string message;{std::lock_guard<std::mutex> guard(mutex_);snapshot=frame_;message=status_;}
 NSMutableArray* heroes=[NSMutableArray new];for(const auto& h:snapshot.heroes){if(heroes.count>=5)break;[heroes addObject:@{@"hero_id":@(h.id),@"x":@(h.position.x),@"y":@(h.position.y),@"hp":@(h.hp),@"max_hp":@(h.maxHP),@"ultimate_cd":@(h.ultimateCD),@"summoner_cd":@(h.summonerCD)}];}
 NSMutableArray* matrix=[NSMutableArray new];for(float element:snapshot.matrix)[matrix addObject:@(element)];
 NSDictionary* report=@{@"recorded_at":@([[NSDate date] timeIntervalSince1970]),@"build_commit":[NSBundle.mainBundle objectForInfoDictionaryKey:@"ReconstructionCommit"]?:@"local build",@"status":[NSString stringWithUTF8String:message.c_str()],@"target_pid":@(diagnosticPID_),@"module_address":[NSString stringWithFormat:@"0x%llx",(unsigned long long)diagnosticModule_],@"coordinate_memory_verified":@(coordinatesVerified_),@"confirmed_opcode_locations":@(confirmedLocations_),@"frame_timestamp":@(snapshot.timestamp),@"hero_count":@(snapshot.heroes.size()),@"minion_count":@(snapshot.minions.size()),@"monster_count":@(snapshot.monsters.size()),@"heroes":heroes,@"matrix":matrix,@"offset_profile":@"original embedded; game-version compatibility requires observation"};
 NSError* error=nil;NSData* data=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:&error];
 if(data&&![data writeToFile:@"/var/mobile/Library/Caches/local.trollspeed.rebuild.status.json" options:NSDataWritingAtomic error:&error])NSLog(@"Cannot write diagnostics: %@",error);
}
std::string Engine::status()const{std::lock_guard<std::mutex> guard(mutex_);return status_;}
ts::Frame Engine::frame()const{std::lock_guard<std::mutex> guard(mutex_);return frame_;}
void Engine::start(){if(running_.exchange(true))return;if(worker_.joinable())worker_.join();worker_=std::thread(&Engine::run,this);}
void Engine::stop(){running_=false;if(worker_.joinable())worker_.join();std::lock_guard<std::mutex> guard(mutex_);frame_={};}
void Engine::run(){
 @autoreleasepool {
  while(running_){
   diagnosticPID_=0;diagnosticModule_=0;coordinatesVerified_=false;confirmedLocations_=0;
   if(![TSLicenseBridge hasValidLease]){status("授权无效或到期，请返回应用激活");break;}
   MachSession session;std::string error;pid_t pid=MachSession::findProcess("smoba",error);
   diagnosticPID_=pid>0?pid:0;
   if(pid<1||!session.attach(pid,error)){status(error);for(int i=0;i<20&&running_;i++)std::this_thread::sleep_for(std::chrono::milliseconds(50));continue;}
   auto image=session.findGameImage(error);if(!image){status(error);std::this_thread::sleep_for(std::chrono::milliseconds(250));continue;}
   diagnosticModule_=image->address;
   ts::Offsets offsets;ts::CoordinateAccess coordinates(session,offsets);ts::GameReader reader(session,offsets,image->address,pid);
   double nextCoordinates=0,nextAuthorization=0,nextRenewal=0;bool coordinateReady=false;
   while(running_){
    @autoreleasepool {
     double time=now();
     if(time>=nextRenewal){nextRenewal=time+30;[TSLicenseBridge renewIfNeeded];}
     if(time>=nextAuthorization){nextAuthorization=time+1;if(![TSLicenseBridge hasValidLease]){status("授权已到期，请返回应用续期");running_=false;break;}}
     if(time>=nextCoordinates){nextCoordinates=time+3;if(!coordinateReady)status("正在查找并验证坐标对象");error.clear();auto regions=session.regions(error);
      if(!error.empty()){{std::lock_guard<std::mutex> guard(mutex_);frame_={};}status(error);break;}
      auto result=coordinates.initialize(regions);coordinateReady=result.success;coordinatesVerified_=result.success;confirmedLocations_=0;for(const auto& record:result.records)confirmedLocations_+=__builtin_popcount(record.matchedMask|record.existingMask);
      if(!coordinateReady){std::lock_guard<std::mutex> guard(mutex_);frame_={};}
      status(result.reason);
     }
     if(coordinateReady){auto capture=reader.capture(time,true);if(capture){std::lock_guard<std::mutex> guard(mutex_);frame_=std::move(*capture);status_="正在读取："+std::to_string(frame_.heroes.size())+" 位敌方英雄";}else{{std::lock_guard<std::mutex> guard(mutex_);frame_={};}status("矩阵或数据根节点不可读；等待对局数据");}}
     publish();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(8));
   }
   {std::lock_guard<std::mutex> guard(mutex_);frame_={};}
  }
  std::lock_guard<std::mutex> guard(mutex_);frame_={};running_=false;
 }
}
