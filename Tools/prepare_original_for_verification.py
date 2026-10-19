"""Extract the hash-checked original only into the build evidence directory."""
from pathlib import Path
import hashlib,zipfile
root=Path(__file__).resolve().parents[1]
with zipfile.ZipFile(root/'归档.zip') as archive:
    candidates=[entry for entry in archive.infolist() if Path(entry.filename).name=='TrollSpeed' and not entry.is_dir()]
    if len(candidates)!=1:raise SystemExit('Expected exactly one original TrollSpeed executable')
    binary=archive.read(candidates[0])
if hashlib.sha256(binary).hexdigest()!='9edd63717324f62a1ff5fcb52c93bc3a56201e2416813db493cfb9556c89a605':raise SystemExit('Original executable digest mismatch')
output=root/'build/original-arm64.bin';output.parent.mkdir(parents=True,exist_ok=True);output.write_bytes(binary)
print('Verified original ARM64 reference extracted into build directory')
