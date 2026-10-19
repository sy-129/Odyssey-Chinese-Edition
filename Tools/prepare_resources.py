#!/usr/bin/env python3
from pathlib import Path
import hashlib,zipfile
root=Path(__file__).resolve().parents[1]
archive=root/'归档.zip'
expected='25974941c91b3a9daecf6171a844c6172495b2692679d7910866b2e90981a633'
if hashlib.sha256(archive.read_bytes()).hexdigest()!=expected:raise SystemExit('Original resource archive SHA256 mismatch')
count=0
with zipfile.ZipFile(archive) as z:
    for info in z.infolist():
        parts=Path(info.filename).parts
        if 'wztx' not in parts or info.is_dir():continue
        at=parts.index('wztx');relative=Path(*parts[at:])
        if '..' in relative.parts or relative.is_absolute():raise SystemExit('Unsafe resource path')
        destination=root/'Resources'/relative;destination.parent.mkdir(parents=True,exist_ok=True)
        destination.write_bytes(z.read(info));count+=1
if count!=699:raise SystemExit(f'Expected 699 assets, got {count}')
print(f'Verified original archive; restored {count} assets')
