#!/usr/bin/env python3
"""Generate a reviewable Xcode application project from checked-in source files."""
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[1]
objects={}
def identifier(s):return hashlib.sha1(s.encode()).hexdigest()[:24].upper()
def obj(identity,isa,**fields):
 key=identifier(identity);objects[key]={'isa':isa,**fields};return key
def emit(x):
 if isinstance(x,dict):return '{ '+''.join(f'{json.dumps(k)} = {emit(v)}; ' for k,v in x.items())+'}'
 if isinstance(x,list):return '('+', '.join(emit(y) for y in x)+')'
 if isinstance(x,int):return str(x)
 return json.dumps(x,ensure_ascii=False)
sources=sorted([*root.glob('Core/*.cpp'),*root.glob('App/*.mm'),root/'AuthClient/LicenseClient.swift',*[root/f'Vendor/imgui/{x}.cpp' for x in ['imgui','imgui_draw','imgui_tables','imgui_widgets']],root/'Vendor/imgui/backends/imgui_impl_metal.mm'])
refs=[];builds=[]
for p in sources:
 path=str(p.relative_to(root));kind={'cpp':'sourcecode.cpp.cpp','mm':'sourcecode.cpp.objcpp','swift':'sourcecode.swift'}[p.suffix[1:]]
 ref=obj(path,'PBXFileReference',lastKnownFileType=kind,path=path,sourceTree='<group>');refs.append(ref)
 builds.append(obj('build:'+path,'PBXBuildFile',fileRef=ref))
for p in sorted([*root.glob('Core/*.hpp'),*root.glob('App/*.hpp'),root/'App/Info.plist',root/'App/Entitlements.plist']):
 path=str(p.relative_to(root));kind='sourcecode.cpp.h' if p.suffix=='.hpp' else 'text.plist.xml'
 refs.append(obj(path,'PBXFileReference',lastKnownFileType=kind,path=path,sourceTree='<group>'))
resources=[]
for path,kind in [('Resources/wztx','folder'),('Resources/xuanfuqiu.jpg','image.jpeg'),('Resources/AppIcon60x60@2x.png','image.png'),('Resources/OriginalOffsets.json','text.json'),('AuthClient/public-config.json','text.json')]:
 ref=obj(path,'PBXFileReference',lastKnownFileType=kind,path=path,sourceTree='<group>');refs.append(ref);resources.append(obj('resource:'+path,'PBXBuildFile',fileRef=ref))
frameworks=[]
for name in ['UIKit','Foundation','Metal','MetalKit','QuartzCore','CoreGraphics','Security']:
 ref=obj('framework:'+name,'PBXFileReference',lastKnownFileType='wrapper.framework',path=f'System/Library/Frameworks/{name}.framework',sourceTree='SDKROOT');refs.append(ref);frameworks.append(obj('link:'+name,'PBXBuildFile',fileRef=ref))
product=obj('product','PBXFileReference',explicitFileType='wrapper.application',path='TrollSpeedRebuild.app',sourceTree='BUILT_PRODUCTS_DIR')
products=obj('products','PBXGroup',name='Products',children=[product],sourceTree='<group>')
main=obj('main','PBXGroup',children=refs+[products],sourceTree='<group>')
phases=[]
for name,isa,files in [('sources','PBXSourcesBuildPhase',builds),('frameworks','PBXFrameworksBuildPhase',frameworks),('resources','PBXResourcesBuildPhase',resources)]:phases.append(obj(name,isa,buildActionMask=2147483647,files=files,runOnlyForDeploymentPostprocessing=0))
project_configs=[];target_configs=[]
for mode in ['Debug','Release']:
 project_configs.append(obj('project:'+mode,'XCBuildConfiguration',name=mode,buildSettings={'CLANG_ENABLE_MODULES':'YES','CLANG_ENABLE_OBJC_ARC':'YES','CLANG_CXX_LANGUAGE_STANDARD':'c++17','IPHONEOS_DEPLOYMENT_TARGET':'16.0','SDKROOT':'iphoneos'}))
 target_configs.append(obj('target:'+mode,'XCBuildConfiguration',name=mode,buildSettings={'PRODUCT_NAME':'$(TARGET_NAME)','PRODUCT_BUNDLE_IDENTIFIER':'local.trollspeed.rebuild','PRODUCT_MODULE_NAME':'TrollSpeedRebuild','SWIFT_OBJC_INTERFACE_HEADER_NAME':'TrollSpeedRebuild-Swift.h','SWIFT_VERSION':'5.0','SWIFT_OPTIMIZATION_LEVEL':'-Onone' if mode=='Debug' else '-O','HEADER_SEARCH_PATHS':['$(SRCROOT)/Core','$(SRCROOT)/App','$(SRCROOT)/Vendor/imgui','$(SRCROOT)/Vendor/imgui/backends'],'CODE_SIGNING_ALLOWED':'NO','CODE_SIGN_ENTITLEMENTS':'App/Entitlements.plist','INFOPLIST_FILE':'App/Info.plist','TARGETED_DEVICE_FAMILY':'1','SUPPORTED_PLATFORMS':'iphoneos','ALWAYS_EMBED_SWIFT_STANDARD_LIBRARIES':'YES','LD_RUNPATH_SEARCH_PATHS':['$(inherited)','@executable_path/Frameworks'],'GCC_OPTIMIZATION_LEVEL':'0' if mode=='Debug' else '2'}))
pc=obj('projectconfigs','XCConfigurationList',buildConfigurations=project_configs,defaultConfigurationIsVisible=0,defaultConfigurationName='Release')
tc=obj('targetconfigs','XCConfigurationList',buildConfigurations=target_configs,defaultConfigurationIsVisible=0,defaultConfigurationName='Release')
target=obj('target','PBXNativeTarget',name='TrollSpeedRebuild',productName='TrollSpeedRebuild',productType='com.apple.product-type.application',productReference=product,buildConfigurationList=tc,buildPhases=phases,buildRules=[],dependencies=[])
project=obj('project','PBXProject',attributes={'LastUpgradeCheck':'1600'},buildConfigurationList=pc,compatibilityVersion='Xcode 14.0',developmentRegion='en',knownRegions=['en','Base'],mainGroup=main,productRefGroup=products,projectDirPath='',projectRoot='',targets=[target])
folder=root/'TrollSpeedRebuild.xcodeproj';folder.mkdir(exist_ok=True)
(folder/'project.pbxproj').write_text('// !$*UTF8*$!\n'+emit({'archiveVersion':1,'classes':{},'objectVersion':56,'objects':objects,'rootObject':project})+'\n')
schemes=folder/'xcshareddata/xcschemes';schemes.mkdir(parents=True,exist_ok=True)
(schemes/'TrollSpeedRebuild.xcscheme').write_text(f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="1600" version="1.3"><BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries><BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES"><BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{target}" BuildableName="TrollSpeedRebuild.app" BlueprintName="TrollSpeedRebuild" ReferencedContainer="container:TrollSpeedRebuild.xcodeproj"/></BuildActionEntry></BuildActionEntries></BuildAction><ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/></Scheme>''')
old=schemes/'TrollSpeedCore.xcscheme'
if old.exists():old.unlink()
print(f'Generated app project: {len(sources)} translation units, {len(objects)} objects')
