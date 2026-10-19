#pragma once
#include "Geometry.hpp"
#include "Settings.hpp"
#include "imgui.h"
namespace ts {
class SettingsMenu {
 bool visible_=false;Vec2 ball_{},lastViewport_{};MapRect bounds_{};float radius_=90;int category_=0;
 bool fields(Settings&);
public:
 bool capture(Vec2)const;
 bool draw(Settings&,Vec2 viewport,ImTextureID floatingImage=nullptr);
};
}
