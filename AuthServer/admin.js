'use strict';
let sessionToken = '';
const byId = id => document.getElementById(id);
async function api(path, body) {
  const options = {headers: {'Content-Type':'application/json'}};
  if (sessionToken) options.headers.Authorization = 'Bearer ' + sessionToken;
  if (body !== undefined) { options.method = 'POST'; options.body = JSON.stringify(body); }
  const response = await fetch(path, options);
  const result = await response.json();
  if (!response.ok) throw new Error(typeof result.detail === 'string' ? result.detail : '请求失败：' + response.status);
  return result;
}
async function guarded(action) {
  byId('message').textContent = '';
  try { await action(); } catch (error) { byId('message').textContent = error.message; }
}
async function refresh() {
  const result = await api('/admin/licenses');
  byId('licenses').replaceChildren();
  for (const entry of result.licenses) {
    const row = document.createElement('tr');
    for (const value of [entry.id, new Date(entry.expires * 1000).toLocaleString(), entry.revoked ? '已撤销' : entry.device ? '已绑定' : '未绑定']) {
      const cell = document.createElement('td'); cell.textContent = value; row.append(cell);
    }
    const cell = document.createElement('td'), button = document.createElement('button');
    button.textContent = '撤销'; button.disabled = !!entry.revoked;
    button.addEventListener('click', () => guarded(async () => { await api('/admin/licenses/' + encodeURIComponent(entry.id) + '/revoke', {}); await refresh(); }));
    cell.append(button); row.append(cell); byId('licenses').append(row);
  }
}
byId('login').addEventListener('submit', event => { event.preventDefault(); guarded(async () => {
  const response = await api('/admin/login', {username:byId('username').value, password:byId('password').value});
  sessionToken = response.token; byId('password').value = ''; byId('controls').hidden = false; await refresh();
}); });
byId('issue').addEventListener('submit', event => { event.preventDefault(); guarded(async () => {
  const result = await api('/admin/licenses', {days:Number(byId('days').value)});
  byId('newkey').textContent = '请保存授权码（服务端不保存其明文）：\n' + result.license_key; await refresh();
}); });
byId('refresh').addEventListener('click', () => guarded(refresh));
