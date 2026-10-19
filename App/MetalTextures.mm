#include "MetalTextures.hpp"
#import <MetalKit/MetalKit.h>
MetalTextures::MetalTextures(id<MTLDevice> device):device_(device){
 auto* descriptor=[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:64 height:64 mipmapped:NO];
 fallback_=[device_ newTextureWithDescriptor:descriptor];std::uint32_t pixels[64*64]{};
 for(int y=0;y<64;y++)for(int x=0;x<64;x++)if((x-32)*(x-32)+(y-32)*(y-32)<=900)pixels[y*64+x]=0xff6464ff;
 [fallback_ replaceRegion:MTLRegionMake2D(0,0,64,64) mipmapLevel:0 withBytes:pixels bytesPerRow:64*4];
}
id<MTLTexture> MetalTextures::loadHero(int assetID){
 auto found=heroes_.find(assetID);if(found!=heroes_.end())return found->second;
 return heroes_[assetID]=loadImage(assetID,true)?:fallback_;
}
id<MTLTexture> MetalTextures::loadSkill(int assetID,int alternate){
 auto key=std::make_pair(assetID,alternate);auto found=skills_.find(key);if(found!=skills_.end())return found->second;
 auto image=loadImage(assetID,false);if(!image&&alternate>0)image=loadImage(alternate,false);
 return skills_[key]=image?:fallback_;
}
id<MTLTexture> MetalTextures::loadImage(int assetID,bool portrait){
 if(!portrait){if(assetID==80116)assetID=80104;else if(assetID==80117)assetID=80118;}
 NSString* path=nil;for(NSString* extension in (portrait?@[@"jpg",@"png"]:@[@"png"])){
  path=[[NSBundle mainBundle] pathForResource:[NSString stringWithFormat:@"%d",assetID] ofType:extension inDirectory:@"wztx"];if(path)break;
 }
 id<MTLTexture> texture=nil;if(path){NSError* error=nil;MTKTextureLoader* loader=[[MTKTextureLoader alloc] initWithDevice:device_];texture=[loader newTextureWithContentsOfURL:[NSURL fileURLWithPath:path] options:@{MTKTextureLoaderOptionSRGB:@NO} error:&error];if(error)NSLog(@"Texture %d: %@",assetID,error);}
 return texture;
}
