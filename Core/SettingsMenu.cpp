#include "SettingsMenu.hpp"
#include "SettingsValidation.hpp"
#include "imgui.h"
#include <algorithm>
#include <cmath>
namespace ts {
bool SettingsMenu::capture(Vec2 p)const{
 float x=p.x-ball_.x,y=p.y-ball_.y;
 return x*x+y*y<=radius_*radius_||(visible_&&p.x>=bounds_.x&&p.y>=bounds_.y&&p.x<=bounds_.x+bounds_.width&&p.y<=bounds_.y+bounds_.height);
}
bool SettingsMenu::fields(Settings& s){bool changed=false;
 if(category_==0)changed=ImGui::Checkbox("启用 ESP##enableESP",&s.enableESP)||changed;
 if(category_==0)changed=ImGui::Checkbox("英雄角框##showHeroBox",&s.showHeroBox)||changed;
 if(category_==0)changed=ImGui::Checkbox("英雄血量##showHeroHP",&s.showHeroHP)||changed;
 if(category_==0)changed=ImGui::Checkbox("英雄编号##showHeroID",&s.showHeroID)||changed;
 if(category_==0)changed=ImGui::Checkbox("小地图##showMiniMap",&s.showMiniMap)||changed;
 if(category_==0)changed=ImGui::Checkbox("小地图头像##showAvatar",&s.showAvatar)||changed;
 if(category_==0)changed=ImGui::Checkbox("人物上方头像##showBoxAvatar",&s.showBoxAvatar)||changed;
 if(category_==0)changed=ImGui::Checkbox("小地图边框##showMiniMapFrame",&s.showMiniMapFrame)||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图 X##miniMapX",&s.miniMapX,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图 Y##miniMapY",&s.miniMapY,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图大小##miniMapSize",&s.miniMapSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图宽度##miniMapWidth",&s.miniMapWidth,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图高度##miniMapHeight",&s.miniMapHeight,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图头像比例##miniMapAvatarScale",&s.miniMapAvatarScale,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("人物框比例##bigMapScale",&s.bigMapScale,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小兵点大小##minionPointSize",&s.minionPointSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("野怪点大小##monsterPointSize",&s.monsterPointSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("野怪计时字号##monsterTimerFontSize",&s.monsterTimerFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图回城字号##miniMapRecallLabelFontSize",&s.miniMapRecallLabelFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("小地图回城计时字号##miniMapRecallTimerFontSize",&s.miniMapRecallTimerFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("人物框线宽##heroBoxThickness",&s.heroBoxThickness,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("血量环线宽##heroHPBarThickness",&s.heroHPBarThickness,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("大招冷却字号##boxUltimateCdFontSize",&s.boxUltimateCdFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("技能冷却字号##boxSkillCdFontSize",&s.boxSkillCdFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("人物框红色##heroBoxColorR",&s.heroBoxColorR,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("人物框绿色##heroBoxColorG",&s.heroBoxColorG,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("人物框蓝色##heroBoxColorB",&s.heroBoxColorB,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("英雄坐标 X 比例##heroScaleX",&s.heroScaleX,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("英雄坐标 Y 比例##heroScaleY",&s.heroScaleY,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("野怪坐标 X 比例##monsterScaleX",&s.monsterScaleX,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("野怪坐标 Y 比例##monsterScaleY",&s.monsterScaleY,.1f,1.f,"%.2f")||changed;
 if(category_==0)changed=ImGui::Checkbox("野怪##showMonster",&s.showMonster)||changed;
 if(category_==0)changed=ImGui::Checkbox("野怪血量##showMonsterHP",&s.showMonsterHP)||changed;
 if(category_==0)changed=ImGui::Checkbox("野怪计时##showMonsterTimer",&s.showMonsterTimer)||changed;
 if(category_==0)changed=ImGui::Checkbox("小兵##showMinion",&s.showMinion)||changed;
 if(category_==0)changed=ImGui::Checkbox("技能冷却##showSkillCD",&s.showSkillCD)||changed;
 if(category_==0)changed=ImGui::Checkbox("回城提示##showRecall",&s.showRecall)||changed;
 if(category_==1)changed=ImGui::InputFloat("回城提示字号##recallLabelFontSize",&s.recallLabelFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("回城计时字号##recallTimerFontSize",&s.recallTimerFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==0)changed=ImGui::Checkbox("顶部技能面板##showTopSkillPanel",&s.showTopSkillPanel)||changed;
 if(category_==0)changed=ImGui::Checkbox("顶部面板回城提示##showTopSkillPanelRecall",&s.showTopSkillPanelRecall)||changed;
 if(category_==1)changed=ImGui::InputFloat("顶部面板 X##topSkillPanelX",&s.topSkillPanelX,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("顶部面板 Y##topSkillPanelY",&s.topSkillPanelY,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("顶部面板比例##topSkillPanelScale",&s.topSkillPanelScale,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("头像技能间距##topSkillPanelAvatarSkillGap",&s.topSkillPanelAvatarSkillGap,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("顶部大招冷却字号##topPanelUltimateCdFontSize",&s.topPanelUltimateCdFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputFloat("顶部技能冷却字号##topPanelSkillCdFontSize",&s.topPanelSkillCdFontSize,.1f,1.f,"%.2f")||changed;
 if(category_==1)changed=ImGui::InputInt("刷新率##frameRate",&s.frameRate)||changed;
 if(category_==0)changed=ImGui::Checkbox("截图保护##hideAtSnapshot",&s.hideAtSnapshot)||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单内容字号比例##menuRightContentFontScale",&s.menuRightContentFontScale,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单导航字号比例##menuLeftNavFontScale",&s.menuLeftNavFontScale,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单文字红色##menuFontColorR",&s.menuFontColorR,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单文字绿色##menuFontColorG",&s.menuFontColorG,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单文字蓝色##menuFontColorB",&s.menuFontColorB,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单背景红色##menuBackgroundColorR",&s.menuBackgroundColorR,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单背景绿色##menuBackgroundColorG",&s.menuBackgroundColorG,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("菜单背景蓝色##menuBackgroundColorB",&s.menuBackgroundColorB,.1f,1.f,"%.2f")||changed;
 if(category_==2)changed=ImGui::InputFloat("悬浮按钮比例##floatingBallScale",&s.floatingBallScale,.1f,1.f,"%.2f")||changed;
 if(changed){validateSettings(s);}
 return changed;
}
bool SettingsMenu::draw(Settings& settings,Vec2 viewport,ImTextureID floatingImage){
 radius_=90*std::clamp(settings.floatingBallScale,.1f,2.f);
 if(lastViewport_.x<=0){ball_={viewport.x-radius_-18,viewport.y*.35f};}
 else if(lastViewport_.x!=viewport.x||lastViewport_.y!=viewport.y){ball_.x=ball_.x/lastViewport_.x*viewport.x;ball_.y=ball_.y/lastViewport_.y*viewport.y;}
 lastViewport_=viewport;
 ball_.x=std::clamp(ball_.x,radius_+4,std::max(radius_+4,viewport.x-radius_-4));ball_.y=std::clamp(ball_.y,radius_+4,std::max(radius_+4,viewport.y-radius_-4));
 ImGui::SetNextWindowPos({ball_.x-radius_,ball_.y-radius_});ImGui::SetNextWindowSize({radius_*2,radius_*2});ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
 ImGui::Begin("##floating",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoMove);
 ImGui::InvisibleButton("##toggle",{radius_*2,radius_*2});
 if(ImGui::IsItemActive()&&ImGui::IsMouseDragging(0,8)){auto delta=ImGui::GetIO().MouseDelta;ball_.x+=delta.x;ball_.y+=delta.y;}
 if(ImGui::IsItemDeactivated()&&ImGui::GetMouseDragDelta(0,8).x==0&&ImGui::GetMouseDragDelta(0,8).y==0)visible_=!visible_;
 if(floatingImage)ImGui::GetWindowDrawList()->AddImageRounded(floatingImage,{ball_.x-radius_,ball_.y-radius_},{ball_.x+radius_,ball_.y+radius_},{0,0},{1,1},0xffffffff,radius_);
 else ImGui::GetWindowDrawList()->AddCircleFilled({ball_.x,ball_.y},radius_,0xcc221b15,48);
 ImGui::GetWindowDrawList()->AddCircle({ball_.x,ball_.y},radius_-2,0xffa4ff74,48,3);
 ImGui::End();ImGui::PopStyleVar();
 bool changed=false;
 if(visible_){
  ImGui::SetNextWindowSize({std::min(viewport.x-40,1080.f),std::min(viewport.y-40,780.f)},ImGuiCond_FirstUseEver);
  if(ImGui::Begin("TrollSpeed",&visible_,ImGuiWindowFlags_NoSavedSettings)){
   auto position=ImGui::GetWindowPos(),size=ImGui::GetWindowSize();bounds_={position.x,position.y,size.x,size.y};
   const char* categories[]={"功能开关","位置与尺寸","界面"};
   for(int n=0;n<3;n++){if(n)ImGui::SameLine();if(ImGui::Button(categories[n]))category_=n;}
   ImGui::Separator();ImGui::BeginChild("settings");changed=fields(settings);ImGui::EndChild();
  }
  ImGui::End();
 }
 return changed;
}
}
