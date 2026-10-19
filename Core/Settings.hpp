#pragma once
#include <cstdint>
namespace ts {
// Defaults recovered from SmobaMenuConfig::init memory writes, not invented.
struct Settings {
 bool enableESP = true; // 0x100030b0c, original field +0x10
 bool showHeroBox = true; // 0x100030b1c, original field +0x11
 bool showHeroHP = true; // 0x100030b2c, original field +0x12
 bool showHeroID = true; // 0x100030b3c, original field +0x13
 bool showMiniMap = true; // 0x100030b4c, original field +0x14
 bool showAvatar = true; // 0x100030b5c, original field +0x15
 bool showBoxAvatar = true; // 0x100030b6c, original field +0x16
 bool showMiniMapFrame = true; // 0x100030b7c, original field +0x17
 float miniMapX = 117.0f; // 0x100030b8c, original field +0x24
 float miniMapY = 12.0f; // 0x100030b9c, original field +0x28
 float miniMapSize = 390.0f; // 0x100030bac, original field +0x2c
 float miniMapWidth = 387.0f; // 0x100030bbc, original field +0x30
 float miniMapHeight = 392.0f; // 0x100030bcc, original field +0x34
 float miniMapAvatarScale = 2.3499999046325684f; // 0x100030bdc, original field +0x38
 float bigMapScale = 2.0f; // 0x100030bec, original field +0x3c
 float minionPointSize = 5.599999904632568f; // 0x100030bfc, original field +0x40
 float monsterPointSize = 6.300000190734863f; // 0x100030c0c, original field +0x44
 float monsterTimerFontSize = 28.299999237060547f; // 0x100030c1c, original field +0x48
 float miniMapRecallLabelFontSize = 54.599998474121094f; // 0x100030c2c, original field +0x4c
 float miniMapRecallTimerFontSize = 54.599998474121094f; // 0x100030c3c, original field +0x50
 float heroBoxThickness = 6.599999904632568f; // 0x100030c4c, original field +0x54
 float heroHPBarThickness = 5.800000190734863f; // 0x100030c5c, original field +0x58
 float boxUltimateCdFontSize = 25.700000762939453f; // 0x100030c6c, original field +0x5c
 float boxSkillCdFontSize = 33.79999923706055f; // 0x100030c7c, original field +0x60
 float heroBoxColorG = 255.0f; // 0x100030c9c, original field +0x68
 float heroBoxColorB = 255.0f; // 0x100030cac, original field +0x6c
 float heroScaleX = 101.80000305175781f; // 0x100030cbc, original field +0x70
 float heroScaleY = 101.80000305175781f; // 0x100030ccc, original field +0x74
 float monsterScaleX = 101.80000305175781f; // 0x100030cdc, original field +0x78
 float monsterScaleY = 101.80000305175781f; // 0x100030cec, original field +0x7c
 bool showMonster = true; // 0x100030cfc, original field +0x18
 bool showMonsterHP = false; // 0x100030d0c, original field +0x19
 bool showMonsterTimer = true; // 0x100030d1c, original field +0x1a
 bool showMinion = true; // 0x100030d2c, original field +0x1b
 bool showSkillCD = true; // 0x100030d3c, original field +0x1c
 bool showRecall = true; // 0x100030d4c, original field +0x1d
 float recallLabelFontSize = 54.599998474121094f; // 0x100030d5c, original field +0x80
 float recallTimerFontSize = 54.599998474121094f; // 0x100030d6c, original field +0x84
 bool showTopSkillPanel = true; // 0x100030d7c, original field +0x1e
 bool showTopSkillPanelRecall = true; // 0x100030d8c, original field +0x1f
 float topSkillPanelX = 1106.0f; // 0x100030d9c, original field +0x88
 float topSkillPanelY = -107.0f; // 0x100030dac, original field +0x8c
 float topSkillPanelScale = 3.6600000858306885f; // 0x100030dbc, original field +0x90
 float topSkillPanelAvatarSkillGap = 10.600000381469727f; // 0x100030dcc, original field +0x94
 float topPanelUltimateCdFontSize = 50.29999923706055f; // 0x100030ddc, original field +0x98
 float topPanelSkillCdFontSize = 43.400001525878906f; // 0x100030dec, original field +0x9c
 int frameRate = 120; // 0x100030dfc, original field +0xa0
 bool hideAtSnapshot = true; // 0x100030e0c, original field +0x20
 float menuRightContentFontScale = 1.0199999809265137f; // 0x100030e1c, original field +0xa4
 float menuLeftNavFontScale = 0.9200000166893005f; // 0x100030e2c, original field +0xa8
 float menuFontColorR = 116.0f; // 0x100030e3c, original field +0xac
 float menuFontColorG = 255.0f; // 0x100030e4c, original field +0xb0
 float menuFontColorB = 164.0f; // 0x100030e5c, original field +0xb4
 float menuBackgroundColorR = 16.0f; // 0x100030e6c, original field +0xb8
 float menuBackgroundColorG = 21.0f; // 0x100030e7c, original field +0xbc
 float menuBackgroundColorB = 27.0f; // 0x100030e8c, original field +0xc0
 float floatingBallScale = 1.0f; // 0x100030e9c, original field +0xc4
};
}
