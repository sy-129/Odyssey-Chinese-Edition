"""Compare rebuilt functions against bounded execution of original ARM64 arithmetic.
No game or network is launched; Objective-C getters are replaced with fixed test values.
"""
import argparse,ctypes,hashlib,json,math,random,struct
from pathlib import Path
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE
from unicorn import arm64_const as A
p=argparse.ArgumentParser();p.add_argument('--binary',type=Path,required=True);p.add_argument('--library',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
b=a.binary.read_bytes();assert hashlib.sha256(b).hexdigest()=='9edd63717324f62a1ff5fcb52c93bc3a56201e2416813db493cfb9556c89a605'
BASE=0x100000000;STACK=0x200000000;STOP=0x300000000;OUT=STACK+0x1000;MATRIX=BASE+0x41d640
u=Uc(UC_ARCH_ARM64,UC_MODE_ARM);u.mem_map(BASE,0x500000);u.mem_write(BASE,b);u.mem_map(STACK,0x20000)
f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
bits=lambda x:struct.unpack('<I',struct.pack('<f',x))[0]
def getf(reg):return struct.unpack('<f',struct.pack('<I',u.reg_read(reg)))[0]
scales=[1.,1.]
def hook(cpu,address,size,user):
 if address==BASE+0xcbae0:cpu.reg_write(A.UC_ARM64_REG_S0,bits(scales[0]));cpu.reg_write(A.UC_ARM64_REG_PC,cpu.reg_read(A.UC_ARM64_REG_LR))
 elif address==BASE+0xcbb00:cpu.reg_write(A.UC_ARM64_REG_S0,bits(scales[1]));cpu.reg_write(A.UC_ARM64_REG_PC,cpu.reg_read(A.UC_ARM64_REG_LR))
 elif address in [BASE+0xc97e4,BASE+0xc97d8]:cpu.reg_write(A.UC_ARM64_REG_PC,cpu.reg_read(A.UC_ARM64_REG_LR))
u.hook_add(UC_HOOK_CODE,hook)
lib=ctypes.CDLL(str(a.library.resolve()));ptr=ctypes.POINTER(ctypes.c_float)
lib.ts_screen.argtypes=[ctypes.c_float]*4+[ptr,ptr];lib.ts_screen.restype=ctypes.c_int
lib.ts_map.argtypes=[ctypes.c_float]*9+[ptr]
rng=random.Random(1829);count=0;maximum_error=0.
def compare(expected,actual):
 global maximum_error
 for e,v in zip(expected,actual):
  error=abs(e-v);maximum_error=max(maximum_error,error)
  if not math.isclose(e,v,rel_tol=2e-6,abs_tol=2e-4):raise AssertionError((expected,actual))
for n in range(400):
 m=[f32(rng.uniform(-2,2)) for _ in range(16)];xy=[f32(rng.uniform(-80,80)) for _ in range(2)];viewport=[1920.,1080.]
 u.mem_write(MATRIX,struct.pack('<16f',*m));u.reg_write(A.UC_ARM64_REG_X0,OUT);u.reg_write(A.UC_ARM64_REG_SP,STACK+0x18000);u.reg_write(A.UC_ARM64_REG_LR,STOP)
 for i,v in enumerate(viewport+xy):u.reg_write(getattr(A,'UC_ARM64_REG_S'+str(i)),bits(v))
 u.emu_start(BASE+0x28f48,STOP,count=1000);yes=u.reg_read(A.UC_ARM64_REG_W0);expected=struct.unpack('<2f',u.mem_read(OUT,8))
 matrix=(ctypes.c_float*16)(*m);out=(ctypes.c_float*2)();actual_yes=lib.ts_screen(*viewport,*xy,matrix,out)
 assert yes==actual_yes;compare(expected,list(out));count+=1
for n in range(400):
 args=[f32(v) for v in [rng.uniform(0,200),rng.uniform(0,100),rng.uniform(50,500),rng.uniform(50,500),rng.uniform(-80,80),rng.uniform(-80,80)]]
 first=rng.choice([-1.,0.,1.]);scales[:]=[f32(rng.uniform(50,400)),f32(rng.uniform(50,400))]
 u.mem_write(MATRIX,struct.pack('<f',first));u.reg_write(A.UC_ARM64_REG_X0,MATRIX);u.reg_write(A.UC_ARM64_REG_X1,BASE+0x400000);u.reg_write(A.UC_ARM64_REG_SP,STACK+0x18000);u.reg_write(A.UC_ARM64_REG_LR,STOP)
 for i,v in enumerate(args):u.reg_write(getattr(A,'UC_ARM64_REG_S'+str(i)),bits(v))
 u.emu_start(BASE+0x34bec,STOP,count=1000);expected=[getf(A.UC_ARM64_REG_S0),getf(A.UC_ARM64_REG_S1)]
 out=(ctypes.c_float*2)();lib.ts_map(*args,first,*scales,out);compare(expected,list(out));count+=1
result={'original_binary_sha256':hashlib.sha256(b).hexdigest(),'compared_cases':count,'passed':True,'max_absolute_error':maximum_error,'tolerance':{'relative':2e-6,'absolute':2e-4},'scope':['ToScreen finite inputs','mini-map projection finite inputs'],'execution':'Unicorn ARM64 arithmetic only; Objective-C retain/release and scale getters controlled; no iOS/game execution'}
a.output.write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
