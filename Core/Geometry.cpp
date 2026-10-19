#include "Geometry.hpp"
#include <cmath>
namespace ts {
bool toScreen(Vec2 viewport,Vec2 p,const Matrix& m,Vec2& out){
 out={};float w=m[14]+std::fma(m[2],p.x,p.y*m[10]);
 if(w<0.01f)return false;
 float x=m[12]+std::fma(m[0],p.x,p.y*m[8]);
 float y=m[13]+std::fma(m[1],p.x,p.y*m[9]);float reciprocal=1.f/w;
 out.x=(viewport.x*std::fma(x,reciprocal,1.f))*.5f;
 out.y=(viewport.y*std::fma(-y,reciprocal,1.f))*.5f;return true;
}
Vec2 toMiniMap(MapRect r,Vec2 p,const Matrix& m,float sx,float sy){
 float sign=m[0]>0?1.f:-1.f;float size=(r.width+r.height)*.5f;
 float x=std::fma((p.x*sign)/sx,size,r.x);
 // Original 0x100034c7c/0x100034c80 uses separate FMUL then FSUB.
 // Retain the intermediate float rounding even when Clang contracts expressions.
 volatile float scaledY=size*((p.y*sign)/sy);
 float y=r.y-scaledY;
 return {std::fma(size,.5f,x),std::fma(size,.5f,y)};
}
std::optional<Vec2> decodePosition(std::int32_t x,std::int32_t y){
 if((x==0&&y==0)||std::abs(std::int64_t(x))>80000||std::abs(std::int64_t(y))>80000)return {};
 return Vec2{float(x)/1000.f,float(y)/1000.f};
}
Vec2 PositionSmoother::sample(std::uint64_t id,Vec2 p){
 if(p.x==0.f&&p.y==0.f)return p;
 auto found=values_.find(id);if(found==values_.end()){values_[id]=p;return p;}
 auto& old=found->second;old={std::fma(p.x-old.x,.15f,old.x),std::fma(p.y-old.y,.15f,old.y)};return old;
}
}
