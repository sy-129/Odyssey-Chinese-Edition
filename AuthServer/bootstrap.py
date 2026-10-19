"""Create this NEW product's administrator and Ed25519 issuer; never reads original auth."""
import argparse,base64,hashlib,json,os,secrets,sqlite3
from pathlib import Path
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
SCHEMA='''
CREATE TABLE IF NOT EXISTS admin(username TEXT PRIMARY KEY,salt TEXT NOT NULL,password_hash TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS sessions(token_hash TEXT PRIMARY KEY,expires INTEGER NOT NULL);
CREATE TABLE IF NOT EXISTS licenses(id TEXT PRIMARY KEY,key_hash TEXT UNIQUE NOT NULL,expires INTEGER NOT NULL,device TEXT,revoked INTEGER NOT NULL DEFAULT 0);
CREATE TABLE IF NOT EXISTS challenges(nonce_hash TEXT PRIMARY KEY,device TEXT NOT NULL,expires INTEGER NOT NULL);
CREATE TABLE IF NOT EXISTS refresh_tokens(token_hash TEXT PRIMARY KEY,license_id TEXT NOT NULL,device TEXT NOT NULL,expires INTEGER NOT NULL);
'''
def password_hash(password,salt):return hashlib.pbkdf2_hmac('sha256',password.encode(),bytes.fromhex(salt),600000).hex()
def init(state:Path):
 state.mkdir(parents=True,exist_ok=True);state.chmod(0o700)
 if (state/'issuer.pem').exists() or (state/'auth.db').exists():raise SystemExit('State already exists; refusing to overwrite credentials or signing keys.')
 key=Ed25519PrivateKey.generate();private=key.private_bytes(serialization.Encoding.PEM,serialization.PrivateFormat.PKCS8,serialization.NoEncryption());(state/'issuer.pem').write_bytes(private);(state/'issuer.pem').chmod(0o600)
 password=secrets.token_urlsafe(24);salt=secrets.token_hex(16)
 with sqlite3.connect(state/'auth.db') as db:
  db.executescript(SCHEMA);db.execute('INSERT INTO admin VALUES(?,?,?)',('admin',salt,password_hash(password,salt)))
 (state/'auth.db').chmod(0o600)
 public=base64.b64encode(key.public_key().public_bytes(serialization.Encoding.Raw,serialization.PublicFormat.Raw)).decode()
 (state/'public-config.json').write_text(json.dumps({'algorithm':'Ed25519','public_key_base64':public,'token_lifetime_seconds':600},indent=2))
 (state/'ADMIN-CREDENTIALS.txt').write_text('New TrollSpeed rebuild authorization system\nUsername: admin\nPassword: '+password+'\nThese credentials do not unlock the original application.\nNever place this file or issuer.pem in an IPA or public repository.\n');(state/'ADMIN-CREDENTIALS.txt').chmod(0o600)
 print('New authorization system initialized. Credentials are stored in ADMIN-CREDENTIALS.txt; values are not printed.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--state',type=Path,required=True);a=p.parse_args();init(a.state)
