#pragma once
#include "Offsets.hpp"
#include "RemoteMemory.hpp"
#include <vector>
#include <array>
#include <string>
namespace ts {
struct Region {std::uint64_t address=0,size=0;bool readable=false,writable=false,executable=false;std::string mappedFile;};
struct VMRecord {
 std::uint64_t object=0,state=0,bytecodeObject=0,code=0;
 std::uint32_t length=0;std::uint8_t nop=0,store=0,invalid=0;
 unsigned patched=0;std::uint32_t matchedMask=0,existingMask=0;
};
struct CoordinateResult {bool success=false;unsigned patched=0;std::string reason;std::vector<VMRecord> records;};
// Reconstructs the original VM coordinate initialization routine, not a general cryptographic decoder.
class CoordinateAccess {
 WritableMemory& memory_;Offsets offsets_;std::uint64_t module_=0;
 std::array<std::uint32_t,16> pcs_;
 std::vector<VMRecord> cache_;
 std::uint64_t pointer(std::uint64_t base,std::uint64_t offset)const;
 template<class T>std::optional<T> field(std::uint64_t base,std::uint64_t offset)const{auto address=add(base,offset);return address?memory_.value<T>(*address):std::nullopt;}
 bool readRecord(std::uint64_t object,VMRecord&)const;
 std::uint32_t requiredMask(std::uint32_t length)const;
 bool patchRecord(VMRecord&);
 bool verify(const VMRecord&)const;
public:
 CoordinateAccess(WritableMemory&,Offsets);
 CoordinateResult initialize(const std::vector<Region>&);
};
}
