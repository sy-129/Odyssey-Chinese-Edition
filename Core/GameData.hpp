#pragma once
#include "Geometry.hpp"
#include "Offsets.hpp"
#include "RemoteMemory.hpp"
#include <vector>
#include <map>
namespace ts {
struct Hero {
 std::uint64_t identity=0;int id=0,team=0,hp=0,maxHP=0;
 float healthFraction=0;Vec2 position{};bool dead=false;
 int ultimateCD=0,summonerCD=0,skill0CD=0,skill1CD=0,summonerID=0,alternateSummonerID=0;
 std::array<bool,4> ready{};bool recalling=false;float recallProgress=0,recallSeconds=0;
};
struct Monster {int id=0,remaining=0,total=0;Vec2 position{};};
struct Frame {Matrix matrix{};std::vector<Hero> heroes;std::vector<Vec2> minions;std::vector<Monster> monsters;double timestamp=0;};
class GameReader {
 const Memory& memory_;Offsets offsets_;std::uint64_t module_;int pid_;PositionSmoother smoother_;
 struct CachedSkill {double updated=0;int ultimate=0,summoner=0,skill0=0,skill1=0;std::array<bool,4> ready{};};
 struct CachedRecall {bool active=false;double began=0,lastSeen=0;};
 std::map<std::uint64_t,CachedSkill> skills_;std::map<std::uint64_t,CachedRecall> recalls_;
 std::vector<Monster> monsters_;double monstersUpdated_=0;
 std::uint64_t pointer(std::uint64_t address)const;
 int integer(std::uint64_t address)const;
 std::uint64_t at(std::uint64_t base,std::uint64_t offset)const;
 Vec2 rawPosition(std::uint64_t address)const;
 Vec2 entityPosition(std::uint64_t entity)const;
 Vec2 heroPosition(std::uint64_t entity)const;
 Vec2 minionPosition(std::uint64_t entity)const;
 int cooldown(std::uint64_t entity,std::uint64_t slot)const;
 bool skillReady(std::uint64_t skill)const;
 std::array<int,2> talents(std::uint64_t entity)const;
 bool recalling(std::uint64_t entity)const;
public:
 GameReader(const Memory& m,Offsets offsets,std::uint64_t module,int pid):memory_(m),offsets_(offsets),module_(module),pid_(pid){}
 void reset();
 std::optional<Frame> capture(double now,bool includeMinions=true);
};
}
