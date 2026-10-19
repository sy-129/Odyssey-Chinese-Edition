"""Verify package provenance, bundled assets and ad hoc main-code/resource seals.
This is an integrity check, not device execution or a certificate-trust decision.
"""
import argparse,hashlib,json,plistlib,struct,zipfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--ipa',type=Path,required=True);p.add_argument('--commit',required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
source=Path(__file__).resolve().parents[1]
with zipfile.ZipFile(args.ipa) as z:
    assert z.testzip() is None,'ZIP CRC failed'
    names=set(z.namelist());assert len(names)==len(z.namelist()),'Duplicate ZIP entries'
    assert all(not Path(n).is_absolute() and '..' not in Path(n).parts for n in names),'Unsafe package path'
    infos=[n for n in names if len(Path(n).parts)==3 and n.startswith('Payload/') and n.endswith('.app/Info.plist')]
    assert len(infos)==1,'Expected exactly one root application'
    prefix=infos[0].rsplit('/',1)[0]+'/'
    info_bytes=z.read(infos[0]);info=plistlib.loads(info_bytes)
    assert info['ReconstructionCommit']==args.commit,'Package does not match expected source commit'
    assert info['MinimumOSVersion']=='16.0','Unexpected deployment target'
    assert info['CFBundleIdentifier']=='local.trollspeed.rebuild'
    executable=prefix+info['CFBundleExecutable'];binary=z.read(executable)
    assert (z.getinfo(executable).external_attr>>16)&0o111,'Missing executable mode'
    assert hashlib.sha256(binary).hexdigest()!='9edd63717324f62a1ff5fcb52c93bc3a56201e2416813db493cfb9556c89a605','Original executable was reused'
    magic,cpu=struct.unpack_from('<II',binary)
    assert magic==0xfeedfacf and cpu==0x100000c,'Expected thin arm64 Mach-O'
    count=struct.unpack_from('<I',binary,16)[0];offset=32;signature=None;minimum=None
    for _ in range(count):
        command,size=struct.unpack_from('<II',binary,offset);assert size>=8 and offset+size<=len(binary)
        if command==0x1d:signature=struct.unpack_from('<II',binary,offset+8)
        if command==0x32:
            platform,minimum,sdk=struct.unpack_from('<III',binary,offset+8);assert platform==2 and minimum==0x100000,'Mach-O target mismatch'
        offset+=size
    assert signature and minimum is not None,'Missing signature/build metadata'
    cs_offset,cs_size=signature;sig_magic,sig_length,sig_count=struct.unpack_from('>III',binary,cs_offset)
    assert sig_magic==0xfade0cc0 and sig_length<=cs_size and cs_offset+sig_length<=len(binary)
    blobs={}
    for n in range(sig_count):
        slot,relative=struct.unpack_from('>II',binary,cs_offset+12+n*8);start=cs_offset+relative;_,length=struct.unpack_from('>II',binary,start)
        assert length>=8 and start+length<=cs_offset+sig_length
        blobs[slot]=binary[start:start+length]
    assert 5 in blobs,'Missing signed XML entitlements'
    entitlements=plistlib.loads(blobs[5][8:]);expected=plistlib.loads((source/'App/Entitlements.plist').read_bytes())
    assert entitlements==expected,'Signed entitlements differ from source'
    sealed_bytes=z.read(prefix+'_CodeSignature/CodeResources');sealed=plistlib.loads(sealed_bytes)
    special={1:info_bytes,3:sealed_bytes,**{slot:data for slot,data in blobs.items() if slot in (2,5,7)}}
    code_slots=0;special_slots=0;directories=0
    for slot,cd in blobs.items():
        if struct.unpack_from('>I',cd)[0]!=0xfade0c02:continue
        directories+=1
        version,flags,hash_offset,identifier,special_count,code_count,code_limit=struct.unpack_from('>7I',cd,8)
        hash_size,hash_type,platform,page_exp=struct.unpack_from('4B',cd,36)
        digest={1:hashlib.sha1,2:hashlib.sha256,3:hashlib.sha256,4:hashlib.sha384}[hash_type]
        if version>=0x20300:
            large_limit=struct.unpack_from('>Q',cd,56)[0]
            if large_limit:code_limit=large_limit
        page=1<<page_exp if page_exp else code_limit
        assert code_count==(code_limit+page-1)//page and code_limit<=cs_offset
        for n in range(code_count):
            data=binary[n*page:min((n+1)*page,code_limit)];wanted=cd[hash_offset+n*hash_size:hash_offset+(n+1)*hash_size]
            assert digest(data).digest()[:hash_size]==wanted,('Code page mismatch',slot,n)
            code_slots+=1
        for n,data in special.items():
            if n>special_count:continue
            wanted=cd[hash_offset-n*hash_size:hash_offset-(n-1)*hash_size]
            assert digest(data).digest()[:hash_size]==wanted,('Special slot mismatch',slot,n)
            special_slots+=1
    assert directories and code_slots,'No verified CodeDirectory'
    resource_count=0;nested=0
    for path,entry in sealed.get('files2',sealed.get('files',{})).items():
        full=prefix+path
        if full not in names:
            assert isinstance(entry,dict) and entry.get('optional'),('Missing sealed resource',path)
            continue
        if isinstance(entry,bytes):assert hashlib.sha1(z.read(full)).digest()==entry;resource_count+=1;continue
        if entry.get('hash2') is not None:assert hashlib.sha256(z.read(full)).digest()==entry['hash2'];resource_count+=1
        elif entry.get('hash') is not None:assert hashlib.sha1(z.read(full)).digest()==entry['hash'];resource_count+=1
        elif entry.get('cdhash') is not None:nested+=1
    assets=[n for n in names if n.startswith(prefix+'wztx/') and not n.endswith('/')];assert len(assets)==699
    for path in assets:
        relative=path[len(prefix):];assert z.read(path)==(source/'Resources'/relative).read_bytes(),('Original asset mismatch',relative)
    for name in ['xuanfuqiu.jpg','AppIcon60x60@2x.png']:
        assert z.read(prefix+name)==(source/'Resources'/name).read_bytes(),('Original UI asset mismatch',name)
    assert z.read(prefix+'public-config.json')==(source/'AuthClient/public-config.json').read_bytes()
    forbidden={'Reference','Tests','AuthServer','issuer.pem','auth.db','ADMIN-CREDENTIALS.txt','original-arm64.bin','offline-license-fixture.json','offline-client-checks'}
    assert all(not forbidden.intersection(Path(n).parts) for n in names),'Private/reference/test material in application'
report={'passed':True,'source_commit':args.commit,'ipa_sha256':hashlib.sha256(args.ipa.read_bytes()).hexdigest(),'executable_sha256':hashlib.sha256(binary).hexdigest(),'minimum_ios':'16.0','architecture':'arm64','original_game_images':len(assets),'original_ui_images':2,'main_code_directories_verified':directories,'main_code_pages_verified':code_slots,'main_special_slots_verified':special_slots,'resource_hashes_verified':resource_count,'nested_code_entries_not_independently_verified':nested,'entitlements_match_source':True,'runtime_verified':False,'scope':'Package provenance, main executable page/special hashes and resource hashes; device execution, certificate trust and current-game compatibility are outside this check'}
args.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
