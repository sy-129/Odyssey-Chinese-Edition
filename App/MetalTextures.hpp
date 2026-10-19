#pragma once
#import <Metal/Metal.h>
#include "ESPRenderer.hpp"
#include <map>
class MetalTextures:public ts::Textures {
 id<MTLDevice> device_;std::map<int,id<MTLTexture>> heroes_;std::map<std::pair<int,int>,id<MTLTexture>> skills_;id<MTLTexture> fallback_,floating_;
 id<MTLTexture> loadImage(int,bool);
 id<MTLTexture> loadHero(int);
 id<MTLTexture> loadSkill(int,int);
public:
 explicit MetalTextures(id<MTLDevice>);
 ImTextureID floating()const{return (__bridge void*)floating_;}
 ImTextureID hero(int assetID)override{return (__bridge void*)loadHero(assetID);}
 ImTextureID skill(int assetID,int alternate=0)override{return (__bridge void*)loadSkill(assetID,alternate);}
 bool isFallback(ImTextureID value)const override{return value==(__bridge void*)fallback_;}
};
