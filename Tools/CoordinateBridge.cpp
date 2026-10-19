#include "CoordinateAccess.hpp"
#include <cstring>
namespace {
constexpr std::uint64_t base=0x500000000ULL,code=base+0x310000;
class BorrowedMemory:public ts::WritableMemory{
 unsigned char* bytes_;std::size_t size_;int mode_;
public:
 BorrowedMemory(unsigned char* bytes,std::size_t size,int mode):bytes_(bytes),size_(size),mode_(mode){}
 ts::ReadResult read(std::uint64_t address,void* out,std::size_t n)const override{
  if(address<base||address-base>size_||n>size_-(address-base))return {false,0};
  std::size_t count=mode_==3&&address>=code&&address<code+80000&&n==4?3:n;
  std::memcpy(out,bytes_+address-base,count);return {true,count};
 }
 bool write(std::uint64_t address,const void* in,std::size_t n)override{
  if(mode_==1||address<base||address-base>size_||n>size_-(address-base))return false;
  std::memcpy(bytes_+address-base,in,n);if(mode_==2&&n>1)bytes_[address-base+1]^=0xff;return true;
 }
};
}
extern "C" unsigned ts_coordinate_run(unsigned char* memory,std::size_t size,int mode,unsigned* mask){
 BorrowedMemory fixture(memory,size,mode);ts::CoordinateAccess access(fixture,ts::Offsets{});
 auto result=access.initialize({{base,0x280000,true,false,true,"/Frameworks/tersafe.framework/tersafe"},{base+0x300000,0x40000,true,true,false,""}});
 *mask=0;for(const auto& record:result.records)*mask|=record.matchedMask;return result.patched;
}
