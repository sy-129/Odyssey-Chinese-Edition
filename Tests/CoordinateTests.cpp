#include "CoordinateAccess.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
struct Memory:ts::WritableMemory {
 const std::uint64_t base=0x100000000ULL;std::vector<std::uint8_t> bytes=std::vector<std::uint8_t>(0x20000);bool rejectWrite=false,corruptWrite=false;unsigned writes=0;
 ts::ReadResult read(std::uint64_t a,void* out,std::size_t n)const override{
  if(a<base||a-base>bytes.size()||n>bytes.size()-(a-base))return {false,0};
  std::memcpy(out,bytes.data()+a-base,n);return {true,n};
 }
 bool write(std::uint64_t a,const void* in,std::size_t n)override{
  ++writes;if(rejectWrite||a<base||a-base>bytes.size()||n>bytes.size()-(a-base))return false;
  std::memcpy(bytes.data()+a-base,in,n);if(corruptWrite&&n>1)bytes[a-base+1]^=0xff;return true;
 }
 template<class T>void set(std::uint64_t offset,T value){std::memcpy(bytes.data()+offset,&value,sizeof value);}
};
struct Fixture {
 Memory memory;ts::Offsets offsets;std::vector<ts::Region> regions;
 Fixture(){auto base=memory.base;
  offsets.tsDispatcherOffset=128;offsets.tsVtableOffset=256;offsets.tsCoordCount=2;offsets.tsCoordPc0=100;offsets.tsCoordPc1=200;
  memory.set<std::uint32_t>(0,0xfeedfacf);memory.set<std::uint32_t>(128,offsets.tsDispatcherInstr);
  const std::uint64_t object=0x2001,state=0x4000,bc=0x6000,code=0x8000;
  memory.set<std::uint64_t>(object,base+offsets.tsVtableOffset);
  memory.set<std::uint64_t>(object+offsets.tsVmObjStateOff,base+state);
  memory.set<std::uint64_t>(object+offsets.tsVmObjBytecodeObjOff,base+bc);
  memory.set<std::uint8_t>(object+offsets.tsVmObjTrueNopOpcodeOff,offsets.tsOpcodeTrueNopValue);
  memory.set<std::uint8_t>(object+offsets.tsVmObjStoreOpcodeOff,offsets.tsOpcodeStoreValue);
  memory.set<std::uint8_t>(object+offsets.tsVmObjOpcodeTableOff,offsets.tsOpcodeInvalidValue);
  memory.set<std::uint8_t>(state+offsets.tsStateModeOff,offsets.tsVmMode);
  memory.set<std::uint32_t>(state+offsets.tsStateCodeLenOff,1024);
  memory.set<std::uint64_t>(bc+offsets.tsBcObjDataOff,base+code);
  memory.set<std::uint8_t>(code+100,offsets.tsOpcodeStoreValue);memory.set<std::uint8_t>(code+200,offsets.tsOpcodeStoreValue);
  regions={{base,0x1000,true,false,true,"/Frameworks/tersafe.framework/tersafe"},{base+0x2000,0x10000,true,true,false,""}};
 }
};
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
 Fixture f;ts::CoordinateAccess access(f.memory,f.offsets);auto first=access.initialize(f.regions);
 require(first.success&&first.patched==2&&first.records.size()==1,"unaligned object discovery and patch verification");
 auto second=access.initialize(f.regions);require(second.success&&second.patched==0&&f.memory.writes==2,"idempotent recheck must not write again");
 f.memory.bytes[0x8065]=0;auto changed=access.initialize(f.regions);require(!changed.success,"changing second patched byte invalidates cache");
 Fixture denied;denied.memory.rejectWrite=true;ts::CoordinateAccess deny(denied.memory,denied.offsets);require(!deny.initialize(denied.regions).success,"denied writes cannot report success");
 Fixture corrupt;corrupt.memory.corruptWrite=true;ts::CoordinateAccess corruptAccess(corrupt.memory,corrupt.offsets);require(!corruptAccess.initialize(corrupt.regions).success,"short/corrupt writes cannot report success");
 Fixture wrong;wrong.memory.set<std::uint32_t>(128,0);ts::CoordinateAccess wrongAccess(wrong.memory,wrong.offsets);require(!wrongAccess.initialize(wrong.regions).success&&wrong.memory.writes==0,"wrong module signature must prevent all writes");
 Fixture configured;ts::Offsets original;configured.offsets.tsCoordCount=original.tsCoordCount;configured.offsets.tsCoordPc0=original.tsCoordPc0;configured.offsets.tsCoordPc1=original.tsCoordPc1;
 configured.memory.set<std::uint32_t>(0x4000+original.tsStateCodeLenOff,80000);configured.regions[1].size=0x1e000;
 std::array<std::uint64_t,9> pcs{original.tsCoordPc0,original.tsCoordPc1,original.tsCoordPc2,original.tsCoordPc3,original.tsCoordPc4,original.tsCoordPc5,original.tsCoordPc6,original.tsCoordPc7,original.tsCoordPc8};
 for(auto pc:pcs){configured.memory.set<std::uint32_t>(0x8000+pc,0x13121102);}
 ts::CoordinateAccess configuredAccess(configured.memory,configured.offsets);auto complete=configuredAccess.initialize(configured.regions);
 require(complete.success&&complete.patched==9,"all nine configured PCs must be patched and read back");
 require(complete.records.size()==1&&complete.records[0].matchedMask==0x1ff,"every configured opcode contributes to the verified mask");
 require(configured.memory.bytes[0x8000+100]==original.tsOpcodeStoreValue,"unconfigured STORE bytes must remain unchanged");
 std::cout<<"9 coordinate memory fault/retry/profile checks passed\n";
}
