#include "ESPRenderer.hpp"
#include <iostream>
#include <stdexcept>
struct Textures:ts::Textures {
 ImTextureID hero(int) override{return reinterpret_cast<void*>(1);}
 ImTextureID skill(int) override{return reinterpret_cast<void*>(2);}
};
int main(){
 IMGUI_CHECKVERSION();ImGui::CreateContext();auto& io=ImGui::GetIO();io.DisplaySize={2532,1170};io.DeltaTime=1.f/60;
 unsigned char* pixels=nullptr;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);io.Fonts->SetTexID(reinterpret_cast<void*>(3));
 Textures textures;ts::ESPRenderer renderer(textures);ts::Settings settings;
 ts::Frame frame;frame.matrix[0]=.01f;frame.matrix[9]=.01f;frame.matrix[14]=1;
 // Isolated renderer fixtures: deliberately not presented as game observations.
 for(int i=0;i<5;i++){ts::Hero hero;hero.id=100+i;hero.position={float(i*5),float(i*4)};hero.healthFraction=.2f*i;hero.ready={true,false,true,false};hero.recalling=i%2;hero.recallSeconds=4.2f;hero.skill0CD=10;hero.summonerCD=25;hero.ultimateCD=30;frame.heroes.push_back(hero);}
 frame.minions={{1,2},{3,4}};frame.monsters={{0x1aab,20,30,{1,2}},{0x401b,30,30,{3,4}}};
 ImGui::NewFrame();renderer.draw(ImGui::GetBackgroundDrawList(),frame,settings,{2532,1170});ImGui::Render();
 auto* result=ImGui::GetDrawData();if(!result||result->TotalVtxCount<1000||result->TotalIdxCount<1000)throw std::runtime_error("ESP branches failed to emit geometry");
 const int vertices=result->TotalVtxCount;
 ImGui::NewFrame();renderer.draw(ImGui::GetBackgroundDrawList(),{},settings,{2532,1170});ImGui::Render();
 if(ImGui::GetDrawData()->TotalVtxCount>=vertices)throw std::runtime_error("empty frames must not retain prior heroes");
 ImGui::DestroyContext();std::cout<<"Renderer populated/empty frame checks passed; "<<vertices<<" vertices in populated fixture.\n";
}
