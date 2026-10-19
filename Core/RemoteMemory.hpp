#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
namespace ts {
struct ReadResult { bool success; std::size_t copied; };
class Memory {
public:
 virtual ~Memory() = default;
 virtual ReadResult read(std::uint64_t address, void* destination, std::size_t count) const = 0;
 template<class T> std::optional<T> value(std::uint64_t address) const {
  static_assert(std::is_trivially_copyable<T>::value, "plain values only");
  T v{}; if (address > std::numeric_limits<std::uint64_t>::max() - sizeof(T)) return {};
  auto r=read(address,&v,sizeof v);if(!r.success || r.copied!=sizeof v)return {};return v;
 }
};
inline std::optional<std::uint64_t> add(std::uint64_t a,std::uint64_t b) {
 if(b>std::numeric_limits<std::uint64_t>::max()-a){return {};}
 return a+b;
}
// REA FUN_100029420: a complete 0x40-byte matrix read is required.
inline std::optional<std::array<float,16>> matrix(const Memory& m,std::uint64_t address) {
 return m.value<std::array<float,16>>(address);
}
struct Image { std::uint64_t address; std::uint32_t fileType; std::string installName; };
// Parse a caller-selected mapped Mach-O header. Never executes the input.
std::optional<Image> inspectImage(const Memory&,std::uint64_t address);
}

namespace ts {
class WritableMemory : public Memory {
public:
 virtual bool write(std::uint64_t address,const void* bytes,std::size_t count)=0;
};
}
