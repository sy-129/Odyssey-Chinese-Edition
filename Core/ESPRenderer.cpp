#include "ESPRenderer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace ts {
namespace {
constexpr float pi=3.14159265358979323846f;
ImVec2 v(Vec2 p){return {p.x,p.y};}
float bounded(float x,float lo,float hi){return std::isfinite(x)?std::clamp(x,lo,hi):lo;}
bool timedMonster(int id){
 if(id==0x1aab||id==0x401b||id==0x28105||id==0x289724||id==0x2898bd||id==0x28972e)return true;
 const int bit=id-0x40f18;return bit>=0&&bit<54&&((0x307c000000802dULL>>bit)&1);
}
}
void ESPRenderer::text(ImDrawList* d,Vec2 p,float size,const char* value,ImU32 color){
 auto* font=ImGui::GetFont();auto extent=font->CalcTextSizeA(size,1e9f,0,value);p.x-=extent.x*.5f;
 for(auto offset:{Vec2{1,1},Vec2{-1,-1},Vec2{1,-1},Vec2{-1,1}})d->AddText(font,size,{p.x+offset.x,p.y+offset.y},0xff000000,value);
 d->AddText(font,size,v(p),color,value);
}
void ESPRenderer::recallBorder(ImDrawList* d,Vec2 a,Vec2 b,float stroke,float radius){
 stroke=std::max(stroke,1.f);float w=b.x-a.x,h=b.y-a.y;if(w<1||h<1)return;
 float corner=std::max(std::min(w,h)*.28f,4.f);
 d->AddRectFilled(v(a),v(b),0x42120c08,radius);
 d->AddRectFilled(v(a),{b.x,a.y+std::max(h*.14f,2.f)},0x3a48b8ff,radius);
 d->AddRect(v(a),v(b),0x3448b8ff,radius,0,stroke+4);
 d->AddRect(v(a),v(b),0x7648b8ff,radius,0,stroke+2);
 d->AddRect(v(a),v(b),0xe048b8ff,radius,0,stroke);
 for(auto p:{a,Vec2{b.x,a.y},Vec2{a.x,b.y},b}){
  d->AddLine(v(p),{p.x+(p.x==a.x?corner:-corner),p.y},0xff48b8ff,stroke);
  d->AddLine(v(p),{p.x,p.y+(p.y==a.y?corner:-corner)},0xff48b8ff,stroke);
 }
}
void ESPRenderer::timer(ImDrawList* d,Vec2 a,Vec2 b,float seconds,float font){
 char value[24];std::snprintf(value,sizeof value,"%.1f",seconds);
 auto extent=ImGui::GetFont()->CalcTextSizeA(font,1e9f,0,value);
 float width=std::min(extent.x+2*std::max(font*.4f,4.f),b.x-a.x-2),height=extent.y+2*std::max(font*.18f,1.5f);
 if(width<=0)return;
 Vec2 left{(a.x+b.x-width)*.5f,b.y-height-std::max(font*.08f,1.5f)},right{left.x+width,left.y+height};
 d->AddRectFilled(v(left),v(right),0xe0120e0a,height*.48f);
 d->AddRectFilled(v(left),{right.x,left.y+height*.45f},0x4a48b8ff,height*.48f);
 d->AddRect(v(left),v(right),0xe648b8ff,height*.48f,0,1.2f);
 text(d,{(a.x+b.x)*.5f,left.y+(height-font)*.5f-font*.06f},font,value,0xffffffff);
}
void ESPRenderer::icon(ImDrawList* d,ImTextureID texture,Vec2 a,float side,int cd,float font,bool ready){
 Vec2 b{a.x+side,a.y+side};
 if(texture)d->AddImage(texture,v(a),v(b));else d->AddRectFilled(v(a),v(b),0xff275f27);
 if(cd>0||!ready)d->AddRectFilled(v(a),v(b),0xa9000000);
 if(cd>0){char value[24];std::snprintf(value,sizeof value,"%d",cd);text(d,{a.x+side*.5f,a.y+(side-font)*.5f},font,value,0xffffffff);}
}
void ESPRenderer::draw(ImDrawList* d,const Frame& frame,const Settings& s,Vec2 viewport){
 if(!d||viewport.x<=0||viewport.y<=0)return;
 MapRect map{s.miniMapX,s.miniMapY,s.miniMapWidth,s.miniMapHeight};
 if(s.showMiniMapFrame)d->AddRect({map.x,map.y},{map.x+map.width,map.y+map.height},0xff00ff00);
 if(s.showMinion)for(auto world:frame.minions){auto p=toMiniMap(map,world,frame.matrix,s.heroScaleX,s.heroScaleY);d->AddCircleFilled(v(p),bounded(s.minionPointSize,.5f,8.f),0xff00ffff,20);}
 for(const auto& h:frame.heroes){
  if(h.dead)continue;
  auto portrait=textures_.hero(h.id);
  if(s.enableESP&&s.showAvatar){
   auto p=toMiniMap(map,h.position,frame.matrix,s.heroScaleX,s.heroScaleY);float side=20*s.miniMapAvatarScale;
   if(portrait)d->AddImage(portrait,{p.x-side*.5f,p.y-side*.5f},{p.x+side*.5f,p.y+side*.5f});
   float ratio=bounded(h.healthFraction,0,1),lost=1-ratio;
   ImU32 color=0xff0000ff|(ImU32(lost*145)<<8)|(ImU32(lost*190)<<16);
   d->PathArcTo(v(p),side*.5f+3,-pi*.5f,ratio*2*pi-pi*.5f,64);d->PathStroke(color,0,bounded(s.heroHPBarThickness,1,8));
   if(s.showRecall&&h.recalling){
    char seconds[24];std::snprintf(seconds,sizeof seconds,"%.1f",h.recallSeconds);
    float font=bounded(s.miniMapRecallTimerFontSize,6,78);
    auto extent=ImGui::GetFont()->CalcTextSizeA(font,1e9f,0,seconds);
    float px=std::max(font*.42f,4.f),py=std::max(font*.22f,2.f),centerY=p.y-side*.86f;
    Vec2 a{p.x-extent.x*.5f-px,centerY-extent.y*.5f-py},b{p.x+extent.x*.5f+px,centerY+extent.y*.5f+py};
    float radius=(b.y-a.y)*.48f;
    d->AddRectFilled(v(a),v(b),0xe4120e0a,radius);
    d->AddRectFilled(v(a),{b.x,a.y+(b.y-a.y)*.44f},0x5648b8ff,radius);
    d->AddRect(v(a),v(b),0xf048b8ff,radius,0,1.2f);
    text(d,{p.x,a.y+((b.y-a.y)-font)*.5f-font*.06f},font,seconds,0xffffffff);
    float label=bounded(s.miniMapRecallLabelFontSize,6,78);
    text(d,{p.x,p.y+side*.63f-label*.5f},label,"回城",0xff48b8ff);
   }
  }
  Vec2 screen;if(!toScreen(viewport,h.position,frame.matrix,screen))continue;
  float scale=s.bigMapScale;Vec2 a{screen.x-32.5f*scale,screen.y-75*scale},b{screen.x+32.5f*scale,screen.y+25*scale};
  if(s.showHeroBox){float length=12*scale,stroke=bounded(s.heroBoxThickness,1,8);
   for(auto p:{a,Vec2{b.x,a.y},Vec2{a.x,b.y},b}){
    d->AddLine(v(p),{p.x+(p.x==a.x?length:-length),p.y},0xffffffff,stroke);
    d->AddLine(v(p),{p.x,p.y+(p.y==a.y?length:-length)},0xffffffff,stroke);
   }
   if(s.showRecall&&h.recalling){
    float side=bounded(std::min({65*scale*.62f,100*scale*.42f,s.recallLabelFontSize}),8,60);
    Vec2 ra{screen.x-side*.5f,(a.y+b.y-side)*.5f},rb{ra.x+side,ra.y+side};
    auto recall=textures_.skill(99999);if(recall&&!textures_.isFallback(recall))d->AddImage(recall,v(ra),v(rb));else text(d,{screen.x,ra.y},side,"回城",0xff48b8ff);
    recallBorder(d,ra,rb,1.5f,4);timer(d,a,b,h.recallSeconds,bounded(s.recallTimerFontSize,6,78));
   }
  }
  float side=30*scale;
  if(s.showBoxAvatar){Vec2 p{screen.x-side*.5f,a.y-side+3*scale};if(portrait)d->AddImage(portrait,v(p),{p.x+side,p.y+side});d->AddRect(v(p),{p.x+side,p.y+side},0xebffffff,std::max(2.f,4*scale),0,std::max(1.f,scale));}
  if(s.showBoxAvatar||s.showSkillCD){
   int count=s.showSkillCD?4:3;float gap=7*scale,y=(a.y+b.y-count*side-(count-1)*gap)*.5f;
   if(s.showSkillCD){icon(d,textures_.skill(h.summonerID,h.alternateSummonerID),{b.x+9*scale,y},side,h.summonerCD,std::max(8.f,bounded(s.boxSkillCdFontSize,6,52)*scale*.92f),true);y+=side+gap;}
   int cds[]={h.skill0CD,h.skill1CD,h.ultimateCD};
   for(int i=0;i<3;i++,y+=side+gap)icon(d,textures_.skill(h.id*100+(i+1)*10),{b.x+9*scale,y},side,cds[i],std::max(8.f,bounded(i==2?s.boxUltimateCdFontSize:s.boxSkillCdFontSize,6,52)*scale*.92f),h.ready[i]);
  }
 }
 if(s.showMonster)for(const auto& monster:frame.monsters){
  if(monster.remaining<0)continue;
  auto p=toMiniMap(map,monster.position,frame.matrix,s.monsterScaleX,s.monsterScaleY);
  if(monster.remaining==monster.total)d->AddCircleFilled(v(p),bounded(s.monsterPointSize,.5f,8),0xffffffff,20);
  else if(timedMonster(monster.id)){char value[24];std::snprintf(value,sizeof value,"%d",monster.remaining);float font=bounded(s.monsterTimerFontSize,6,30);text(d,{p.x,p.y-font},font,value,0xffffffff);}
 }
 if(s.showTopSkillPanel){
  float scale=bounded(s.topSkillPanelScale,1,6),side=27*scale,tiny=(side-3*scale)*.5f,gap=std::max(scale*3,bounded(s.topSkillPanelAvatarSkillGap,0,40)*scale*.5f),rgap=std::max(gap*.55f,2*scale);
  bool recall=false;for(const auto& h:frame.heroes)recall|=!h.dead&&h.recalling;
  recall=recall&&s.showRecall&&s.showTopSkillPanelRecall;
  float width=side*5+12*scale*4+8*scale,height=side+gap+tiny+8*scale+(recall?tiny+rgap:0);
  float x=bounded(s.topSkillPanelX,6,std::max(6.f,viewport.x-width-6)),y=bounded(s.topSkillPanelY,6,std::max(6.f,viewport.y-height-6));
  int index=0;for(const auto& h:frame.heroes){if(h.dead)continue;if(index==5)break;
   float left=x+index*(side+12*scale),top=y;
   icon(d,textures_.hero(h.id),{left,top},side,0,0,true);
   d->AddRect({left,top},{left+side,top+side},0xebaad6e8,std::max(2.f,5*scale),0,std::max(1.2f,1.1f*scale));
   float font=std::max(7.f,std::min(bounded(s.topPanelSkillCdFontSize,6,52)*scale,tiny*.6f));
   icon(d,textures_.skill(h.summonerID,h.alternateSummonerID),{left,top+side+gap},tiny,h.summonerCD,font,true);
   float rounding=std::max(2.f,4*scale);d->AddRect({left,top+side+gap},{left+tiny,top+side+gap+tiny},0xe1ffffff,rounding,0,scale);
   Vec2 ultimate{left+tiny+3*scale,top+side+gap},ultimateEnd{ultimate.x+tiny,ultimate.y+tiny};
   d->AddRectFilled(v(ultimate),v(ultimateEnd),h.ultimateCD>0?0xeb161210:0xeb6ed240,rounding);
   d->AddRect(v(ultimate),v(ultimateEnd),h.ultimateCD>0?0xe1ffffff:0xdce0ffd2,rounding,0,scale);
   if(h.ultimateCD>0){char value[24];std::snprintf(value,sizeof value,"%d",h.ultimateCD);float font=std::max(7.f,std::min(bounded(s.topPanelUltimateCdFontSize,6,52)*scale,tiny*.6f));text(d,{ultimate.x+tiny*.5f,ultimate.y+(tiny-font)*.5f},font,value,0xffffffff);}
   if(recall&&h.recalling){Vec2 a{left,top+side+gap+tiny+rgap},b{left+tiny,a.y+tiny};
    auto image=textures_.skill(99999);if(image&&!textures_.isFallback(image))d->AddImage(image,v(a),v(b));else{d->AddRectFilled(v(a),v(b),0xdc1e1814,3*scale);text(d,{left+tiny*.5f,a.y+tiny*.18f},std::max(6.f,tiny*.4f),"回城",0xff48b8ff);}
    recallBorder(d,a,b,scale,3*scale);timer(d,a,b,h.recallSeconds,std::max(6.f,bounded(s.topPanelSkillCdFontSize,6,52)*scale*.6f));}
   ++index;
  }
  if(index==0)text(d,{x+width*.5f,y+4*scale},16*scale,"无英雄数据",0xffffffff);
 }
}
}
