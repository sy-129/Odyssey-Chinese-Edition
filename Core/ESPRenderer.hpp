#pragma once
#include "GameData.hpp"
#include "Settings.hpp"
#include "imgui.h"
namespace ts {
class Textures {
public:
 virtual ~Textures()=default;
 virtual ImTextureID hero(int id)=0;
 virtual ImTextureID skill(int id)=0;
};
class ESPRenderer {
 Textures& textures_;
 void text(ImDrawList*,Vec2,float,const char*,ImU32);
 void recallBorder(ImDrawList*,Vec2,Vec2,float,float);
 void timer(ImDrawList*,Vec2,Vec2,float,float);
 void icon(ImDrawList*,ImTextureID,Vec2,float,int,float,bool);
public:
 explicit ESPRenderer(Textures& textures):textures_(textures){}
 void draw(ImDrawList*,const Frame&,const Settings&,Vec2 viewport);
};
}
