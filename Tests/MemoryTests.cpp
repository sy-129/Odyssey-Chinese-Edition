#include "RemoteMemory.hpp"
#include "Geometry.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <limits>
struct Fixture:ts::Memory{
 std::vector<std::uint8_t> bytes;std::uint64_t base=0x100000000;bool shortRead=false;
 explicit Fixture(std::size_t n):bytes(n){}
 ts::ReadResult read(std::uint64_t address,void* out,std::size_t count)const override{
  if(address<base||address-base>bytes.size()||count>bytes.size()-(address-base))return {false,0};
  std::size_t copied=shortRead&&count?count-1:count;std::memcpy(out,bytes.data()+(address-base),copied);return {true,copied};
 }
 void set(std::size_t off,std::uint32_t v){std::memcpy(bytes.data()+off,&v,4);}
};
int checks=0;
void require(bool condition,const char* why){++checks;if(!condition)throw std::runtime_error(why);}
int main(){try{
 require(!ts::add(UINT64_MAX,1),"address overflow must fail");require(ts::add(10,20)==30,"address sum");
 Fixture m(128);std::array<float,16> matrix;for(unsigned i=0;i<16;i++)matrix[i]=float(i)+0.25f;
 std::memcpy(m.bytes.data(),matrix.data(),64);auto v=ts::matrix(m,m.base);require(v&&*v==matrix,"matrix byte order and full content");
 m.shortRead=true;require(!ts::matrix(m,m.base),"successful short read must not become valid matrix");m.shortRead=false;
 require(!ts::matrix(m,m.base+100),"unmapped matrix read must fail");require(!m.value<uint64_t>(UINT64_MAX-2),"overflow read rejected");
 Fixture image(128);image.set(0,0xfeedfacf);image.set(12,6);image.set(16,1);image.set(20,64);image.set(32,0xd);image.set(36,64);image.set(40,24);
 const char name[]="@rpath/UnityFramework.framework/UnityFramework";std::memcpy(image.bytes.data()+56,name,sizeof name);
 // Increase the declared command size to fit this install name.
 image.set(20,80);image.set(36,80);
 auto parsed=ts::inspectImage(image,image.base);require(parsed&&parsed->fileType==6&&parsed->installName==name,"valid module install name");
 image.set(36,7);require(!ts::inspectImage(image,image.base),"undersized load command rejected");
 image.set(36,81);require(!ts::inspectImage(image,image.base),"load command beyond declared area rejected");
 image.set(36,80);image.set(40,81);require(!ts::inspectImage(image,image.base),"string offset beyond command rejected");
 image.set(40,24);image.shortRead=true;require(!ts::inspectImage(image,image.base),"partial Mach-O header rejected");image.shortRead=false;
 image.set(16,5000);require(!ts::inspectImage(image,image.base),"unbounded command count rejected");
 Fixture exe(32);exe.set(0,0xfeedfacf);exe.set(12,2);auto main=ts::inspectImage(exe,exe.base);require(main&&main->fileType==2,"main executable without install name");
 exe.set(0,0);require(!ts::inspectImage(exe,exe.base),"invalid header rejected");
 ts::Matrix projection{};projection[0]=1;projection[9]=1;projection[14]=1;ts::Vec2 point;
 require(ts::toScreen({1000,600},{.5f,.5f},projection,point)&&point.x==750&&point.y==150,"projection maps world quadrants");
 projection[14]=.009f;require(!ts::toScreen({1000,600},{0,0},projection,point)&&point.x==0&&point.y==0,"behind-camera points clear output");projection[14]=1;
 auto pos=ts::toMiniMap({10,20,100,200},{10,20},projection,100,100);require(pos.x==100&&pos.y==65,"minimap uses mean dimension");
 projection[0]=-1;pos=ts::toMiniMap({10,20,100,200},{10,20},projection,100,100);require(pos.x==70&&pos.y==125,"opposite team flips map axes");
 require(!ts::decodePosition(INT32_MIN,0)&&!ts::decodePosition(80001,0)&&!ts::decodePosition(0,0),"invalid raw position and signed edge rejected");
 auto valid=ts::decodePosition(80000,-80000);require(valid&&valid->x==80&&valid->y==-80,"raw position inclusive boundary");
 ts::PositionSmoother smooth;auto a=smooth.sample(1,{10,20});require(a.x==10&&a.y==20,"first sample is unfiltered");a=smooth.sample(1,{20,40});require(a.x==11.5f&&a.y==23,"original smoothing coefficient");
 a=smooth.sample(1,{0,0});require(a.x==0&&a.y==0,"zero sample remains invalid");a=smooth.sample(1,{20,40});require(a.x>11.5f&&a.x<20,"invalid zero does not poison history");smooth.clear();a=smooth.sample(1,{20,40});require(a.x==20&&a.y==40,"new match resets smoothing");
 std::cout<<checks<<" checks passed\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
