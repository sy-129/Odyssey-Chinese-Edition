#include "GameData.hpp"
#include <map>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <limits>
struct Memory:ts::Memory {
 std::map<std::uint64_t,std::uint8_t> bytes;
 template<class T>void set(std::uint64_t p,const T& value){auto* b=reinterpret_cast<const std::uint8_t*>(&value);for(std::size_t n=0;n<sizeof value;n++)bytes[p+n]=b[n];}
 ts::ReadResult read(std::uint64_t p,void* out,std::size_t count)const override{auto* b=static_cast<std::uint8_t*>(out);for(std::size_t n=0;n<count;n++){auto it=bytes.find(p+n);if(it==bytes.end())return {false,n};b[n]=it->second;}return {true,count};}
};
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(){
 Memory memory;ts::Offsets o;const std::uint64_t module=0x100000000ULL;
 const auto root=module+0x1000,entities=module+0x2000,actors=module+0x3000,hero=module+0x4000,hp=module+0x5000,skills=module+0x6000;
 auto link=[&](std::uint64_t address,std::uint64_t target){memory.set(address,target);};
 link(module+o.gameStruct,root);link(root+o.entityRoot,entities);link(entities+o.actorArray,actors);link(actors,hero);
 memory.set<std::int32_t>(hero+o.team,2);memory.set<std::int32_t>(hero+o.heroId,123);link(hero+o.hpBase,hp);memory.set<std::int32_t>(hp+o.hp,50);memory.set<std::int32_t>(hp+o.maxHp,100);
 const auto c0=module+0x7000,c1=module+0x8000,c2=module+0x9000,position=module+0xa000;
 link(hero+o.coordBase,c0);link(c0+o.coordHop2,c1);link(c1+o.coordHop3,c2);link(c2+o.coordHop4,position);
 memory.set<std::int32_t>(position+o.posX,10000);memory.set<std::int32_t>(position+o.posY,-20000);
 auto m0=module+0x10000,m1=module+0x11000,m2=module+0x12000,m3=module+0x13000;
 link(module+o.matrix,m0);link(m0+o.matrixHop1,m1);link(m1+o.matrixHop2,m2);link(m2+o.matrixHop3,m3);
 ts::Matrix matrix{};matrix[0]=1;matrix[14]=1;memory.set(m3+o.matrixField,matrix);link(hero+o.skillList,skills);
 std::array<std::uint64_t,5> slots{o.skill0,o.skill1,o.skill2,o.skill3,o.skill4};
 for(unsigned i=0;i<slots.size();i++){
  auto skill=module+0x20000+i*0x1000,cd=module+0x30000+i*0x1000;link(skills+slots[i],skill);link(skill+o.skillCdPtr,cd);
  memory.set<std::int32_t>(cd+o.skillCdValue,(i+1)*8192000);memory.set<std::int32_t>(skill+o.skillValidField,1);memory.set<std::int32_t>(skill+o.skillReadyField,i%2);
 }
 ts::GameReader reader(memory,o,module,123);auto result=reader.capture(10,false);require(result&&result->heroes.size()==1,"enemy actor pointer graph");const auto& h=result->heroes[0];
 require(h.id==123&&h.hp==50&&h.maxHP==100&&h.healthFraction==.5f,"hero identity and health");
 require(h.position.x==10&&h.position.y==-20,"legacy coordinate chain");
 // The original loader maps DAT_10041db18/db1c/db20/db24 to skill1/2/3/4.
 require(h.skill0CD==2&&h.skill1CD==3&&h.ultimateCD==4&&h.summonerCD==5,"visible skill numbering differs from raw profile slot zero");
 require(h.ready==std::array<bool,4>{true,false,true,false},"visible readiness follows slots 1/2/3/4");
 memory.set<std::int32_t>(position+o.posX,20000);result=reader.capture(10.2,false);require(result&&result->heroes[0].position.x==11.5f,"within-match samples retain smoothing history");
 link(entities+o.actorArray,0);require(!reader.capture(10.3,false),"missing actor root invalidates the frame");
 link(entities+o.actorArray,actors);result=reader.capture(10.4,false);require(result&&result->heroes[0].position.x==20,"new actor root must not reuse previous-match smoothing");
 memory.set<std::int32_t>(hero+o.team,1);result=reader.capture(11,false);require(result&&result->heroes.empty(),"own team must be excluded");
 matrix[7]=std::numeric_limits<float>::quiet_NaN();memory.set(m3+o.matrixField,matrix);require(!reader.capture(11.5,false),"nonfinite matrix must not reach rendering or diagnostic serialization");
 memory.set(m3+o.matrixField,ts::Matrix{});require(!reader.capture(12,false),"zero matrix invalidates match data");
 std::cout<<"11 data pointer-graph, skill-slot and match-reset checks passed\n";
}
