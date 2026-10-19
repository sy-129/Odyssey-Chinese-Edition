"""New standalone authorization server for the rebuilt application; no original-auth bypass."""
import base64,hashlib,hmac,json,os,secrets,sqlite3,time,threading
from collections import defaultdict,deque
from contextlib import contextmanager
from pathlib import Path
from fastapi import FastAPI,HTTPException,Request,Depends
from fastapi.security import HTTPBearer,HTTPAuthorizationCredentials
from pydantic import BaseModel,Field
from cryptography.hazmat.primitives import serialization
from bootstrap import password_hash
STATE=Path(os.environ.get('TS_AUTH_STATE_DIR',str(Path.home()/'.local/share/trollspeed-auth')))
if not (STATE/'auth.db').exists():raise RuntimeError('Run bootstrap.py --state <directory>, then set TS_AUTH_STATE_DIR to that directory.')
ISSUER=serialization.load_pem_private_key((STATE/'issuer.pem').read_bytes(),password=None)
app=FastAPI(title='TrollSpeed rebuilt product authorization',docs_url=None,redoc_url=None,openapi_url=None)
security=HTTPBearer(auto_error=False)
@contextmanager
def database():
 db=sqlite3.connect(STATE/'auth.db',timeout=10);db.row_factory=sqlite3.Row
 try:yield db;db.commit()
 except Exception:db.rollback();raise
 finally:db.close()
with database() as db:
 db.execute('CREATE TABLE IF NOT EXISTS refresh_tokens(token_hash TEXT PRIMARY KEY,license_id TEXT NOT NULL,device TEXT NOT NULL,expires INTEGER NOT NULL)')
def digest(s):return hashlib.sha256(s.encode()).hexdigest()
def now():return int(time.time())
lock=threading.Lock();rates=defaultdict(deque)
def limit(request:Request,route:str,maximum:int):
 host=request.client.host if request.client else 'unknown';key=(host,route);t=time.monotonic()
 with lock:
  # Limit state retained by the single-process limiter; deployments should add an edge limiter.
  if len(rates)>10000:
   for k in list(rates):
    if not rates[k] or rates[k][-1]<t-60:del rates[k]
  q=rates[key]
  while q and q[0]<t-60:q.popleft()
  if len(q)>=maximum:raise HTTPException(429,'Too many requests')
  q.append(t)
def admin(auth:HTTPAuthorizationCredentials|None=Depends(security)):
 if not auth:raise HTTPException(401,'Administrator login required')
 with database() as db:
  row=db.execute('SELECT expires FROM sessions WHERE token_hash=?',(digest(auth.credentials),)).fetchone()
 if not row or row['expires']<=now():raise HTTPException(401,'Administrator session expired')
class Login(BaseModel):
 username:str=Field(min_length=1,max_length=64)
 password:str=Field(min_length=1,max_length=256)
class Issue(BaseModel):days:int=Field(default=30,ge=1,le=3650)
class Challenge(BaseModel):device_id:str=Field(min_length=16,max_length=128,pattern=r'^[A-Za-z0-9-]+$')
class Activate(Challenge):
 license_key:str=Field(min_length=16,max_length=160)
 nonce:str=Field(min_length=16,max_length=128)
class Refresh(Challenge):
 refresh_token:str=Field(min_length=16,max_length=160)
 nonce:str=Field(min_length=16,max_length=128)
def signed_lease(db,row,device,nonce):
 token=secrets.token_urlsafe(48)
 db.execute('DELETE FROM refresh_tokens WHERE license_id=? OR expires<=?',(row['id'],now()))
 db.execute('INSERT INTO refresh_tokens VALUES(?,?,?,?)',(digest(token),row['id'],device,row['expires']))
 t=now();payload={'version':1,'license_id':row['id'],'device_id':device,'issued_at':t,'expires_at':min(t+600,row['expires']),'nonce':nonce,'scope':'trollspeed-rebuild'}
 raw=json.dumps(payload,separators=(',',':'),sort_keys=True).encode()
 return {'payload':base64.b64encode(raw).decode(),'signature':base64.b64encode(ISSUER.sign(raw)).decode(),'refresh_token':token}
@app.post('/admin/login')
def login(body:Login,request:Request):
 limit(request,'login',10)
 with database() as db:
  row=db.execute('SELECT * FROM admin WHERE username=?',(body.username,)).fetchone()
  salt=row['salt'] if row else '00'*16;candidate=password_hash(body.password,salt)
  if not row or not hmac.compare_digest(candidate,row['password_hash']):raise HTTPException(401,'Invalid administrator credentials')
  token=secrets.token_urlsafe(32);expiry=now()+3600
  db.execute('DELETE FROM sessions WHERE expires<=?',(now(),));db.execute('INSERT INTO sessions VALUES(?,?)',(digest(token),expiry))
 return {'token':token,'expires_at':expiry}
@app.post('/admin/licenses',dependencies=[Depends(admin)])
def issue(body:Issue):
 key='ts_'+secrets.token_urlsafe(32);identifier=secrets.token_hex(12);expiry=now()+body.days*86400
 with database() as db:db.execute('INSERT INTO licenses(id,key_hash,expires) VALUES(?,?,?)',(identifier,digest(key),expiry))
 return {'license_id':identifier,'license_key':key,'expires_at':expiry}
@app.get('/admin/licenses',dependencies=[Depends(admin)])
def licenses():
 with database() as db:rows=db.execute('SELECT id,expires,device,revoked FROM licenses ORDER BY rowid DESC LIMIT 1000').fetchall()
 return {'licenses':[dict(r) for r in rows]}
@app.post('/admin/licenses/{identifier}/revoke',dependencies=[Depends(admin)])
def revoke(identifier:str):
 with database() as db:
  c=db.execute('UPDATE licenses SET revoked=1 WHERE id=?',(identifier,))
  if not c.rowcount:raise HTTPException(404,'License not found')
 return {'revoked':True}
@app.post('/v1/challenge')
def challenge(body:Challenge,request:Request):
 limit(request,'challenge',30);nonce=secrets.token_urlsafe(32)
 with database() as db:
  db.execute('DELETE FROM challenges WHERE expires<=?',(now(),));db.execute('INSERT INTO challenges VALUES(?,?,?)',(digest(nonce),body.device_id,now()+120))
 return {'nonce':nonce,'expires_at':now()+120}
@app.post('/v1/activate')
def activate(body:Activate,request:Request):
 limit(request,'activate',30)
 with database() as db:
  db.execute('BEGIN IMMEDIATE')
  row=db.execute('SELECT * FROM licenses WHERE key_hash=?',(digest(body.license_key),)).fetchone()
  proof=db.execute('SELECT * FROM challenges WHERE nonce_hash=?',(digest(body.nonce),)).fetchone()
  if not row or row['revoked'] or row['expires']<=now():raise HTTPException(403,'License is invalid, expired, or revoked')
  if not proof or proof['expires']<=now() or proof['device']!=body.device_id:raise HTTPException(403,'Invalid or expired challenge')
  if row['device'] and row['device']!=body.device_id:raise HTTPException(403,'License is bound to another installation')
  db.execute('DELETE FROM challenges WHERE nonce_hash=?',(digest(body.nonce),));db.execute('UPDATE licenses SET device=? WHERE id=?',(body.device_id,row['id']))
  envelope=signed_lease(db,row,body.device_id,body.nonce)
 return envelope
@app.post('/v1/refresh')
def refresh(body:Refresh,request:Request):
 limit(request,'refresh',30)
 with database() as db:
  db.execute('BEGIN IMMEDIATE')
  stored=db.execute('SELECT * FROM refresh_tokens WHERE token_hash=?',(digest(body.refresh_token),)).fetchone()
  if not stored or stored['expires']<=now() or stored['device']!=body.device_id:raise HTTPException(403,'Invalid refresh token')
  row=db.execute('SELECT * FROM licenses WHERE id=?',(stored['license_id'],)).fetchone()
  proof=db.execute('SELECT * FROM challenges WHERE nonce_hash=?',(digest(body.nonce),)).fetchone()
  if not row or row['revoked'] or row['expires']<=now() or row['device']!=body.device_id:raise HTTPException(403,'License is invalid, expired, or revoked')
  if not proof or proof['expires']<=now() or proof['device']!=body.device_id:raise HTTPException(403,'Invalid or expired challenge')
  db.execute('DELETE FROM challenges WHERE nonce_hash=?',(digest(body.nonce),))
  envelope=signed_lease(db,row,body.device_id,body.nonce)
 return envelope
@app.get('/health')
def health():return {'status':'ok','product':'trollspeed-rebuild'}

from fastapi.responses import FileResponse
PAGE_DIR=Path(__file__).resolve().parent
PAGE_HEADERS={'Cache-Control':'no-store','Content-Security-Policy':"default-src 'none'; script-src 'self'; style-src 'unsafe-inline'; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'",'X-Content-Type-Options':'nosniff','Referrer-Policy':'no-referrer'}
@app.middleware('http')
async def no_cache(request,call_next):
 response=await call_next(request);response.headers['Cache-Control']='no-store';return response
@app.get('/')
def admin_page():return FileResponse(PAGE_DIR/'admin.html',media_type='text/html',headers=PAGE_HEADERS)
@app.get('/admin.js')
def admin_script():return FileResponse(PAGE_DIR/'admin.js',media_type='text/javascript',headers=PAGE_HEADERS)
