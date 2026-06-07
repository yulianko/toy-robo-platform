#pragma once

namespace WiFiConfigPageContent {
static constexpr const char CSS[] = R"css(
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

body {
  font-family: -apple-system, sans-serif;
  background: #f5f5f5;
  display: flex;
  justify-content: center;
  padding: 2rem 1rem;
  min-height: 100vh;
}

.card {
  background: #fff;
  border-radius: 12px;
  border: 1px solid #e5e5e5;
  padding: 1.5rem;
  width: 100%;
  max-width: 380px;
  height: fit-content;
}

h1 { font-size: 18px; font-weight: 600; margin-bottom: 4px; }

.sub { font-size: 13px; color: #666; margin-bottom: 1.25rem; }

.footer {
  font-size: 12px;
  color: #aaa;
  text-align: center;
  margin-top: 1rem;
}

.section-label {
  font-size: 11px;
  font-weight: 600;
  color: #999;
  text-transform: uppercase;
  letter-spacing: .08em;
  margin-bottom: 8px;
}

.net {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 9px 10px;
  border-radius: 8px;
  cursor: pointer;
  border: 1px solid transparent;
  margin-bottom: 6px;
  transition: background .15s;
}
.net:has(input:checked) { background: #eff6ff; border-color: #bfdbfe; }
.net input[type=radio]  { accent-color: #3b82f6; flex-shrink: 0; }

.net-name {
  font-size: 14px;
  font-weight: 500;
  flex: 1;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.net-meta { font-size: 12px; color: #888; }

.badge {
  font-size: 10px;
  padding: 2px 6px;
  border-radius: 4px;
  background: #dcfce7;
  color: #15803d;
  font-weight: 600;
  flex-shrink: 0;
}
.lock { font-size: 13px; color: #aaa; flex-shrink: 0; }

label.pw { display: block; font-size: 13px; color: #555; margin: 1rem 0 6px; }

.pw-wrap { position: relative; }
.pw-wrap input {
  width: 100%;
  padding: 8px 36px 8px 10px;
  border: 1px solid #d4d4d4;
  border-radius: 8px;
  font-size: 14px;
  outline: none;
}
.pw-wrap input:focus { border-color: #3b82f6; box-shadow: 0 0 0 3px #eff6ff; }
.pw-toggle {
  position: absolute;
  right: 8px;
  top: 50%;
  transform: translateY(-50%);
  background: none;
  border: none;
  cursor: pointer;
  color: #999;
  font-size: 16px;
}

.btn-row { display: flex; gap: 8px; margin-top: 1.25rem; }

.btn-rescan {
  flex-shrink: 0;
  padding: 9px 14px;
  border: 1px solid #d4d4d4;
  border-radius: 8px;
  background: none;
  font-size: 14px;
  cursor: pointer;
  color: #555;
}
.btn-rescan:disabled { opacity: .4; cursor: not-allowed; }

.btn-connect {
  flex: 1;
  padding: 9px;
  background: #111;
  color: #fff;
  border: none;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
}
.btn-connect:disabled { opacity: .35; cursor: not-allowed; }

#result {
  margin-top: 1rem;
  padding: 10px;
  border-radius: 8px;
  font-size: 13px;
  display: none;
}
#result.ok  { background: #dcfce7; color: #166534; }
#result.err { background: #fee2e2; color: #b91c1c; }

.spinner {
  display: inline-block;
  width: 14px;
  height: 14px;
  border: 2px solid #ccc;
  border-top-color: #555;
  border-radius: 50%;
  animation: spin .7s linear infinite;
  vertical-align: middle;
  margin-right: 4px;
}
@keyframes spin { to { transform: rotate(360deg); } }

.empty { font-size: 13px; color: #999; padding: 12px 0; text-align: center; }

.row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 0;
  border-bottom: 1px solid #f0f0f0;
  font-size: 14px;
}
.row:last-child { border-bottom: none; }

.label { color: #888; font-size: 13px; }
.value { font-weight: 500; }
)css";

static constexpr const char CONFIG_PAGE[] = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Wi-Fi Setup</title>
<link rel="stylesheet" href="/style.css">
</head>
<body>
<div class="card">
  <h1>&#x1F4F6; Wi-Fi setup</h1>
  <p class="sub">Select a network and enter the password to connect.</p>

  <p class="section-label">Available networks</p>
  <div id="net-list"><p class="empty"><span class="spinner"></span> Scanning...</p></div>

  <label class="pw" for="pw" id="pw-label">Password</label>
  <div class="pw-wrap">
    <input type="password" id="pw" placeholder="Enter password...">
    <button type="button" class="pw-toggle" onclick="togglePw()">&#x1F441;</button>
  </div>

  <div class="btn-row">
    <button class="btn-rescan" id="rescan-btn" onclick="startScan()">&#x21BB; Rescan</button>
    <button class="btn-connect" id="conn-btn" disabled onclick="doConnect()">Connect</button>
  </div>

  <div id="result"></div>
  <p class="footer">Connected to <strong>ESP32-Config</strong> &middot; 192.168.4.1</p>
</div>

<script>
  var selectedSsid   = null;
  var selectedIsOpen = false;
  var scanPollTimer  = null;

  async function startScan() {
    selectedSsid = null;
    document.getElementById('conn-btn').disabled = true;
    document.getElementById('rescan-btn').disabled = true;
    document.getElementById('net-list').innerHTML =
      '<p class="empty"><span class="spinner"></span> Scanning...</p>';
    clearTimeout(scanPollTimer);
    try { await fetch('/api/scan', { method: 'POST' }); } catch(e) {}
    pollScanResults();
  }

  async function pollScanResults() {
    try {
      var res  = await fetch('/api/scan');
      var data = await res.json();
      if (!data.ready) { scanPollTimer = setTimeout(pollScanResults, 800); return; }
      renderNetworks(data.networks);
    } catch(e) {
      document.getElementById('net-list').innerHTML =
        '<p class="empty">Scan failed - try again</p>';
    }
    document.getElementById('rescan-btn').disabled = false;
  }

  function renderNetworks(networks) {
    var list = document.getElementById('net-list');
    if (!networks || networks.length === 0) {
      list.innerHTML = '<p class="empty">No networks found</p>';
      return;
    }
    list.innerHTML = networks.map(function(n) {
      return '<label class="net">' +
        '<input type="radio" name="ssid" value="' + escHtml(n.ssid) + '"' +
        ' data-open="' + n.open + '" onchange="onNetworkSelect(this)">' +
        '<div style="flex:1;min-width:0;">' +
        '<div class="net-name">' + escHtml(n.ssid) + '</div>' +
        '<div class="net-meta">ch ' + n.channel + ' &middot; ' + n.rssi + ' dBm</div>' +
        '</div>' +
        (n.open ? '<span class="badge">open</span>' : '<span class="lock">&#x1F512;</span>') +
        '</label>';
    }).join('');
  }

  function onNetworkSelect(radio) {
    selectedSsid   = radio.value;
    selectedIsOpen = radio.dataset.open === 'true';
    var pwLabel = document.getElementById('pw-label');
    var pwWrap  = document.querySelector('.pw-wrap');
    pwLabel.style.display = selectedIsOpen ? 'none'  : 'block';
    pwWrap.style.display  = selectedIsOpen ? 'none'  : 'block';
    document.getElementById('conn-btn').disabled = false;
    document.getElementById('pw').value = '';
  }

  async function doConnect() {
    if (!selectedSsid) return;
    var password = document.getElementById('pw').value;
    var btn = document.getElementById('conn-btn');
    btn.innerHTML = '<span class="spinner"></span> Connecting...';
    btn.disabled = true;
    var body = new URLSearchParams({ ssid: selectedSsid, password: password });
    try {
      var res  = await fetch('/api/connect', { method: 'POST', body: body });
      var text = await res.text();
      showResult(res.ok, text);
    } catch(e) {
      showResult(false, 'Request failed - ' + e.message);
    }
    btn.textContent = 'Connect';
    btn.disabled = false;
  }

  function togglePw() {
    var pw = document.getElementById('pw');
    pw.type = pw.type === 'password' ? 'text' : 'password';
  }

  function showResult(ok, msg) {
    var el = document.getElementById('result');
    el.className = ok ? 'ok' : 'err';
    el.textContent = msg;
    el.style.display = 'block';
  }

  function escHtml(s) {
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;')
            .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
  }

  startScan();
</script>
</body>
</html>
)html";

static constexpr const char STATUS_PAGE_TEMPLATE[] = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="30">
<title>WiFi Status</title>
<link rel="stylesheet" href="/style.css">
</head>
<body>
<div class="card">
  <h1>&#x1F4F6; WiFi Status</h1>
  <div class="row">
    <span class="label">Status</span>
    <span class="value">Connected</span>
  </div>
  <div class="row">
    <span class="label">Network</span>
    <span class="value">%s</span>
  </div>
  <div class="row">
    <span class="label">IP Address</span>
    <span class="value">%s</span>
  </div>
</div>
</body>
</html>
)html";

}  // namespace WiFiConfigPageContent
