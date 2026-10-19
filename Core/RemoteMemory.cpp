#include "RemoteMemory.hpp"
#include <algorithm>
#include <vector>
namespace ts {
namespace { std::uint32_t u32(const std::uint8_t* p){std::uint32_t x;std::memcpy(&x,p,4);return x;} }
std::optional<Image> inspectImage(const Memory& m,std::uint64_t address) {
 std::array<std::uint8_t,32> h{};auto r=m.read(address,h.data(),h.size());
 if(!r.success || r.copied!=h.size() || u32(h.data())!=0xfeedfacf)return {};
 auto commands=u32(h.data()+16),bytes=u32(h.data()+20);
 // Reconstruction hardening; not a recovered original constant.
 if(commands>4096 || bytes>4*1024*1024 || commands>bytes/8)return {};
 auto start=add(address,32),end=start?add(*start,bytes):std::nullopt;if(!end)return {};
 Image image{address,u32(h.data()+12),{}};std::uint64_t cursor=*start;
 for(std::uint32_t n=0;n<commands;n++){
  std::array<std::uint8_t,8> lc{};r=m.read(cursor,lc.data(),lc.size());
  if(!r.success||r.copied!=lc.size())return {};
  auto cmd=u32(lc.data()),size=u32(lc.data()+4);auto next=add(cursor,size);
  if(size<8||!next||*next>*end)return {};
  if(cmd==0xd){ // LC_ID_DYLIB; original module finder searches this name.
   if(size<24)return {};
   auto field=add(cursor,8);if(!field)return {};
   auto offset=m.value<std::uint32_t>(*field);if(!offset||*offset<24||*offset>=size)return {};
   auto at=add(cursor,*offset);if(!at)return {};
   auto count=std::min<std::size_t>(size-*offset,4096);std::vector<char> name(count);
   r=m.read(*at,name.data(),count);if(!r.success||r.copied!=count)return {};
   auto nul=std::find(name.begin(),name.end(),'\0');if(nul==name.end())return {};
   image.installName.assign(name.begin(),nul);
  }
  cursor=*next;
 }
 return image;
}
}
