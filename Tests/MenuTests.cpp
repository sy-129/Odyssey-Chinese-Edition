#include "SettingsMenu.hpp"
#include "SettingsValidation.hpp"
#include "imgui.h"
#include <iostream>
#include <limits>
#include <stdexcept>
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(){
 ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.DisplaySize={2532,1170};io.DeltaTime=1.f/60;
 unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
 ts::Settings settings;ts::SettingsMenu menu;auto frame=[&](){ImGui::NewFrame();menu.draw(settings,{2532,1170});ImGui::Render();};
 frame();require(menu.capture({2424,409.5f}),"floating control hit region");require(!menu.capture({10,1100}),"game area passes through while closed");
 io.AddMousePosEvent(2424,409.5f);frame();io.AddMouseButtonEvent(0,true);frame();io.AddMouseButtonEvent(0,false);frame();
 require(menu.capture({100,100}),"release opens settings hit region");require(!menu.capture({10,1100}),"outside expanded menu still passes through");
 settings.heroScaleX=0;settings.miniMapWidth=-5;settings.bigMapScale=std::numeric_limits<float>::quiet_NaN();ts::validateSettings(settings);
 require(settings.heroScaleX>0&&settings.miniMapWidth>=1&&settings.bigMapScale==ts::Settings{}.bigMapScale,"invalid settings cannot produce divided-by-zero geometry");
 ImGui::DestroyContext();std::cout<<"5 menu interaction and invalid-setting checks passed\n";
}
