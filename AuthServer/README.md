# 自有授权系统

这是为重建产品新设计的授权服务，不查询、破解或接受原作者的密码。管理页面在服务根路径 `/`。

## 新建一套全新系统

```sh
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
python3 bootstrap.py --state ./state
TS_AUTH_STATE_DIR="$PWD/state" python3 -m uvicorn server:app --host 127.0.0.1 --port 8787
```

访问 http://127.0.0.1:8787 。管理员账号为 admin，随机密码在 state/ADMIN-CREDENTIALS.txt。bootstrap不会覆盖已有状态。新建实例后将 public-config.json 的公钥同步到客户端，不能直接沿用另一套实例的公钥。

## 使用本次已生成的账号和签发密钥

本次部署状态单独保存在 TrollSpeed-auth-private-state.zip，不包含在源码ZIP。把其中 state 目录放到服务器私有位置，然后将 TS_AUTH_STATE_DIR 指向这个目录。数据库中的密码经过加盐PBKDF2处理；issuer.pem是服务端Ed25519私钥。客户端AuthClient/public-config.json与本次私钥匹配。

源码包可进入私有开发仓库；部署状态包、ADMIN-CREDENTIALS.txt、issuer.pem和数据库不得进入公开仓库或IPA。不要把管理员密码硬编码到客户端。

当前没有部署到公网：设备端 LicenseClient 强制HTTPS。正式使用前需要自有服务器/域名、TLS反向代理与持久化状态目录；未提供可从iPhone访问的服务地址。反向代理应增加速率限制、备份和运维监控。进程内限速器只适用于单进程基础保护，多worker应增加共享限速层。

## 使用

管理员在网页登录后生成授权码。授权码首次激活绑定客户端Keychain安装标识；客户端先取得一次性challenge，再激活得到10分钟以内有效的Ed25519签名结果，HUD 在到期前自动向服务器验证并轮换续期令牌。撤销可阻止新授权，已经签出的授权最长仍可使用10分钟。

这不是硬件身份认证或App Attest。用户若修改客户端代码、复制安装标识、调整本机时钟或绕过本地调用点，不能靠这套纯软件方案承诺绝对防破解。新 App 已在 HUD 启动入口和读取循环校验签名租约；设备端读写权限与实际运行仍须实机检查。

## 接口

- POST /admin/login：用户名和密码，返回1小时会话。
- POST /admin/licenses：管理员Bearer会话，发放指定天数的授权码，只在创建时返回明文。
- GET /admin/licenses：查看最近1000条授权。
- POST /admin/licenses/{id}/revoke：撤销授权。
- POST /v1/challenge：device_id，返回短期一次性nonce。
- POST /v1/activate：device_id、license_key、nonce；返回原始载荷base64、Ed25519签名base64与续期令牌。
- POST /v1/refresh：device_id、refresh_token、nonce；校验撤销状态并轮换令牌，旧令牌立即失效。

验证：`python3 selfcheck.py`，独立临时数据库和真实本地HTTP服务，覆盖错误密码、未登录发码、签名、篡改、重放、跨设备使用、撤销和过期。

## Docker 与 HTTPS

已提供 Dockerfile、compose.yaml 和 Caddyfile。先把 Caddyfile 中的域名改为你的域名，将已有私有状态放到 AuthServer/state，然后运行 `docker compose up -d --build`。应用中填写该 HTTPS 域名。不要对已有状态重新运行 bootstrap；服务会自动创建续期令牌表，保留已有管理员与签发密钥。
