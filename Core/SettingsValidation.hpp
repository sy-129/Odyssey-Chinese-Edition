#pragma once
#include "Settings.hpp"
#include <algorithm>
#include <cmath>
namespace ts {
inline void validateSettings(Settings& s){const Settings defaults;
 if(!std::isfinite(s.miniMapX))s.miniMapX=defaults.miniMapX;
 if(!std::isfinite(s.miniMapY))s.miniMapY=defaults.miniMapY;
 if(!std::isfinite(s.miniMapSize))s.miniMapSize=defaults.miniMapSize;
 if(!std::isfinite(s.miniMapWidth))s.miniMapWidth=defaults.miniMapWidth;
 if(!std::isfinite(s.miniMapHeight))s.miniMapHeight=defaults.miniMapHeight;
 if(!std::isfinite(s.miniMapAvatarScale))s.miniMapAvatarScale=defaults.miniMapAvatarScale;
 if(!std::isfinite(s.bigMapScale))s.bigMapScale=defaults.bigMapScale;
 if(!std::isfinite(s.minionPointSize))s.minionPointSize=defaults.minionPointSize;
 if(!std::isfinite(s.monsterPointSize))s.monsterPointSize=defaults.monsterPointSize;
 if(!std::isfinite(s.monsterTimerFontSize))s.monsterTimerFontSize=defaults.monsterTimerFontSize;
 if(!std::isfinite(s.miniMapRecallLabelFontSize))s.miniMapRecallLabelFontSize=defaults.miniMapRecallLabelFontSize;
 if(!std::isfinite(s.miniMapRecallTimerFontSize))s.miniMapRecallTimerFontSize=defaults.miniMapRecallTimerFontSize;
 if(!std::isfinite(s.heroBoxThickness))s.heroBoxThickness=defaults.heroBoxThickness;
 if(!std::isfinite(s.heroHPBarThickness))s.heroHPBarThickness=defaults.heroHPBarThickness;
 if(!std::isfinite(s.boxUltimateCdFontSize))s.boxUltimateCdFontSize=defaults.boxUltimateCdFontSize;
 if(!std::isfinite(s.boxSkillCdFontSize))s.boxSkillCdFontSize=defaults.boxSkillCdFontSize;
 if(!std::isfinite(s.heroBoxColorR))s.heroBoxColorR=defaults.heroBoxColorR;
 if(!std::isfinite(s.heroBoxColorG))s.heroBoxColorG=defaults.heroBoxColorG;
 if(!std::isfinite(s.heroBoxColorB))s.heroBoxColorB=defaults.heroBoxColorB;
 if(!std::isfinite(s.heroScaleX))s.heroScaleX=defaults.heroScaleX;
 if(!std::isfinite(s.heroScaleY))s.heroScaleY=defaults.heroScaleY;
 if(!std::isfinite(s.monsterScaleX))s.monsterScaleX=defaults.monsterScaleX;
 if(!std::isfinite(s.monsterScaleY))s.monsterScaleY=defaults.monsterScaleY;
 if(!std::isfinite(s.recallLabelFontSize))s.recallLabelFontSize=defaults.recallLabelFontSize;
 if(!std::isfinite(s.recallTimerFontSize))s.recallTimerFontSize=defaults.recallTimerFontSize;
 if(!std::isfinite(s.topSkillPanelX))s.topSkillPanelX=defaults.topSkillPanelX;
 if(!std::isfinite(s.topSkillPanelY))s.topSkillPanelY=defaults.topSkillPanelY;
 if(!std::isfinite(s.topSkillPanelScale))s.topSkillPanelScale=defaults.topSkillPanelScale;
 if(!std::isfinite(s.topSkillPanelAvatarSkillGap))s.topSkillPanelAvatarSkillGap=defaults.topSkillPanelAvatarSkillGap;
 if(!std::isfinite(s.topPanelUltimateCdFontSize))s.topPanelUltimateCdFontSize=defaults.topPanelUltimateCdFontSize;
 if(!std::isfinite(s.topPanelSkillCdFontSize))s.topPanelSkillCdFontSize=defaults.topPanelSkillCdFontSize;
 if(!std::isfinite(s.menuRightContentFontScale))s.menuRightContentFontScale=defaults.menuRightContentFontScale;
 if(!std::isfinite(s.menuLeftNavFontScale))s.menuLeftNavFontScale=defaults.menuLeftNavFontScale;
 if(!std::isfinite(s.menuFontColorR))s.menuFontColorR=defaults.menuFontColorR;
 if(!std::isfinite(s.menuFontColorG))s.menuFontColorG=defaults.menuFontColorG;
 if(!std::isfinite(s.menuFontColorB))s.menuFontColorB=defaults.menuFontColorB;
 if(!std::isfinite(s.menuBackgroundColorR))s.menuBackgroundColorR=defaults.menuBackgroundColorR;
 if(!std::isfinite(s.menuBackgroundColorG))s.menuBackgroundColorG=defaults.menuBackgroundColorG;
 if(!std::isfinite(s.menuBackgroundColorB))s.menuBackgroundColorB=defaults.menuBackgroundColorB;
 if(!std::isfinite(s.floatingBallScale))s.floatingBallScale=defaults.floatingBallScale;
 s.heroScaleX=std::clamp(s.heroScaleX,.001f,10000.f);
 s.heroScaleY=std::clamp(s.heroScaleY,.001f,10000.f);
 s.monsterScaleX=std::clamp(s.monsterScaleX,.001f,10000.f);
 s.monsterScaleY=std::clamp(s.monsterScaleY,.001f,10000.f);
 s.miniMapWidth=std::clamp(s.miniMapWidth,1.f,10000.f);
 s.miniMapHeight=std::clamp(s.miniMapHeight,1.f,10000.f);
 s.bigMapScale=std::clamp(s.bigMapScale,.1f,10.f);
 s.miniMapAvatarScale=std::clamp(s.miniMapAvatarScale,.1f,10.f);
 s.floatingBallScale=std::clamp(s.floatingBallScale,.1f,2.f);
 s.frameRate=std::clamp(s.frameRate,30,120);
}
}
