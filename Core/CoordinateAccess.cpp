#include "CoordinateAccess.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <set>
namespace ts {
CoordinateAccess::CoordinateAccess(WritableMemory& memory,Offsets offsets):memory_(memory),offsets_(offsets),pcs_{
 static_cast<std::uint32_t>(offsets.tsCoordPc0),static_cast<std::uint32_t>(offsets.tsCoordPc1),static_cast<std::uint32_t>(offsets.tsCoordPc2),static_cast<std::uint32_t>(offsets.tsCoordPc3),
 static_cast<std::uint32_t>(offsets.tsCoordPc4),static_cast<std::uint32_t>(offsets.tsCoordPc5),static_cast<std::uint32_t>(offsets.tsCoordPc6),static_cast<std::uint32_t>(offsets.tsCoordPc7),
 static_cast<std::uint32_t>(offsets.tsCoordPc8),static_cast<std::uint32_t>(offsets.tsCoordPc9),static_cast<std::uint32_t>(offsets.tsCoordPc10),static_cast<std::uint32_t>(offsets.tsCoordPc11),
 static_cast<std::uint32_t>(offsets.tsCoordPc12),static_cast<std::uint32_t>(offsets.tsCoordPc13),static_cast<std::uint32_t>(offsets.tsCoordPc14),static_cast<std::uint32_t>(offsets.tsCoordPc15)}{}
std::uint64_t CoordinateAccess::pointer(std::uint64_t base,std::uint64_t offset)const{
 auto address=add(base,offset);if(!address)return 0;auto p=memory_.value<std::uint64_t>(*address);return p.value_or(0);
}
bool CoordinateAccess::readRecord(std::uint64_t object,VMRecord& result)const{
 auto table=field<std::uint64_t>(object,0);auto expected=add(module_,offsets_.tsVtableOffset);if(!table||!expected||*table!=*expected)return false;
 result={};result.object=object;result.state=pointer(object,offsets_.tsVmObjStateOff);result.bytecodeObject=pointer(object,offsets_.tsVmObjBytecodeObjOff);
 if(!result.state||!result.bytecodeObject)return false;
 auto nop=field<std::uint8_t>(object,offsets_.tsVmObjTrueNopOpcodeOff),store=field<std::uint8_t>(object,offsets_.tsVmObjStoreOpcodeOff),invalid=field<std::uint8_t>(object,offsets_.tsVmObjOpcodeTableOff);
 auto mode=field<std::uint8_t>(result.state,offsets_.tsStateModeOff);auto length=field<std::uint32_t>(result.state,offsets_.tsStateCodeLenOff);
 if(!nop||!store||!invalid||!mode||!length||*nop!=offsets_.tsOpcodeTrueNopValue||*store!=offsets_.tsOpcodeStoreValue||*invalid!=offsets_.tsOpcodeInvalidValue||*mode!=offsets_.tsVmMode)return false;
 if(*length<64||*length>offsets_.tsBytecodeScanSize)return false;
 if(offsets_.tsKillTargetCodeLen&&*length!=offsets_.tsKillTargetCodeLen)return false;
 result.code=pointer(result.bytecodeObject,offsets_.tsBcObjDataOff);if(result.code<0x100000000ULL||!add(result.code,*length))return false;
 result.length=*length;result.nop=*nop;result.store=*store;result.invalid=*invalid;return true;
}
std::uint32_t CoordinateAccess::requiredMask(std::uint32_t length)const{
 std::uint32_t mask=0;for(unsigned i=0;i<offsets_.tsCoordCount;i++)if(std::uint64_t(pcs_[i])+3<length)mask|=std::uint32_t(1)<<i;return mask;
}
bool CoordinateAccess::patchRecord(VMRecord& record){
 std::vector<std::uint8_t> code(record.length);auto read=memory_.read(record.code,code.data(),code.size());if(!read.success||read.copied!=code.size())return false;
 std::array<std::uint8_t,4> replacement{record.nop,record.nop,record.nop,record.nop};
 for(unsigned i=0;i<offsets_.tsCoordCount;i++){
  auto pc=pcs_[i];if(std::uint64_t(pc)+3>=record.length)continue;
  if(std::equal(replacement.begin(),replacement.end(),code.begin()+pc)){record.existingMask|=std::uint32_t(1)<<i;continue;}
  if(code[pc]!=record.store)continue;
  auto address=add(record.code,pc);if(!address||!memory_.write(*address,replacement.data(),replacement.size()))continue;
  auto actual=memory_.value<std::array<std::uint8_t,4>>(*address);if(!actual||*actual!=replacement)continue;
  ++record.patched;record.matchedMask|=std::uint32_t(1)<<i;
 }
 auto required=requiredMask(record.length);return record.patched>0||(required&&record.existingMask==required);
}
bool CoordinateAccess::verify(const VMRecord& record)const{
 const auto fullMask=requiredMask(record.length);
 VMRecord live;if(!readRecord(record.object,live)||live.code!=record.code||live.length!=record.length||(record.matchedMask==0&&(!fullMask||record.existingMask!=fullMask)))return false;
 for(unsigned i=0;i<offsets_.tsCoordCount;i++)if((record.matchedMask|record.existingMask)&(std::uint32_t(1)<<i)){
  if(std::uint64_t(pcs_[i])+3>=live.length)return false;
  auto value=field<std::array<std::uint8_t,4>>(live.code,pcs_[i]);
  if(!value||*value!=std::array<std::uint8_t,4>{live.nop,live.nop,live.nop,live.nop})return false;
 }return true;
}
CoordinateResult CoordinateAccess::initialize(const std::vector<Region>& regions){
 CoordinateResult result;if(offsets_.tsCoordCount==0||offsets_.tsCoordCount>16||offsets_.tsBytecodeScanSize<64||offsets_.tsBytecodeScanSize>1024*1024){result.reason="坐标配置无效";return result;}
 module_=0;
 for(const auto& region:regions){
  if(!region.executable||region.mappedFile.find("tersafe")==std::string::npos||region.size<=offsets_.tsDispatcherOffset+4)continue;
  auto magic=field<std::uint32_t>(region.address,0),instruction=field<std::uint32_t>(region.address,offsets_.tsDispatcherOffset);
  if(magic&&instruction&&*magic==0xfeedfacf&&*instruction==offsets_.tsDispatcherInstr){module_=region.address;break;}
 }
 if(!module_){result.reason="未找到匹配坐标配置的模块";return result;}
 if(!cache_.empty()){
  bool valid=true;for(const auto& record:cache_)valid=verify(record)&&valid;
  if(valid){result.success=true;result.records=cache_;result.reason="既有坐标初始化已重新读回确认";return result;}
  cache_.clear();
 }
 auto table=add(module_,offsets_.tsVtableOffset);if(!table){result.reason="模块地址溢出";return result;}
 std::set<std::uint64_t> objects,codeAddresses;std::vector<std::uint8_t> buffer(0x40007);
 for(const auto& region:regions){
  if(!region.readable||!region.writable||region.executable||region.size<4096||region.size>0x20000000ULL)continue;
  for(std::uint64_t offset=0;offset<region.size;offset+=0x40000){
   if(objects.size()>=64)break;
   auto address=add(region.address,offset);if(!address)break;
   auto remaining=region.size-offset,count=std::min<std::uint64_t>(remaining,buffer.size()),scan=std::min<std::uint64_t>(remaining,0x40000);
   auto read=memory_.read(*address,buffer.data(),count);if(!read.success||read.copied!=count)continue;
   for(std::size_t n=0;n<scan&&n+8<=count;n++){
    std::uint64_t value;std::memcpy(&value,buffer.data()+n,8);if(value!=*table)continue;
    auto object=add(*address,n);if(!object||objects.count(*object))continue;VMRecord record;
    if(!readRecord(*object,record)||codeAddresses.count(record.code))continue;
    objects.insert(*object);codeAddresses.insert(record.code);patchRecord(record);result.patched+=record.patched;result.records.push_back(record);
    if(objects.size()>=64)break;
   }
  }
 }
 for(const auto& record:result.records)if(verify(record)){result.success=true;cache_.push_back(record);}
 result.reason=result.success?(result.patched?"坐标初始化已写入并读回确认":"已读回确认全部既有坐标初始化位置"):"未找到可完成坐标初始化的有效字节码对象";return result;
}
}
