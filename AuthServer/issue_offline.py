"""Issue an owner-signed, installation-bound offline code; issuer state stays private."""
import argparse,base64,hashlib,json,secrets,sqlite3,time,uuid
from pathlib import Path
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey

def issue(state:Path,device:str,days:int,output:Path):
    uuid.UUID(device)  # Validate without changing case: the Keychain identifier is case-sensitive.
    if not 1<=days<=30:raise ValueError('Offline validity must be 1–30 days')
    if not (state/'auth.db').is_file():raise FileNotFoundError('Authorization database is missing')
    key=serialization.load_pem_private_key((state/'issuer.pem').read_bytes(),password=None)
    if not isinstance(key,Ed25519PrivateKey):raise ValueError('Issuer must be Ed25519')
    t=int(time.time());identifier=secrets.token_hex(12)
    payload={'version':1,'license_id':identifier,'device_id':device,'issued_at':t,'expires_at':t+days*86400,'nonce':'offline-'+secrets.token_urlsafe(24),'scope':'trollspeed-rebuild'}
    raw=json.dumps(payload,separators=(',',':'),sort_keys=True).encode()
    envelope={'payload':base64.b64encode(raw).decode(),'signature':base64.b64encode(key.sign(raw)).decode()}
    code='TSOFF1.'+base64.urlsafe_b64encode(json.dumps(envelope,separators=(',',':')).encode()).decode().rstrip('=')
    with sqlite3.connect(state/'auth.db') as db:
        db.execute('INSERT INTO licenses(id,key_hash,expires,device) VALUES(?,?,?,?)',(identifier,hashlib.sha256(('offline-'+secrets.token_hex(32)).encode()).hexdigest(),payload['expires_at'],device))
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(code+'\n');output.chmod(0o600)
    return payload
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--state',type=Path,required=True);parser.add_argument('--device',required=True);parser.add_argument('--days',type=int,default=7);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    try:payload=issue(args.state,args.device.strip(),args.days,args.output)
    except (ValueError,OSError,sqlite3.Error) as e:parser.exit(1,'Offline code was not issued: '+str(e)+'\n')
    print('Offline code saved; installation-bound, expires at '+str(payload['expires_at'])+'. Code and private key were not printed.')
