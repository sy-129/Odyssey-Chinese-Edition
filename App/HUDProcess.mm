#include "HUDProcess.hpp"
#include <mach-o/dyld.h>
#include <spawn.h>
#include <dlfcn.h>
#include <cerrno>
#include <cstring>
#include <vector>
extern char** environ;
namespace ts {
LaunchResult spawnHUDMode(bool enable){
 using Persona=int(*)(posix_spawnattr_t*,uid_t,uint32_t);
 using UID=int(*)(posix_spawnattr_t*,uid_t);
 using GID=int(*)(posix_spawnattr_t*,gid_t);
 auto persona=reinterpret_cast<Persona>(dlsym(RTLD_DEFAULT,"posix_spawnattr_set_persona_np"));
 auto uid=reinterpret_cast<UID>(dlsym(RTLD_DEFAULT,"posix_spawnattr_set_persona_uid_np"));
 auto gid=reinterpret_cast<GID>(dlsym(RTLD_DEFAULT,"posix_spawnattr_set_persona_gid_np"));
 if(!persona||!uid||!gid)return {false,0,ENOSYS,"系统未提供原程序使用的 persona 接口"};
 uint32_t count=0;_NSGetExecutablePath(nullptr,&count);if(!count||count>1024*1024)return {false,0,EINVAL,"无法取得当前程序路径"};
 std::vector<char> path(count);if(_NSGetExecutablePath(path.data(),&count)!=0)return {false,0,EINVAL,"程序路径读取失败"};
 posix_spawnattr_t attr;int status=posix_spawnattr_init(&attr);if(status)return {false,0,status,strerror(status)};
 struct Cleanup{posix_spawnattr_t* value;~Cleanup(){posix_spawnattr_destroy(value);}} cleanup{&attr};
 if((status=persona(&attr,99,1)) || (status=uid(&attr,0)) || (status=gid(&attr,0)))return {false,0,status,strerror(status)};
 if(enable){if((status=posix_spawnattr_setpgroup(&attr,0)) || (status=posix_spawnattr_setflags(&attr,POSIX_SPAWN_SETPGROUP)))return {false,0,status,strerror(status)};}
 char enabled[]="-hud",disabled[]="-exit";char* args[]={path.data(),enable?enabled:disabled,nullptr};pid_t pid=0;
 status=posix_spawn(&pid,path.data(),nullptr,&attr,args,environ);
 if(status)return {false,0,status,strerror(status)};
 return {true,pid,0,"子进程创建成功；这不证明 HUD 初始化或游戏读取成功"};
}
}
