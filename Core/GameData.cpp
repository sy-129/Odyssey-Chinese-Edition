#include "GameData.hpp"
#include <algorithm>
#include <set>
namespace ts {
namespace {bool valid(std::uint64_t p){return p>=0x100000000ULL&&p<0x8000000000ULL;}}
std::uint64_t GameReader::at(std::uint64_t b,std::uint64_t o)const{auto p=add(b,o);return p&&valid(*p)?*p:0;}
std::uint64_t GameReader::pointer(std::uint64_t p)const{if(!valid(p))return 0;auto v=memory_.value<std::uint64_t>(p);return v&&valid(*v)?*v:0;}
int GameReader::integer(std::uint64_t p)const{if(!valid(p))return 0;return memory_.value<std::int32_t>(p).value_or(0);}
Vec2 GameReader::rawPosition(std::uint64_t p)const{
 if(!valid(p)){return {};}
 return decodePosition(integer(at(p,offsets_.posX)),integer(at(p,offsets_.posY))).value_or(Vec2{});
}
Vec2 GameReader::entityPosition(std::uint64_t entity)const{
 auto p=pointer(at(entity,offsets_.posComponent));p=pointer(at(p,offsets_.posPrimary));return rawPosition(p);
}
Vec2 GameReader::heroPosition(std::uint64_t entity)const{
 auto p=pointer(at(entity,offsets_.coordBase));p=pointer(at(p,offsets_.coordHop2));p=pointer(at(p,offsets_.coordHop3));p=pointer(at(p,offsets_.coordHop4));return rawPosition(p);
}
Vec2 GameReader::minionPosition(std::uint64_t entity)const{
 Vec2 p=entityPosition(entity);if(p.x!=0||p.y!=0)return p;
 auto b=pointer(at(entity,offsets_.minionCoord));b=pointer(at(b,offsets_.coordHop4));return rawPosition(b);
}
int GameReader::cooldown(std::uint64_t entity,std::uint64_t slot)const{
 if(!slot){return 0;}
 auto p=pointer(at(entity,offsets_.skillList));p=pointer(at(p,slot));p=pointer(at(p,offsets_.skillCdPtr));return integer(at(p,offsets_.skillCdValue))/8192000;
}
bool GameReader::skillReady(std::uint64_t p)const{return valid(p)&&integer(at(p,offsets_.skillValidField))!=0&&integer(at(p,offsets_.skillReadyField))==1;}
std::array<int,2> GameReader::talents(std::uint64_t entity)const{
 std::array<int,2> result{};auto p=pointer(at(entity,offsets_.skillList));p=pointer(at(p,offsets_.skill4));
 for(auto slot:{offsets_.talentSlot0,offsets_.talentSlot1,offsets_.talentSlot2,offsets_.talentSlot3,offsets_.talentSlot4}){
  if(!slot){continue;}
  auto skill=pointer(at(p,slot));int id=integer(at(skill,offsets_.skillCfgId));if(id<80100||id>=80200)continue;
  if(!result[0])result[0]=id;else if(result[0]!=id){result[1]=id;break;}
 }return result;
}
bool GameReader::recalling(std::uint64_t entity)const{
 auto p=pointer(at(entity,offsets_.skillList));p=pointer(at(p,offsets_.recallRoot));p=pointer(at(p,offsets_.recallState));return integer(at(p,offsets_.recallFlag))!=0;
}
void GameReader::reset(){smoother_.clear();skills_.clear();recalls_.clear();monsters_.clear();monstersUpdated_=0;}
std::optional<Frame> GameReader::capture(double now,bool includeMinions){
 if(pid_<1||!valid(module_))return {};
 auto p=pointer(at(module_,offsets_.matrix));for(auto off:{offsets_.matrixHop1,offsets_.matrixHop2,offsets_.matrixHop3})p=pointer(at(p,off));
 auto matrixValue=matrix(memory_,at(p,offsets_.matrixField));if(!matrixValue||((*matrixValue)[0]==0&&(*matrixValue)[10]==0)){reset();return {};}
 auto root=pointer(at(module_,offsets_.gameStruct));auto entityRoot=pointer(at(root,offsets_.entityRoot));auto actors=pointer(at(entityRoot,offsets_.actorArray));if(!actors)return {};
 Frame frame;frame.matrix=*matrixValue;frame.timestamp=now;int ownTeam=frame.matrix[0]>0?1:2;
 for(auto i=skills_.begin();i!=skills_.end();)if(now-i->second.updated>3)i=skills_.erase(i);else ++i;
 for(auto i=recalls_.begin();i!=recalls_.end();)if(now-i->second.lastSeen>3)i=recalls_.erase(i);else ++i;
 for(unsigned i=0;i<128;i++){
  auto actor=pointer(at(actors,offsets_.entityStride*i));if(!actor)continue;int team=integer(at(actor,offsets_.team));if((team!=1&&team!=2)||team==ownTeam)continue;
  auto pos=heroPosition(actor);if(pos.x==0&&pos.y==0)continue;auto hpBase=pointer(at(actor,offsets_.hpBase));Hero h;h.identity=actor;h.team=team;
  h.id=integer(at(actor,offsets_.heroId));h.hp=integer(at(hpBase,offsets_.hp));h.maxHP=integer(at(hpBase,offsets_.maxHp));if(h.maxHP<=0)continue;
  h.healthFraction=static_cast<float>(h.hp)/static_cast<float>(h.maxHP);h.dead=h.hp==0;h.position=smoother_.sample(actor,pos);
  auto talent=talents(actor);h.summonerID=talent[0]?talent[0]:talent[1];h.alternateSummonerID=talent[1];
  auto& cache=skills_[actor];if(cache.updated==0||now-cache.updated>=.15){
   cache.ultimate=cooldown(actor,offsets_.skill3);cache.summoner=cooldown(actor,offsets_.skill4);cache.skill0=cooldown(actor,offsets_.skill0);cache.skill1=cooldown(actor,offsets_.skill1);
   auto skills=pointer(at(actor,offsets_.skillList));std::array<std::uint64_t,4> slots{offsets_.skill0,offsets_.skill1,offsets_.skill2,offsets_.skill3};
   for(unsigned n=0;n<4;n++){cache.ready[n]=skillReady(pointer(at(skills,slots[n])));}
   cache.updated=now;
  }
  h.ultimateCD=cache.ultimate;h.summonerCD=cache.summoner;h.skill0CD=cache.skill0;h.skill1CD=cache.skill1;h.ready=cache.ready;
  auto& recall=recalls_[actor];recall.lastSeen=now;h.recalling=recalling(actor);
  if(h.recalling){if(!recall.active)recall.began=now;recall.active=true;double elapsed=std::max(0.,now-recall.began);h.recallProgress=std::min(1.f,float(elapsed)/7.f);h.recallSeconds=std::max(0.f,7.f-float(elapsed));}
  else{recall.active=false;recall.began=0;}
  frame.heroes.push_back(h);
 }
 if(includeMinions){auto minions=pointer(at(entityRoot,offsets_.minionArray));std::set<std::uint64_t> seen;
  for(unsigned i=0;minions&&i<128;i++){auto actor=pointer(at(minions,offsets_.entityStride*i));if(!actor||!seen.insert(actor).second)continue;
   int team=integer(at(actor,offsets_.team));if((team!=1&&team!=2)||team==ownTeam)continue;
   auto hp=pointer(at(actor,offsets_.hpBase));if(integer(at(hp,offsets_.hp))<=0)continue;auto pos=minionPosition(actor);if(pos.x!=0||pos.y!=0)frame.minions.push_back(pos);
  }
 }
 if(monstersUpdated_==0||now-monstersUpdated_>=.2){auto monsters=pointer(at(root,offsets_.monsterRoot));monsters=pointer(at(monsters,offsets_.monsterList1));monsters=pointer(at(monsters,offsets_.monsterArray));
  if(monsters){std::vector<Monster> refreshed;for(unsigned i=0;i<20;i++){auto actor=pointer(at(monsters,offsets_.entityStride*i));if(!actor)continue;
   Monster monster;monster.id=integer(at(actor,offsets_.monsterId));monster.remaining=integer(at(actor,offsets_.monsterHp))/1000+3;monster.total=integer(at(actor,offsets_.monsterMaxHp))/1000+3;
   monster.position={float(integer(at(actor,offsets_.monsterPosX)))/1000.f,float(integer(at(actor,offsets_.monsterPosY)))/1000.f};refreshed.push_back(monster);
  }monsters_=std::move(refreshed);monstersUpdated_=now;}
 }
 frame.monsters=monsters_;return frame;
}
}
