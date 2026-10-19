"""Compare the original ARM64 candidate verifier/patcher with reconstructed C++.
The original instructions execute in Unicorn. Only task/kernel calls, vector
allocation and deallocation are controlled. No iOS process or game is executed.
"""
import argparse,ctypes,hashlib,json,struct
from pathlib import Path
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE
from unicorn import arm64_const as A
p=argparse.ArgumentParser();p.add_argument('--binary',type=Path,required=True);p.add_argument('--library',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
binary=args.binary.read_bytes();digest=hashlib.sha256(binary).hexdigest()
assert digest=='9edd63717324f62a1ff5fcb52c93bc3a56201e2416813db493cfb9556c89a605'
BASE=0x100000000;STACK=0x200000000;CONFIG=0x400000000;STATE=0x400001000;GUARD=0x400002000;BUFFER=0x400010000
REMOTE=0x500000000;SIZE=0x400000;OBJECT=REMOTE+0x300000;VMSTATE=REMOTE+0x301000;BC=REMOTE+0x302000;CODE=REMOTE+0x310000;STOP=0x700000000;RECORD=STACK+0x1000
profile=json.loads((Path(__file__).resolve().parents[1]/'Resources/OriginalOffsets.json').read_text())
pcs=[profile['tsCoordPc'+str(n)] for n in range(profile['tsCoordCount'])]
lib=ctypes.CDLL(str(args.library.resolve()));lib.ts_coordinate_run.argtypes=[ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,ctypes.c_int,ctypes.POINTER(ctypes.c_uint)];lib.ts_coordinate_run.restype=ctypes.c_uint
register=lambda n:getattr(A,'UC_ARM64_REG_X'+str(n))
def fixture(active):
 b=bytearray(SIZE)
 def setv(address,fmt,value):struct.pack_into('<'+fmt,b,address-REMOTE,value)
 setv(REMOTE,'I',0xfeedfacf);setv(REMOTE+profile['tsDispatcherOffset'],'I',profile['tsDispatcherInstr'])
 setv(OBJECT,'Q',REMOTE+profile['tsVtableOffset']);setv(OBJECT+8,'Q',VMSTATE);setv(OBJECT+24,'Q',BC);setv(BC+8,'Q',CODE)
 for offset,value in [(98,0),(100,2),(131,33)]:setv(OBJECT+offset,'B',value)
 setv(VMSTATE+372,'B',2);setv(VMSTATE+312,'I',80000)
 for pc in pcs[:active]:b[CODE-REMOTE+pc:CODE-REMOTE+pc+4]=bytes([2,17,18,19])
 return b
def execute_original(memory,mode):
 u=Uc(UC_ARCH_ARM64,UC_MODE_ARM);u.mem_map(BASE,0x500000);u.mem_write(BASE,binary);u.mem_map(STACK,0x20000);u.mem_map(CONFIG,0x100000);u.mem_map(REMOTE,SIZE);u.mem_write(REMOTE,bytes(memory))
 def put(address,fmt,value):u.mem_write(address,struct.pack('<'+fmt,value))
 for slot,target in [(0x10032cb60,CONFIG),(0x10032cb58,STATE),(0x10032cb38,GUARD)]:put(slot,'Q',target)
 put(GUARD,'B',1);put(STATE+0x10,'Q',REMOTE);put(CONFIG+0x18,'Q',profile['tsVtableOffset'])
 fields={0x20:'tsVmObjStateOff',0x24:'tsVmObjBytecodeObjOff',0x28:'tsVmObjOpcodeTableOff',0x2c:'tsVmObjStoreOpcodeOff',0x30:'tsVmObjTrueNopOpcodeOff',0x34:'tsOpcodeInvalidValue',0x38:'tsOpcodeStoreValue',0x3c:'tsOpcodeTrueNopValue',0x40:'tsStatePcOff',0x44:'tsStateCodeLenOff',0x48:'tsStateModeOff',0x4c:'tsVmMode',0x50:'tsBcObjDataOff',0x54:'tsKillTargetCodeLen',0x58:'tsBytecodeScanSize',0x9c:'tsCoordCount'}
 for offset,key in fields.items():put(CONFIG+offset,'I',profile[key])
 for n,pc in enumerate(pcs):put(CONFIG+0x5c+n*4,'I',pc)
 put(RECORD,'Q',OBJECT)
 def hook(cpu,address,size,user):
  def ret(value=None):
   if value is not None:cpu.reg_write(register(0),value)
   cpu.reg_write(A.UC_ARM64_REG_PC,cpu.reg_read(A.UC_ARM64_REG_LR))
  if address==0x10003625c:ret(123)
  elif address==0x1000192bc:
   vector,length,fill=(cpu.reg_read(register(n)) for n in range(3));cpu.mem_write(BUFFER,bytes(cpu.mem_read(fill,1))*length)
   cpu.mem_write(vector,struct.pack('<3Q',BUFFER,BUFFER+length,BUFFER+length));ret()
  elif address==0x1000c9268:ret()
  elif address==0x1000c9a60:
   source,length,destination,copied=(cpu.reg_read(register(n)) for n in range(1,5))
   count=3 if mode==3 and CODE<=source<CODE+80000 and length==4 else length
   if REMOTE<=source and source+count<=REMOTE+SIZE:
    cpu.mem_write(destination,bytes(cpu.mem_read(source,count)));cpu.mem_write(copied,struct.pack('<Q',count));ret(0)
   else:cpu.mem_write(copied,struct.pack('<Q',0));ret(1)
  elif address==0x1000c9658:
   destination,source,length=(cpu.reg_read(register(n)) for n in range(1,4))
   if mode==1:ret(5)
   else:
    data=bytearray(cpu.mem_read(source,length))
    if mode==2 and length>1:data[1]^=0xff
    cpu.mem_write(destination,bytes(data));ret(0)
 u.hook_add(UC_HOOK_CODE,hook);u.reg_write(register(0),RECORD);u.reg_write(A.UC_ARM64_REG_SP,STACK+0x18000);u.reg_write(A.UC_ARM64_REG_LR,STOP)
 u.emu_start(BASE+0x18b60,STOP,count=200000)
 assert u.reg_read(A.UC_ARM64_REG_PC)==STOP,'Original routine exceeded execution bound'
 count=u.reg_read(A.UC_ARM64_REG_W0);record=bytes(u.mem_read(RECORD,48));record_count,mask=struct.unpack_from('<II',record,40)
 assert count==record_count,(count,record_count)
 return count,mask,bytes(u.mem_read(REMOTE,SIZE))
cases=[]
for active in range(10):cases.append((f'{active}_STORE_locations',fixture(active),0))
for name,address,fmt,value in [('wrong_vtable',OBJECT,'Q',0),('missing_state',OBJECT+8,'Q',0),('wrong_mode',VMSTATE+372,'B',1),('wrong_NOP_opcode',OBJECT+131,'B',32),('wrong_STORE_opcode',OBJECT+100,'B',5),('wrong_invalid_opcode',OBJECT+98,'B',1),('short_code',VMSTATE+312,'I',63),('oversized_code',VMSTATE+312,'I',80001)]:
 b=fixture(9);struct.pack_into('<'+fmt,b,address-REMOTE,value);cases.append((name,b,0))
for mode,name in [(1,'write_denied'),(2,'write_corruption'),(3,'short_read_back')]:cases.append((name,fixture(9),mode))
b=fixture(0)
for pc in pcs:b[CODE-REMOTE+pc:CODE-REMOTE+pc+4]=bytes([33])*4
cases.append(('already_NOP_fresh_candidate',b,0))
results=[]
for name,memory,mode in cases:
 expected_count,expected_mask,expected_memory=execute_original(memory,mode)
 buffer=(ctypes.c_ubyte*SIZE).from_buffer_copy(memory);mask=ctypes.c_uint();actual_count=lib.ts_coordinate_run(buffer,SIZE,mode,ctypes.byref(mask));actual_memory=bytes(buffer)
 assert (actual_count,mask.value,actual_memory)==(expected_count,expected_mask,expected_memory),f'Mismatch: {name}, counts {actual_count}/{expected_count}, masks {mask.value}/{expected_mask}'
 results.append({'case':name,'patched':actual_count,'matched_mask':mask.value,'memory_equal':True})
report={'original_binary_sha256':digest,'routine':'0x100018b60','passed':True,'case_count':len(results),'cases':results,'scope':'Original ARM64 candidate validation and opcode patching versus reconstructed C++; not region discovery, target-game compatibility or iOS execution','controlled_boundaries':['task-port lookup','kernel memory read/write','vector allocation/deallocation']}
args.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':True,'case_count':len(results),'all_remote_bytes_equal':True,'output':str(args.output)}))
