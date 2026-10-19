#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include "Settings.hpp"
namespace ts {
struct Vec2 {float x=0,y=0;};
struct MapRect {float x,y,width,height;};
using Matrix=std::array<float,16>;
// Recovered from ToScreen 0x100028f48, including ARM64 fused operations.
bool toScreen(Vec2 viewport,Vec2 world,const Matrix&,Vec2& result);
// Recovered from 0x100034bec ARM64 S0/S1 returns; the initial pseudocode was mistyped.
Vec2 toMiniMap(MapRect,Vec2 world,const Matrix&,float scaleX,float scaleY);
std::optional<Vec2> decodePosition(std::int32_t x,std::int32_t y);
class PositionSmoother {
 std::map<std::uint64_t,Vec2> values_;
public:
 Vec2 sample(std::uint64_t identity,Vec2 position);
 void clear(){values_.clear();}
};
}
