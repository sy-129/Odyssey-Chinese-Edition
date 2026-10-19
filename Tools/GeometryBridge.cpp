#include "Geometry.hpp"
extern "C" int ts_screen(float width,float height,float x,float y,const float* matrix,float* output){ts::Matrix m;std::copy(matrix,matrix+16,m.begin());ts::Vec2 result;bool yes=ts::toScreen({width,height},{x,y},m,result);output[0]=result.x;output[1]=result.y;return yes;}
extern "C" void ts_map(float x,float y,float width,float height,float px,float py,float first,float sx,float sy,float* output){ts::Matrix m{};m[0]=first;auto result=ts::toMiniMap({x,y,width,height},{px,py},m,sx,sy);output[0]=result.x;output[1]=result.y;}
