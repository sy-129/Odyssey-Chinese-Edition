#pragma once
#import <Metal/Metal.h>
#include "ESPRenderer.hpp"
#include <map>
class MetalTextures:public ts::Textures {
 id<MTLDevice> device_;std::map<int,id<MTLTexture>> heroes_,skills_;id<MTLTexture> fallback_;
 id<MTLTexture> load(int,bool);
public:
 explicit MetalTextures(id<MTLDevice>);
 ImTextureID hero(int id)override{return (__bridge void*)load(id,true);}
 ImTextureID skill(int id)override{return (__bridge void*)load(id,false);}
};
