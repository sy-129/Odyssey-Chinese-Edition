#include "MachSession.hpp"
#include <sys/sysctl.h>
#include <unistd.h>
#include <mach/vm_map.h>
#include <mach/vm_region.h>
#include <vector>
#include <cstring>
#include <limits>
#include <dlfcn.h>
MachSession::~MachSession(){close();}
void MachSession::close(){if(task_!=MACH_PORT_NULL)mach_port_deallocate(mach_task_self(),task_);task_=MACH_PORT_NULL;}
bool MachSession::attach(pid_t pid,std::string& error){
 close();if(pid<1){error="无效进程编号";return false;}
 kern_return_t kr=KERN_FAILURE;
 // REA _SmobaMachTarget_Init: five attempts, 40ms between attempts.
 for(int i=0;i<5;i++){
  mach_port_t candidate=MACH_PORT_NULL;kr=task_for_pid(mach_task_self(),pid,&candidate);
  if(kr==KERN_SUCCESS&&candidate!=MACH_PORT_NULL){task_=candidate;return true;}
  if(candidate!=MACH_PORT_NULL)mach_port_deallocate(mach_task_self(),candidate);
  if(i!=4)usleep(40000);
 }
 error="task_for_pid 失败 ("+std::to_string(kr)+"): "+mach_error_string(kr);return false;
}
ts::ReadResult MachSession::read(std::uint64_t address,void* destination,std::size_t count)const{
 if(task_==MACH_PORT_NULL||!destination||count==0||!ts::add(address,count))return {false,0};
 vm_size_t copied=0;auto kr=vm_read_overwrite(task_,address,count,reinterpret_cast<vm_address_t>(destination),&copied);
 return {kr==KERN_SUCCESS,static_cast<std::size_t>(copied)};
}
pid_t MachSession::findProcess(const std::string& name,std::string& error){
 if(name.empty()||name.size()>16){error="请输入1–16字节的进程名";return -1;}
 if(name=="self")return getpid();
 int mib[]={CTL_KERN,KERN_PROC,KERN_PROC_ALL,0};size_t bytes=0;
 if(sysctl(mib,3,nullptr,&bytes,nullptr,0)!=0){error="系统拒绝枚举进程";return -1;}
 for(int attempt=0;attempt<3;attempt++){
  std::vector<kinfo_proc> list(bytes/sizeof(kinfo_proc)+32);bytes=list.size()*sizeof(kinfo_proc);
  if(sysctl(mib,3,list.data(),&bytes,nullptr,0)==0){
   for(size_t i=0;i<bytes/sizeof(kinfo_proc);i++)if(strncmp(list[i].kp_proc.p_comm,name.c_str(),sizeof list[i].kp_proc.p_comm)==0)return list[i].kp_proc.p_pid;
   error="未找到目标进程："+name;return -1;
  }
 }
 error="进程枚举失败";return -1;
}
std::optional<ts::Image> MachSession::findGameImage(std::string& error)const{
 if(task_==MACH_PORT_NULL){error="尚未取得进程访问句柄";return {};}
 vm_address_t address=0;natural_t depth=0;std::optional<ts::Image> executable;
 // REA FUN_10002d908: inspect readable/executable mappings and prefer UnityFramework.
 for(unsigned count=0;count<65536;count++){
  vm_size_t size=0;vm_region_submap_info_data_64_t info{};mach_msg_type_number_t words=VM_REGION_SUBMAP_INFO_COUNT_64;
  auto kr=vm_region_recurse_64(task_,&address,&size,&depth,reinterpret_cast<vm_region_recurse_info_t>(&info),&words);
  if(kr==KERN_INVALID_ADDRESS)break;
  if(kr!=KERN_SUCCESS){error="内存区域枚举失败："+std::string(mach_error_string(kr));return {};}
  if(info.is_submap){if(depth>=64){error="内存映射嵌套超过限制";return {};}++depth;continue;}
  if((info.protection&(VM_PROT_READ|VM_PROT_EXECUTE))==(VM_PROT_READ|VM_PROT_EXECUTE)){
   auto image=ts::inspectImage(*this,address);
   if(image){if(image->fileType==2&&!executable)executable=image;
    if(image->fileType==6&&image->installName.find("UnityFramework")!=std::string::npos)return image;}
  }
  auto next=ts::add(address,size);if(!next||*next<=address){error="内存区域边界无效";return {};}address=*next;
 }
 if(executable)return executable;
 error="未找到可读取的 Mach-O 映像";return {};
}

bool MachSession::write(std::uint64_t address,const void* bytes,std::size_t count){
 if(task_==MACH_PORT_NULL||!bytes||count==0||count>UINT32_MAX||!ts::add(address,count))return false;
 return vm_write(task_,address,reinterpret_cast<vm_offset_t>(bytes),static_cast<mach_msg_type_number_t>(count))==KERN_SUCCESS;
}
std::vector<ts::Region> MachSession::regions(std::string& error)const{
 std::vector<ts::Region> result;if(task_==MACH_PORT_NULL){error="尚未取得进程句柄";return result;}
 pid_t pid=0;pid_for_task(task_,&pid);vm_address_t address=0;natural_t depth=0;
 for(unsigned i=0;i<65536;i++){
  vm_size_t size=0;vm_region_submap_info_data_64_t info{};mach_msg_type_number_t words=VM_REGION_SUBMAP_INFO_COUNT_64;
  auto kr=vm_region_recurse_64(task_,&address,&size,&depth,reinterpret_cast<vm_region_recurse_info_t>(&info),&words);
  if(kr==KERN_INVALID_ADDRESS)break;if(kr!=KERN_SUCCESS){error=mach_error_string(kr);return {};}
  if(info.is_submap){if(depth>=64){error="内存区域嵌套过深";return {};}++depth;continue;}
  char path[4096]{};using RegionFilename=int(*)(int,uint64_t,void*,uint32_t);
  static auto filename=reinterpret_cast<RegionFilename>(dlsym(RTLD_DEFAULT,"proc_regionfilename"));
  if(pid>0&&filename)filename(pid,address,path,sizeof path);
  result.push_back({address,size,bool(info.protection&VM_PROT_READ),bool(info.protection&VM_PROT_WRITE),bool(info.protection&VM_PROT_EXECUTE),path});
  auto next=ts::add(address,size);if(!next||*next<=address)break;address=*next;
 }
 return result;
}
