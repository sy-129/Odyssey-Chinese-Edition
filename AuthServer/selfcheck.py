"""Real local HTTP integration checks, isolated from the product issuer state."""
import base64,json,os,pathlib,socket,sqlite3,subprocess,sys,tempfile,time,urllib.request,urllib.error
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from cryptography.exceptions import InvalidSignature
root=pathlib.Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='trollspeed-auth-check-') as temporary:
 state=pathlib.Path(temporary);subprocess.run([sys.executable,str(root/'bootstrap.py'),'--state',str(state)],check=True,stdout=subprocess.DEVNULL)
 password=(state/'ADMIN-CREDENTIALS.txt').read_text().split('Password: ',1)[1].splitlines()[0]
 with socket.socket() as sock:sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
 env=dict(os.environ,TS_AUTH_STATE_DIR=str(state));proc=subprocess.Popen([sys.executable,'-m','uvicorn','server:app','--host','127.0.0.1','--port',str(port),'--no-access-log'],cwd=root,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 checks=0
 def check(condition):
  global checks
  checks+=1
  if not condition:raise AssertionError('Authorization check failed, number '+str(checks))
 def request(path,body=None,token=None):
  headers={'Content-Type':'application/json'}
  if token:headers['Authorization']='Bearer '+token
  req=urllib.request.Request('http://127.0.0.1:'+str(port)+path,data=json.dumps(body).encode() if body is not None else None,headers=headers)
  try:
   with urllib.request.urlopen(req,timeout=10) as r:return r.status,json.load(r)
  except urllib.error.HTTPError as e:return e.code,json.load(e)
 try:
  for _ in range(100):
   if proc.poll() is not None:raise RuntimeError('Server did not start')
   try:
    if request('/health')[0]==200:break
   except OSError:time.sleep(.05)
  else:raise RuntimeError('Server startup timed out')
  check(request('/admin/licenses',{'days':30})[0]==401)
  check(request('/admin/login',{'username':'admin','password':'incorrect'})[0]==401)
  status,response=request('/admin/login',{'username':'admin','password':password});check(status==200);token=response['token']
  status,issued=request('/admin/licenses',{'days':30},token);check(status==200)
  device='11111111-2222-3333-4444-555555555555';other='aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee'
  nonce=request('/v1/challenge',{'device_id':device})[1]['nonce'];body={'device_id':device,'license_key':issued['license_key'],'nonce':nonce}
  status,envelope=request('/v1/activate',body);check(status==200)
  public=Ed25519PublicKey.from_public_bytes(base64.b64decode(json.loads((state/'public-config.json').read_text())['public_key_base64']))
  raw=base64.b64decode(envelope['payload']);signature=base64.b64decode(envelope['signature']);public.verify(signature,raw);check(json.loads(raw)['device_id']==device)
  try:public.verify(signature,raw+b' ');check(False)
  except InvalidSignature:check(True)
  check(request('/v1/activate',body)[0]==403)
  another_nonce=request('/v1/challenge',{'device_id':other})[1]['nonce'];check(request('/v1/activate',{'device_id':other,'license_key':issued['license_key'],'nonce':another_nonce})[0]==403)
  check(request('/admin/licenses/'+issued['license_id']+'/revoke',{},token)[0]==200)
  nonce=request('/v1/challenge',{'device_id':device})[1]['nonce'];check(request('/v1/activate',dict(body,nonce=nonce))[0]==403)
  check(request('/admin/licenses',token=token)[1]['licenses'][0]['revoked']==1)
  status,expired=request('/admin/licenses',{'days':1},token);check(status==200)
  with sqlite3.connect(state/'auth.db') as db:db.execute('UPDATE licenses SET expires=0 WHERE id=?',(expired['license_id'],))
  nonce=request('/v1/challenge',{'device_id':device})[1]['nonce'];check(request('/v1/activate',{'device_id':device,'license_key':expired['license_key'],'nonce':nonce})[0]==403)
  with sqlite3.connect(state/'auth.db') as db:db.execute('UPDATE sessions SET expires=0')
  check(request('/admin/licenses',token=token)[0]==401)
  print(str(checks)+' authorization checks passed; real local HTTP, signature, replay, binding, expiry and revocation verified.')
 finally:
  proc.terminate()
  try:proc.wait(5)
  except subprocess.TimeoutExpired:proc.kill();proc.wait()
