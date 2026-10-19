#include "SettingsStore.hpp"
namespace ts {
static NSString* const path=@"/var/mobile/Library/Preferences/local.trollspeed.rebuild.settings.plist";
NSDictionary<NSString*,NSNumber*>* settingsDictionary(const Settings& s){return @{
@"enableESP":@(s.enableESP),
@"showHeroBox":@(s.showHeroBox),
@"showHeroHP":@(s.showHeroHP),
@"showHeroID":@(s.showHeroID),
@"showMiniMap":@(s.showMiniMap),
@"showAvatar":@(s.showAvatar),
@"showBoxAvatar":@(s.showBoxAvatar),
@"showMiniMapFrame":@(s.showMiniMapFrame),
@"miniMapX":@(s.miniMapX),
@"miniMapY":@(s.miniMapY),
@"miniMapSize":@(s.miniMapSize),
@"miniMapWidth":@(s.miniMapWidth),
@"miniMapHeight":@(s.miniMapHeight),
@"miniMapAvatarScale":@(s.miniMapAvatarScale),
@"bigMapScale":@(s.bigMapScale),
@"minionPointSize":@(s.minionPointSize),
@"monsterPointSize":@(s.monsterPointSize),
@"monsterTimerFontSize":@(s.monsterTimerFontSize),
@"miniMapRecallLabelFontSize":@(s.miniMapRecallLabelFontSize),
@"miniMapRecallTimerFontSize":@(s.miniMapRecallTimerFontSize),
@"heroBoxThickness":@(s.heroBoxThickness),
@"heroHPBarThickness":@(s.heroHPBarThickness),
@"boxUltimateCdFontSize":@(s.boxUltimateCdFontSize),
@"boxSkillCdFontSize":@(s.boxSkillCdFontSize),
@"heroBoxColorG":@(s.heroBoxColorG),
@"heroBoxColorB":@(s.heroBoxColorB),
@"heroScaleX":@(s.heroScaleX),
@"heroScaleY":@(s.heroScaleY),
@"monsterScaleX":@(s.monsterScaleX),
@"monsterScaleY":@(s.monsterScaleY),
@"showMonster":@(s.showMonster),
@"showMonsterHP":@(s.showMonsterHP),
@"showMonsterTimer":@(s.showMonsterTimer),
@"showMinion":@(s.showMinion),
@"showSkillCD":@(s.showSkillCD),
@"showRecall":@(s.showRecall),
@"recallLabelFontSize":@(s.recallLabelFontSize),
@"recallTimerFontSize":@(s.recallTimerFontSize),
@"showTopSkillPanel":@(s.showTopSkillPanel),
@"showTopSkillPanelRecall":@(s.showTopSkillPanelRecall),
@"topSkillPanelX":@(s.topSkillPanelX),
@"topSkillPanelY":@(s.topSkillPanelY),
@"topSkillPanelScale":@(s.topSkillPanelScale),
@"topSkillPanelAvatarSkillGap":@(s.topSkillPanelAvatarSkillGap),
@"topPanelUltimateCdFontSize":@(s.topPanelUltimateCdFontSize),
@"topPanelSkillCdFontSize":@(s.topPanelSkillCdFontSize),
@"frameRate":@(s.frameRate),
@"hideAtSnapshot":@(s.hideAtSnapshot),
@"menuRightContentFontScale":@(s.menuRightContentFontScale),
@"menuLeftNavFontScale":@(s.menuLeftNavFontScale),
@"menuFontColorR":@(s.menuFontColorR),
@"menuFontColorG":@(s.menuFontColorG),
@"menuFontColorB":@(s.menuFontColorB),
@"menuBackgroundColorR":@(s.menuBackgroundColorR),
@"menuBackgroundColorG":@(s.menuBackgroundColorG),
@"menuBackgroundColorB":@(s.menuBackgroundColorB),
@"floatingBallScale":@(s.floatingBallScale)};}
Settings settingsFromDictionary(NSDictionary<NSString*,NSNumber*>* d){Settings s;
 if([d[@"enableESP"] isKindOfClass:NSNumber.class])s.enableESP=[d[@"enableESP"] boolValue];
 if([d[@"showHeroBox"] isKindOfClass:NSNumber.class])s.showHeroBox=[d[@"showHeroBox"] boolValue];
 if([d[@"showHeroHP"] isKindOfClass:NSNumber.class])s.showHeroHP=[d[@"showHeroHP"] boolValue];
 if([d[@"showHeroID"] isKindOfClass:NSNumber.class])s.showHeroID=[d[@"showHeroID"] boolValue];
 if([d[@"showMiniMap"] isKindOfClass:NSNumber.class])s.showMiniMap=[d[@"showMiniMap"] boolValue];
 if([d[@"showAvatar"] isKindOfClass:NSNumber.class])s.showAvatar=[d[@"showAvatar"] boolValue];
 if([d[@"showBoxAvatar"] isKindOfClass:NSNumber.class])s.showBoxAvatar=[d[@"showBoxAvatar"] boolValue];
 if([d[@"showMiniMapFrame"] isKindOfClass:NSNumber.class])s.showMiniMapFrame=[d[@"showMiniMapFrame"] boolValue];
 if([d[@"miniMapX"] isKindOfClass:NSNumber.class])s.miniMapX=[d[@"miniMapX"] floatValue];
 if([d[@"miniMapY"] isKindOfClass:NSNumber.class])s.miniMapY=[d[@"miniMapY"] floatValue];
 if([d[@"miniMapSize"] isKindOfClass:NSNumber.class])s.miniMapSize=[d[@"miniMapSize"] floatValue];
 if([d[@"miniMapWidth"] isKindOfClass:NSNumber.class])s.miniMapWidth=[d[@"miniMapWidth"] floatValue];
 if([d[@"miniMapHeight"] isKindOfClass:NSNumber.class])s.miniMapHeight=[d[@"miniMapHeight"] floatValue];
 if([d[@"miniMapAvatarScale"] isKindOfClass:NSNumber.class])s.miniMapAvatarScale=[d[@"miniMapAvatarScale"] floatValue];
 if([d[@"bigMapScale"] isKindOfClass:NSNumber.class])s.bigMapScale=[d[@"bigMapScale"] floatValue];
 if([d[@"minionPointSize"] isKindOfClass:NSNumber.class])s.minionPointSize=[d[@"minionPointSize"] floatValue];
 if([d[@"monsterPointSize"] isKindOfClass:NSNumber.class])s.monsterPointSize=[d[@"monsterPointSize"] floatValue];
 if([d[@"monsterTimerFontSize"] isKindOfClass:NSNumber.class])s.monsterTimerFontSize=[d[@"monsterTimerFontSize"] floatValue];
 if([d[@"miniMapRecallLabelFontSize"] isKindOfClass:NSNumber.class])s.miniMapRecallLabelFontSize=[d[@"miniMapRecallLabelFontSize"] floatValue];
 if([d[@"miniMapRecallTimerFontSize"] isKindOfClass:NSNumber.class])s.miniMapRecallTimerFontSize=[d[@"miniMapRecallTimerFontSize"] floatValue];
 if([d[@"heroBoxThickness"] isKindOfClass:NSNumber.class])s.heroBoxThickness=[d[@"heroBoxThickness"] floatValue];
 if([d[@"heroHPBarThickness"] isKindOfClass:NSNumber.class])s.heroHPBarThickness=[d[@"heroHPBarThickness"] floatValue];
 if([d[@"boxUltimateCdFontSize"] isKindOfClass:NSNumber.class])s.boxUltimateCdFontSize=[d[@"boxUltimateCdFontSize"] floatValue];
 if([d[@"boxSkillCdFontSize"] isKindOfClass:NSNumber.class])s.boxSkillCdFontSize=[d[@"boxSkillCdFontSize"] floatValue];
 if([d[@"heroBoxColorG"] isKindOfClass:NSNumber.class])s.heroBoxColorG=[d[@"heroBoxColorG"] floatValue];
 if([d[@"heroBoxColorB"] isKindOfClass:NSNumber.class])s.heroBoxColorB=[d[@"heroBoxColorB"] floatValue];
 if([d[@"heroScaleX"] isKindOfClass:NSNumber.class])s.heroScaleX=[d[@"heroScaleX"] floatValue];
 if([d[@"heroScaleY"] isKindOfClass:NSNumber.class])s.heroScaleY=[d[@"heroScaleY"] floatValue];
 if([d[@"monsterScaleX"] isKindOfClass:NSNumber.class])s.monsterScaleX=[d[@"monsterScaleX"] floatValue];
 if([d[@"monsterScaleY"] isKindOfClass:NSNumber.class])s.monsterScaleY=[d[@"monsterScaleY"] floatValue];
 if([d[@"showMonster"] isKindOfClass:NSNumber.class])s.showMonster=[d[@"showMonster"] boolValue];
 if([d[@"showMonsterHP"] isKindOfClass:NSNumber.class])s.showMonsterHP=[d[@"showMonsterHP"] boolValue];
 if([d[@"showMonsterTimer"] isKindOfClass:NSNumber.class])s.showMonsterTimer=[d[@"showMonsterTimer"] boolValue];
 if([d[@"showMinion"] isKindOfClass:NSNumber.class])s.showMinion=[d[@"showMinion"] boolValue];
 if([d[@"showSkillCD"] isKindOfClass:NSNumber.class])s.showSkillCD=[d[@"showSkillCD"] boolValue];
 if([d[@"showRecall"] isKindOfClass:NSNumber.class])s.showRecall=[d[@"showRecall"] boolValue];
 if([d[@"recallLabelFontSize"] isKindOfClass:NSNumber.class])s.recallLabelFontSize=[d[@"recallLabelFontSize"] floatValue];
 if([d[@"recallTimerFontSize"] isKindOfClass:NSNumber.class])s.recallTimerFontSize=[d[@"recallTimerFontSize"] floatValue];
 if([d[@"showTopSkillPanel"] isKindOfClass:NSNumber.class])s.showTopSkillPanel=[d[@"showTopSkillPanel"] boolValue];
 if([d[@"showTopSkillPanelRecall"] isKindOfClass:NSNumber.class])s.showTopSkillPanelRecall=[d[@"showTopSkillPanelRecall"] boolValue];
 if([d[@"topSkillPanelX"] isKindOfClass:NSNumber.class])s.topSkillPanelX=[d[@"topSkillPanelX"] floatValue];
 if([d[@"topSkillPanelY"] isKindOfClass:NSNumber.class])s.topSkillPanelY=[d[@"topSkillPanelY"] floatValue];
 if([d[@"topSkillPanelScale"] isKindOfClass:NSNumber.class])s.topSkillPanelScale=[d[@"topSkillPanelScale"] floatValue];
 if([d[@"topSkillPanelAvatarSkillGap"] isKindOfClass:NSNumber.class])s.topSkillPanelAvatarSkillGap=[d[@"topSkillPanelAvatarSkillGap"] floatValue];
 if([d[@"topPanelUltimateCdFontSize"] isKindOfClass:NSNumber.class])s.topPanelUltimateCdFontSize=[d[@"topPanelUltimateCdFontSize"] floatValue];
 if([d[@"topPanelSkillCdFontSize"] isKindOfClass:NSNumber.class])s.topPanelSkillCdFontSize=[d[@"topPanelSkillCdFontSize"] floatValue];
 if([d[@"frameRate"] isKindOfClass:NSNumber.class])s.frameRate=[d[@"frameRate"] intValue];
 if([d[@"hideAtSnapshot"] isKindOfClass:NSNumber.class])s.hideAtSnapshot=[d[@"hideAtSnapshot"] boolValue];
 if([d[@"menuRightContentFontScale"] isKindOfClass:NSNumber.class])s.menuRightContentFontScale=[d[@"menuRightContentFontScale"] floatValue];
 if([d[@"menuLeftNavFontScale"] isKindOfClass:NSNumber.class])s.menuLeftNavFontScale=[d[@"menuLeftNavFontScale"] floatValue];
 if([d[@"menuFontColorR"] isKindOfClass:NSNumber.class])s.menuFontColorR=[d[@"menuFontColorR"] floatValue];
 if([d[@"menuFontColorG"] isKindOfClass:NSNumber.class])s.menuFontColorG=[d[@"menuFontColorG"] floatValue];
 if([d[@"menuFontColorB"] isKindOfClass:NSNumber.class])s.menuFontColorB=[d[@"menuFontColorB"] floatValue];
 if([d[@"menuBackgroundColorR"] isKindOfClass:NSNumber.class])s.menuBackgroundColorR=[d[@"menuBackgroundColorR"] floatValue];
 if([d[@"menuBackgroundColorG"] isKindOfClass:NSNumber.class])s.menuBackgroundColorG=[d[@"menuBackgroundColorG"] floatValue];
 if([d[@"menuBackgroundColorB"] isKindOfClass:NSNumber.class])s.menuBackgroundColorB=[d[@"menuBackgroundColorB"] floatValue];
 if([d[@"floatingBallScale"] isKindOfClass:NSNumber.class])s.floatingBallScale=[d[@"floatingBallScale"] floatValue];
 return s;
}
Settings loadSettings(){return settingsFromDictionary([NSDictionary dictionaryWithContentsOfFile:path]?:@{});}
void saveSettings(const Settings& s){[settingsDictionary(s) writeToFile:path atomically:YES];}
}
