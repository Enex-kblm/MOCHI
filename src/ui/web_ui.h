#pragma once

static const char MOCHI_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Mochi ✦</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700;800;900&family=Space+Mono:wght@400;700&display=swap" rel="stylesheet">
<style>
:root {
  --sky:         #eef0fa;
  --lilac:       #e2d9f5;
  --blush:       #fce4ec;
  --mint:        #e0f5ee;
  --cream:       #faf7f2;
  --ink:         #2d2b4e;
  --muted:       #9190b0;
  --border:      rgba(180,170,230,.35);
  --glass:       rgba(255,255,255,.75);
  --accent:      #b49be8;
  --accentDark:  #8c6fd4;
  --accentPink:  #e884a8;
  --accentBlue:  #7ab8e8;
  --success:     #6ec99c;
  --warn:        #f5c542;
  --danger:      #e87c8a;
  --shadow:      0 8px 32px rgba(120,100,200,.13);
  --shadowSm:    0 2px 12px rgba(120,100,200,.09);
}
* { box-sizing: border-box; margin: 0; padding: 0; }
*:focus, *:active { -webkit-tap-highlight-color: transparent; outline: none; -webkit-user-select: none; user-select: none; }
input, textarea { -webkit-user-select: text; user-select: text; }
html { scroll-behavior: smooth; }
body {
  font-family: 'Nunito', sans-serif;
  background: linear-gradient(145deg, #f0edfb 0%, #fce8f3 40%, #e8f3fc 100%);
  color: var(--ink); min-height: 100vh; overflow-x: hidden;
}
.bg-blobs { position: fixed; inset: 0; pointer-events: none; z-index: 0; overflow: hidden; }
.bg-blobs span {
  position: absolute; border-radius: 50%; filter: blur(70px); opacity: .28;
  animation: bfloat 9s ease-in-out infinite;
}
.bg-blobs span:nth-child(1) { width:320px; height:320px; background:#c9b8f0; top:-80px; left:-80px; animation-delay:0s; }
.bg-blobs span:nth-child(2) { width:240px; height:240px; background:#f5b8d0; bottom:60px; right:-60px; animation-delay:3.5s; }
.bg-blobs span:nth-child(3) { width:200px; height:200px; background:#a8d8f0; top:45%; left:55%; animation-delay:6s; }
@keyframes bfloat { 0%,100%{transform:translateY(0) scale(1)} 50%{transform:translateY(-22px) scale(1.05)} }

.wrap { position:relative; z-index:1; max-width:460px; margin:0 auto; padding:0 16px 70px; }

header { position:relative; z-index:1; padding:38px 20px 24px; text-align:center; }
.header-pill {
  display:inline-flex; align-items:center; gap:7px;
  background:rgba(255,255,255,.82); border:1.5px solid var(--border);
  border-radius:100px; padding:5px 16px 5px 10px;
  font-size:.68rem; font-weight:800; color:var(--accentDark);
  letter-spacing:.07em; text-transform:uppercase;
  margin-bottom:14px; backdrop-filter:blur(12px); box-shadow:var(--shadowSm);
}
.dot-live { width:7px; height:7px; background:var(--success); border-radius:50%;
  box-shadow:0 0 0 3px rgba(110,201,156,.25); animation:pulse 2s infinite; }
@keyframes pulse { 0%,100%{box-shadow:0 0 0 3px rgba(110,201,156,.25)} 50%{box-shadow:0 0 0 7px rgba(110,201,156,.06)} }
header h1 { font-size:2.1rem; font-weight:900; color:var(--ink); letter-spacing:-.02em; line-height:1.1; }
header h1 em { font-style:normal; color:var(--accentDark); }
header p { margin-top:5px; font-size:.82rem; color:var(--muted); font-weight:600; }

.tab-nav {
  display:flex; gap:5px;
  background:rgba(255,255,255,.78); border:1.5px solid var(--border);
  border-radius:20px; padding:5px; margin-bottom:18px;
  backdrop-filter:blur(16px); box-shadow:var(--shadowSm); flex-wrap:wrap;
}
.tab-btn {
  flex:1; min-width:56px; padding:9px 6px;
  background:transparent; border:none; border-radius:14px;
  font-family:'Nunito',sans-serif; font-size:.75rem; font-weight:800;
  color:var(--muted); cursor:pointer;
  transition:all .22s cubic-bezier(.34,1.56,.64,1);
  display:flex; flex-direction:column; align-items:center; gap:3px;
}
.tab-icon { font-size:1.15rem; }
.tab-btn:hover { color:var(--ink); background:rgba(255,255,255,.55); }
.tab-btn.active { background:#fff; color:var(--ink); box-shadow:0 3px 14px rgba(120,100,200,.18); }

.panel { display:none; animation:fadeUp .28s ease; }
.panel.active { display:block; }
@keyframes fadeUp { from{opacity:0;transform:translateY(10px)} to{opacity:1;transform:translateY(0)} }

.card {
  background:var(--glass); border:1.5px solid var(--border);
  border-radius:24px; padding:22px 20px 20px; margin-bottom:14px;
  backdrop-filter:blur(20px); box-shadow:var(--shadow);
  transition:box-shadow .2s;
}
.card:hover { box-shadow:0 14px 44px rgba(120,100,200,.18); }
.card-head { display:flex; align-items:center; gap:11px; margin-bottom:18px; }
.card-icon { width:40px; height:40px; border-radius:13px; display:flex; align-items:center; justify-content:center; font-size:1.2rem; flex-shrink:0; }
.ic-purple { background:rgba(180,155,232,.2); }
.ic-pink   { background:rgba(232,132,168,.18); }
.ic-blue   { background:rgba(122,184,232,.18); }
.ic-green  { background:rgba(110,201,156,.18); }
.ic-yellow { background:rgba(245,197,66,.18); }
.card-title { font-size:.9rem; font-weight:800; color:var(--ink); }
.card-sub   { font-size:.72rem; color:var(--muted); font-weight:600; margin-top:2px; }

.field { margin-bottom:14px; }
label.lbl { display:block; font-size:.72rem; font-weight:800; color:var(--muted); letter-spacing:.06em; text-transform:uppercase; margin-bottom:7px; }
input[type=text], input[type=password] {
  width:100%; padding:11px 15px;
  background:rgba(255,255,255,.72); border:1.5px solid var(--border);
  border-radius:14px; font-family:'Nunito',sans-serif;
  font-size:.92rem; font-weight:600; color:var(--ink);
  transition:border-color .2s, box-shadow .2s, background .2s;
}
input:focus { border-color:var(--accent); background:#fff; box-shadow:0 0 0 3px rgba(180,155,232,.14); }

.clock-box {
  background:linear-gradient(135deg, rgba(180,155,232,.14) 0%, rgba(122,184,232,.1) 100%);
  border:1.5px solid rgba(180,155,232,.28); border-radius:18px;
  padding:18px; text-align:center; margin-bottom:18px;
}
.clock-time { font-family:'Space Mono',monospace; font-size:2.5rem; font-weight:700; color:var(--ink); letter-spacing:.05em; line-height:1; }
.clock-date { font-size:.8rem; font-weight:700; color:var(--muted); margin-top:6px; letter-spacing:.03em; text-transform:uppercase; }
.sync-badge {
  display:inline-flex; align-items:center; gap:5px; margin-top:10px;
  font-size:.7rem; font-weight:700; border-radius:100px; padding:3px 11px;
  color:var(--success); background:rgba(110,201,156,.12);
  border:1px solid rgba(110,201,156,.25);
}

.btn {
  display:flex; align-items:center; justify-content:center; gap:7px;
  width:100%; padding:13px 16px; border:none; border-radius:16px;
  font-family:'Nunito',sans-serif; font-size:.9rem; font-weight:800;
  cursor:pointer; transition:all .18s cubic-bezier(.34,1.56,.64,1);
}
.btn:active { transform:scale(.96); }
.btn:disabled { opacity:.45; cursor:not-allowed; transform:none!important; }
.btn-primary { background:linear-gradient(135deg,var(--accentDark) 0%,var(--accent) 100%); color:#fff; box-shadow:0 4px 18px rgba(140,111,212,.32); }
.btn-primary:hover:not(:disabled) { box-shadow:0 6px 24px rgba(140,111,212,.42); transform:translateY(-1px); }
.btn-danger { background:linear-gradient(135deg,#e87c8a 0%,#f0a0b0 100%); color:#fff; box-shadow:0 4px 18px rgba(232,124,138,.28); }
.btn-ghost { background:rgba(255,255,255,.65); border:1.5px solid var(--border); color:var(--muted); }
.btn-ghost:hover { background:#fff; color:var(--ink); }
.btn-row { display:flex; gap:8px; }
.btn-row .btn { flex:1; }
.btn-sm { padding:9px 14px; font-size:.82rem; border-radius:12px; }

.divider { display:flex; align-items:center; gap:10px; margin:16px 0; color:var(--muted); font-size:.68rem; font-weight:800; letter-spacing:.08em; text-transform:uppercase; }
.divider::before,.divider::after { content:''; flex:1; height:1px; background:var(--border); }

.wp-wrap { position:relative; width:100%; aspect-ratio:240/177; border-radius:18px; overflow:hidden; background:rgba(200,190,240,.12); border:2px dashed var(--border); margin-bottom:12px; cursor:pointer; transition:border-color .2s; }
.wp-wrap:hover { border-color:var(--accent); }
.wp-wrap.has-img { border-style:solid; border-color:rgba(180,155,232,.35); cursor:grab; }
.wp-wrap.has-img:active { cursor:grabbing; }
.wp-placeholder { position:absolute; inset:0; display:flex; flex-direction:column; align-items:center; justify-content:center; gap:8px; color:var(--muted); }
.ph-icon { font-size:2.4rem; opacity:.55; }
.ph-text { font-size:.84rem; font-weight:800; }
.ph-sub  { font-size:.7rem; font-weight:600; opacity:.7; }
#cropCanvas { display:none; width:100%; height:100%; object-fit:cover; }
#cropCanvas.active { display:block; }
.canvas-overlay { position:absolute; top:8px; right:8px; display:none; gap:5px; }
.wp-wrap.has-img .canvas-overlay { display:flex; }
.ovr-badge { background:rgba(0,0,0,.42); backdrop-filter:blur(8px); color:#fff; font-size:.62rem; font-weight:800; padding:3px 9px; border-radius:100px; text-transform:uppercase; letter-spacing:.04em; }

.slider-bar { display:none; align-items:center; gap:10px; margin-bottom:14px; padding:10px 14px; background:rgba(255,255,255,.55); border:1.5px solid var(--border); border-radius:14px; }
.slider-bar.active { display:flex; }
.slider-label { font-size:.72rem; font-weight:800; color:var(--muted); white-space:nowrap; letter-spacing:.04em; text-transform:uppercase; }
.slider-val { font-size:.75rem; font-weight:800; color:var(--ink); white-space:nowrap; min-width:38px; text-align:right; }
input[type=range] { flex:1; -webkit-appearance:none; height:5px; background:linear-gradient(90deg,var(--accentDark) 0%,var(--accent) 100%); border-radius:4px; }
input[type=range]::-webkit-slider-thumb { -webkit-appearance:none; width:18px; height:18px; border-radius:50%; background:#fff; border:2px solid var(--accent); box-shadow:0 2px 8px rgba(140,111,212,.28); cursor:pointer; transition:transform .15s; }
input[type=range]::-webkit-slider-thumb:active { transform:scale(1.2); }

.color-section { margin-bottom:18px; }
.color-section-title {
  font-size:.72rem; font-weight:800; color:var(--muted);
  text-transform:uppercase; letter-spacing:.07em; margin-bottom:10px;
  display:flex; align-items:center; gap:6px;
}
.color-section-title::after { content:''; flex:1; height:1px; background:var(--border); }

.theme-preview {
  display:flex; align-items:center; margin-bottom:16px;
  border-radius:18px; overflow:hidden; border:1.5px solid rgba(0,0,0,.12);
}
.theme-preview-inner {
  flex:1; background:#000; padding:14px 18px;
  display:flex; flex-direction:column; align-items:center;
}
.preview-time { font-family:'Space Mono',monospace; font-size:1.7rem; font-weight:700; letter-spacing:.06em; transition:color .2s; }
.preview-date { font-size:.72rem; font-weight:700; letter-spacing:.04em; text-transform:uppercase; margin-top:3px; transition:color .2s; }
.theme-preview-right {
  width:80px; background:linear-gradient(135deg,rgba(180,155,232,.18) 0%,rgba(122,184,232,.12) 100%);
  border-left:1.5px solid rgba(0,0,0,.08); padding:10px;
  display:flex; flex-direction:column; align-items:center; justify-content:center; gap:6px;
}
.mini-swatch-label { font-size:.55rem; font-weight:800; color:var(--muted); text-transform:uppercase; letter-spacing:.05em; }
.mini-swatch { width:28px; height:28px; border-radius:50%; border:2px solid rgba(255,255,255,.7); box-shadow:0 2px 8px rgba(0,0,0,.12); }

.swatch-row { display:flex; gap:7px; flex-wrap:wrap; margin-bottom:12px; }
.swatch { width:30px; height:30px; border-radius:50%; cursor:pointer; border:3px solid transparent; transition:transform .15s, border-color .15s; box-shadow:0 2px 6px rgba(0,0,0,.1); }
.swatch:hover { transform:scale(1.15); }
.swatch.sel { border-color:var(--ink); }
.color-picker-row { display:flex; align-items:center; gap:8px; }
.color-picker-row input[type=color] { -webkit-appearance:none; border:none; width:42px; height:42px; border-radius:12px; cursor:pointer; padding:0; background:none; flex-shrink:0; }
.color-picker-row input[type=color]::-webkit-color-swatch-wrapper { padding:0; border-radius:12px; }
.color-picker-row input[type=color]::-webkit-color-swatch { border:none; border-radius:10px; }
.hex-input { flex:1; padding:10px 13px; background:rgba(255,255,255,.72); border:1.5px solid var(--border); border-radius:12px; font-family:'Space Mono',monospace; font-size:.88rem; font-weight:700; color:var(--ink); }

.info-row { display:flex; justify-content:space-between; align-items:center; font-size:.82rem; }
.info-tag { font-family:'Space Mono',monospace; font-weight:700; font-size:.78rem; background:rgba(122,184,232,.12); padding:3px 10px; border-radius:8px; }

.restart-box { display:none; background:linear-gradient(135deg,rgba(245,197,66,.18) 0%,rgba(255,180,80,.1) 100%); border:1.5px solid rgba(245,197,66,.4); border-radius:14px; padding:14px; margin-top:12px; text-align:center; }
.restart-box.show { display:block; animation:fadeUp .25s ease; }
.restart-box .r-title { font-size:.82rem; font-weight:800; color:#8a6010; }
.restart-box .r-count { font-family:'Space Mono',monospace; font-size:1.5rem; font-weight:700; color:#b07820; margin:4px 0 2px; }
.restart-box .r-sub { font-size:.72rem; color:#9a7030; font-weight:600; }

.toggle-row { display:flex; align-items:center; justify-content:space-between; margin-bottom:16px; }
.toggle-label { font-size:.86rem; font-weight:800; color:var(--ink); }
.toggle-sublabel { font-size:.7rem; font-weight:600; color:var(--muted); margin-top:2px; }
.toggle-track { width:48px; height:26px; border-radius:24px; background:#ddd; position:relative; cursor:pointer; transition:background .25s; flex-shrink:0; }
.toggle-thumb { position:absolute; top:3px; left:3px; width:20px; height:20px; border-radius:50%; background:#fff; box-shadow:0 1px 4px rgba(0,0,0,.2); transition:transform .25s; }

/* ── Toast ── */
.toast-wrap { position:fixed; bottom:24px; left:50%; transform:translateX(-50%); z-index:999; display:flex; flex-direction:column; align-items:center; gap:8px; pointer-events:none; }
.toast {
  display:inline-flex; align-items:center; gap:8px;
  background:rgba(30,26,60,.92); color:#fff;
  font-size:.82rem; font-weight:700; padding:11px 20px;
  border-radius:100px; white-space:nowrap;
  animation:toastIn .32s cubic-bezier(.34,1.56,.64,1) forwards;
  box-shadow:0 4px 24px rgba(0,0,0,.22);
  backdrop-filter:blur(12px);
}
.toast.ok   { border-left:3px solid var(--success); }
.toast.err  { border-left:3px solid var(--danger); background:rgba(60,20,30,.92); }
.toast.info { border-left:3px solid var(--accentBlue); }
.toast.warn { border-left:3px solid var(--warn); }
.toast.leave { animation:toastOut .28s ease forwards; }
@keyframes toastIn  { from{opacity:0;transform:translateY(14px) scale(.94)} to{opacity:1;transform:translateY(0) scale(1)} }
@keyframes toastOut { from{opacity:1;transform:translateY(0) scale(1)} to{opacity:0;transform:translateY(-10px) scale(.94)} }

</style>
</head>
<body>

<div class="bg-blobs"><span></span><span></span><span></span></div>

<header>
  <div class="header-pill"><span class="dot-live"></span>Perangkat Terhubung</div>
  <h1>✦ Mo<em>chi</em> ✦</h1>
  <p>Panel Konfigurasi · Firmware v2.3</p>
</header>

<div class="wrap">
  <div class="tab-nav">
    <button class="tab-btn active" onclick="switchTab('clock')"><span class="tab-icon">🕐</span>Jam</button>
    <button class="tab-btn" onclick="switchTab('wallpaper')"><span class="tab-icon">🖼</span>Wallpaper</button>
    <button class="tab-btn" onclick="switchTab('display')"><span class="tab-icon">🎨</span>Tampilan</button>
    <button class="tab-btn" onclick="switchTab('wifi')"><span class="tab-icon">📶</span>WiFi AP</button>
    <button class="tab-btn" onclick="switchTab('greeting')"><span class="tab-icon">💌</span>Sapaan</button>
  </div>

  <!-- TAB JAM -->
  <div class="panel active" id="panel-clock">
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-purple">🕐</div>
        <div>
          <div class="card-title">Jam Perangkat</div>
          <div class="card-sub">Waktu yang berjalan di Mochi kamu</div>
        </div>
      </div>
      <div class="clock-box">
        <div class="clock-time" id="liveTime">--:--:--</div>
        <div class="clock-date" id="liveDate">-- --- ----</div>
        <div><span class="sync-badge" id="syncBadge">⏳ Belum disinkron</span></div>
      </div>
      <div class="divider">Sinkron dari browser kamu</div>
      <button class="btn btn-primary" onclick="syncTime()"><span>⚡</span> Sinkron Sekarang</button>
    </div>
  </div>

  <!-- TAB WALLPAPER -->
  <div class="panel" id="panel-wallpaper">
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-pink">🌸</div>
        <div>
          <div class="card-title">Foto Wallpaper</div>
          <div class="card-sub">Foto yang tampil di belakang jam · 240×177 px</div>
        </div>
      </div>
      <div class="wp-wrap" id="cropWrap" onclick="onCanvasClick()">
        <div class="wp-placeholder" id="wpPlaceholder">
          <span class="ph-icon">🖼</span>
          <span class="ph-text">Ketuk untuk pilih foto</span>
          <span class="ph-sub">Geser · Cubit untuk zoom</span>
        </div>
        <canvas id="cropCanvas"></canvas>
        <div class="canvas-overlay"><span class="ovr-badge">geser untuk reframe</span><span class="ovr-badge" id="currentWpBadge" style="display:none;background:rgba(140,111,212,.75);">✦ wallpaper saat ini</span></div>
      </div>
      <div class="slider-bar" id="zoomBar">
        <span class="slider-label">🔍 Zoom</span>
        <input type="range" id="zoomSlider" min="1" max="5" step="any" value="1">
      </div>
      <input type="file" id="fileInput" accept="image/*" style="display:none">
      <div class="btn-row">
        <button class="btn btn-ghost btn-sm" onclick="document.getElementById('fileInput').click()">📷 Ganti Foto</button>
        <button class="btn btn-primary btn-sm" id="uploadBtn" disabled onclick="uploadWallpaper()">⬆ Upload</button>
      </div>
    </div>
  </div>

  <!-- TAB TAMPILAN -->
  <div class="panel" id="panel-display">
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-yellow">☀️</div>
        <div>
          <div class="card-title">Kecerahan Layar</div>
          <div class="card-sub">Atur terang-gelapnya layar Mochi</div>
        </div>
      </div>
      <div class="slider-bar active" style="margin-bottom:18px;">
        <span class="slider-label">🔆</span>
        <input type="range" id="brightnessSlider" min="0" max="100" step="1" value="78" oninput="onBrightnessInput(this.value)">
        <span class="slider-val" id="brightnessVal">78%</span>
      </div>
      <button class="btn btn-primary" onclick="saveBrightness()">💾 Simpan Kecerahan</button>
    </div>

    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-purple">🎨</div>
        <div>
          <div class="card-title">Warna Teks Jam</div>
          <div class="card-sub">Atur warna jam dan tanggal (mode digital)</div>
        </div>
      </div>
      <div class="theme-preview">
        <div class="theme-preview-inner">
          <div class="preview-time" id="previewTime">12:34:56</div>
          <div class="preview-date" id="previewDate">SAB, 17 MEI 2025</div>
        </div>
        <div class="theme-preview-right">
          <div><div class="mini-swatch-label">Jam</div><div class="mini-swatch" id="miniSwatchClock" style="background:#fff;"></div></div>
          <div><div class="mini-swatch-label">Tanggal</div><div class="mini-swatch" id="miniSwatchDate" style="background:#aaaaaa;"></div></div>
        </div>
      </div>
      <div class="color-section">
        <div class="color-section-title">🕐 Warna Angka Jam</div>
        <div class="swatch-row" id="clockSwatches"></div>
        <div class="color-picker-row">
          <input type="color" id="clockColorPicker" value="#ffffff" oninput="onClockColorInput(this.value)">
          <input type="text" class="hex-input" id="clockColorHex" placeholder="#FFFFFF" maxlength="7" oninput="onClockHexInput(this.value)">
        </div>
      </div>
      <div class="color-section">
        <div class="color-section-title">📅 Warna Teks Tanggal</div>
        <div class="swatch-row" id="dateSwatches"></div>
        <div class="color-picker-row">
          <input type="color" id="dateColorPicker" value="#aaaaaa" oninput="onDateColorInput(this.value)">
          <input type="text" class="hex-input" id="dateColorHex" placeholder="#AAAAAA" maxlength="7" oninput="onDateHexInput(this.value)">
        </div>
      </div>
      <button class="btn btn-primary" onclick="saveTheme()">🎨 Simpan Warna</button>
    </div>
  </div>

  <!-- TAB WIFI AP -->
  <div class="panel" id="panel-wifi">
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-blue">📶</div>
        <div>
          <div class="card-title">Hotspot Perangkat</div>
          <div class="card-sub">Nama & password WiFi Mochi</div>
        </div>
      </div>
      <div class="field">
        <label class="lbl">📡 Nama Hotspot (SSID)</label>
        <input type="text" id="apSsid" placeholder="cth: Mochi" maxlength="32">
      </div>
      <div class="field">
        <label class="lbl">🔑 Password</label>
        <input type="password" id="apPass" placeholder="Minimal 8 karakter" maxlength="64">
      </div>
      <button class="btn btn-danger" onclick="saveAP()">💾 Simpan & Restart</button>
      <div class="restart-box" id="restartNotice">
        <div class="r-title">Perangkat sedang restart…</div>
        <div class="r-count" id="countdown">5</div>
        <div class="r-sub">Sambungkan kembali ke WiFi dengan nama baru</div>
      </div>
    </div>
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-green">ℹ️</div>
        <div><div class="card-title">Info Koneksi Saat Ini</div></div>
      </div>
      <div style="display:flex;flex-direction:column;gap:10px;">
        <div class="info-row"><span style="color:var(--muted);font-weight:700;">Alamat IP</span><span class="info-tag">192.168.4.1</span></div>
        <div class="info-row"><span style="color:var(--muted);font-weight:700;">Port</span><span class="info-tag">80</span></div>
        <div class="info-row"><span style="color:var(--muted);font-weight:700;">Mode</span><span style="font-weight:800;color:var(--success);">● Soft AP</span></div>
      </div>
    </div>
  </div>

  <!-- TAB SAPAAN -->
  <div class="panel" id="panel-greeting">
    <div class="card">
      <div class="card-head">
        <div class="card-icon ic-pink">💌</div>
        <div><div class="card-title">Pesan Sapaan</div><div class="card-sub">Muncul saat Mochi baru dinyalakan</div></div>
      </div>
      <div class="toggle-row">
        <div><div class="toggle-label">Aktifkan sapaan</div><div class="toggle-sublabel">Tampilkan pesan saat menyala</div></div>
        <div class="toggle-track" id="toggleTrack" onclick="toggleGreeting()"><div class="toggle-thumb" id="toggleThumb"></div></div>
      </div>
      <div class="field">
        <label class="lbl">💬 Tulis sapaan</label>
        <input type="text" id="greetMsg" maxlength="48" placeholder="contoh: halo juli">
        <div style="font-size:.7rem;color:var(--muted);margin-top:6px;">Maks 48 karakter · muncul di layar saat Mochi dinyalakan</div>
      </div>
      <div style="background:rgba(180,155,232,.1);border-radius:14px;padding:12px 14px;margin-bottom:14px;">
        <div style="font-size:.68rem;font-weight:800;color:var(--accentDark);">✦ Preview</div>
        <div style="font-size:1rem;font-weight:700;color:var(--ink);" id="greetPreview">—</div>
      </div>
      <button class="btn btn-primary" onclick="saveGreeting()">💾 Simpan Sapaan</button>
    </div>
  </div>
</div>

<div class="toast-wrap" id="toastWrap"></div>

<script>
// ── Tab ──────────────────────────────────────────────────────────
function switchTab(name) {
  ['clock','wallpaper','display','wifi','greeting'].forEach((p,i) => {
    document.getElementById('panel-'+p).classList.toggle('active', p===name);
    document.querySelectorAll('.tab-btn')[i].classList.toggle('active', p===name);
  });
}

// ── Live clock ───────────────────────────────────────────────────
let synced = false;
function updateLiveClock() {
  const d = new Date();
  document.getElementById('liveTime').textContent = d.toLocaleTimeString('id-ID');
  document.getElementById('liveDate').textContent = d.toLocaleDateString('id-ID',
    {weekday:'short',day:'2-digit',month:'short',year:'numeric'}).replace(/\./g,'');
  document.getElementById('syncBadge').innerHTML = synced ? '✓ Berhasil disinkron' : '⏳ Belum disinkron';
}
updateLiveClock(); setInterval(updateLiveClock, 1000);

async function syncTime() {
  const n = new Date();
  const q = new URLSearchParams({y:n.getFullYear(),mo:n.getMonth()+1,d:n.getDate(),h:n.getHours(),mi:n.getMinutes(),s:n.getSeconds()});
  try {
    const r = await fetch('/settime?' + q);
    if (r.ok) { synced = true; toast('⚡ Jam berhasil disinkron!'); }
    else toast('❌ Gagal sinkron waktu', 'err');
  } catch { toast('❌ Tidak bisa terhubung', 'err'); }
}

// ── Wallpaper crop ───────────────────────────────────────────────
const CROP_W=240, CROP_H=177;
const cropWrap=document.getElementById('cropWrap');
const canvas=document.getElementById('cropCanvas');
const zoomBar=document.getElementById('zoomBar');
const zoomSlider=document.getElementById('zoomSlider');
const uploadBtn=document.getElementById('uploadBtn');
const fileInput=document.getElementById('fileInput');
const placeholder=document.getElementById('wpPlaceholder');
let ctx=null,img=null,scale=1,ox=0,oy=0,dragging=false,lastX=0,lastY=0;

function onCanvasClick() { if (!img) fileInput.click(); }
fileInput.addEventListener('change', e => {
  const file = e.target.files[0]; if (!file) return;
  const reader = new FileReader();
  reader.onload = ev => {
    const i = new Image();
    i.onload = () => {
      img = i;
      canvas.width  = Math.min(i.naturalWidth, 960);
      canvas.height = Math.round(canvas.width * CROP_H / CROP_W);
      ctx = canvas.getContext('2d');
      placeholder.style.display = 'none';
      canvas.classList.add('active');
      cropWrap.classList.add('has-img');
      zoomBar.classList.add('active');
      initCrop();
      uploadBtn.disabled = false;
    };
    i.src = ev.target.result;
  };
  reader.readAsDataURL(file);
});
function minScale() { return Math.max(canvas.width/img.naturalWidth, canvas.height/img.naturalHeight); }
function clamp() {
  const iw=img.naturalWidth*scale, ih=img.naturalHeight*scale;
  ox=Math.min(0,Math.max(canvas.width-iw,ox));
  oy=Math.min(0,Math.max(canvas.height-ih,oy));
}
function initCrop() {
  scale=minScale();
  ox=(canvas.width-img.naturalWidth*scale)/2;
  oy=(canvas.height-img.naturalHeight*scale)/2;
  clamp(); zoomSlider.min=minScale(); zoomSlider.max=minScale()*5; zoomSlider.value=scale; draw();
}
function draw() { if(ctx&&img) { ctx.clearRect(0,0,canvas.width,canvas.height); ctx.drawImage(img,ox,oy,img.naturalWidth*scale,img.naturalHeight*scale); } }

cropWrap.addEventListener('mousedown', e => { if(img){dragging=true;lastX=e.clientX;lastY=e.clientY;} });
window.addEventListener('mousemove',  e => { if(dragging&&img){let px=canvas.width/cropWrap.getBoundingClientRect().width;ox+=(e.clientX-lastX)*px;oy+=(e.clientY-lastY)*px;lastX=e.clientX;lastY=e.clientY;clamp();draw();} });
window.addEventListener('mouseup', () => dragging=false);
cropWrap.addEventListener('touchstart', e=>{if(img){e.preventDefault();dragging=true;lastX=e.touches[0].clientX;lastY=e.touches[0].clientY;}},{passive:false});
cropWrap.addEventListener('touchmove',  e=>{if(dragging&&img){e.preventDefault();let px=canvas.width/cropWrap.getBoundingClientRect().width;ox+=(e.touches[0].clientX-lastX)*px;oy+=(e.touches[0].clientY-lastY)*px;lastX=e.touches[0].clientX;lastY=e.touches[0].clientY;clamp();draw();}},{passive:false});
cropWrap.addEventListener('touchend', () => dragging=false);
zoomSlider.addEventListener('input', () => {
  if(img){let ns=parseFloat(zoomSlider.value),cx=canvas.width/2,cy=canvas.height/2;ox=cx-(cx-ox)*ns/scale;oy=cy-(cy-oy)*ns/scale;scale=ns;clamp();draw();}
});

async function uploadWallpaper() {
  if (!img||!ctx) return;
  uploadBtn.disabled = true;
  toast('✂️ Memotong gambar…', 'info');
  const out  = document.createElement('canvas');
  out.width  = CROP_W; out.height = CROP_H;
  const octx = out.getContext('2d'), s = CROP_W/canvas.width;
  octx.drawImage(img, ox*s, oy*s, img.naturalWidth*scale*s, img.naturalHeight*scale*s);
  out.toBlob(async blob => {
    try {
      const fd = new FormData(); fd.append('wallpaper', blob, 'wallpaper.jpg');
      const r  = await fetch('/upload', {method:'POST', body:fd});
      if (r.ok) toast('🌸 Wallpaper diperbarui!');
      else      toast('❌ Gagal upload', 'err');
    } catch { toast('❌ Tidak bisa terhubung', 'err'); }
    uploadBtn.disabled = false;
  }, 'image/jpeg', 0.88);
}

// ── Brightness ───────────────────────────────────────────────────
function onBrightnessInput(val) { document.getElementById('brightnessVal').textContent = val + '%'; }
async function saveBrightness() {
  const pct = parseInt(document.getElementById('brightnessSlider').value);
  const val = Math.round(pct * 255 / 100);
  try {
    const r = await fetch('/setbrightness', {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:'value='+val});
    if (r.ok) toast('☀️ Kecerahan disimpan!');
    else      toast('❌ Gagal simpan kecerahan', 'err');
  } catch { toast('❌ Tidak bisa terhubung', 'err'); }
}

// ── Theme color ──────────────────────────────────────────────────
const CLOCK_SWATCHES = ['#FFFFFF','#FFD6E0','#E8D5FF','#C3E8FF','#FFFACD','#FFB6C1','#87CEEB','#FFD700'];
const DATE_SWATCHES  = ['#AAAAAA','#DDAACC','#AACCEE','#CCAADD','#FFCCAA','#BBDDAA','#EEC0D0','#FFFFFF'];

function buildSwatches(id, swatches, onSel) {
  const c = document.getElementById(id); c.innerHTML = '';
  swatches.forEach(hex => {
    const s = document.createElement('div');
    s.className = 'swatch'; s.style.background = hex; s.title = hex;
    s.onclick = () => { onSel(hex); markSel(id, s); };
    c.appendChild(s);
  });
}
function markSel(id, el) { document.querySelectorAll('#'+id+' .swatch').forEach(s=>s.classList.remove('sel')); if(el) el.classList.add('sel'); }

function applyClockColor(hex) {
  document.getElementById('previewTime').style.color  = hex;
  document.getElementById('clockColorPicker').value   = hex;
  document.getElementById('clockColorHex').value      = hex.toUpperCase();
  document.getElementById('miniSwatchClock').style.background = hex;
  markSel('clockSwatches', [...document.querySelectorAll('#clockSwatches .swatch')].find(s=>s.title.toUpperCase()===hex.toUpperCase()));
}
function applyDateColor(hex) {
  document.getElementById('previewDate').style.color  = hex;
  document.getElementById('dateColorPicker').value    = hex;
  document.getElementById('dateColorHex').value       = hex.toUpperCase();
  document.getElementById('miniSwatchDate').style.background = hex;
  markSel('dateSwatches', [...document.querySelectorAll('#dateSwatches .swatch')].find(s=>s.title.toUpperCase()===hex.toUpperCase()));
}
function onClockColorInput(h) { applyClockColor(h); }
function onClockHexInput(v)   { let h=v.trim(); if(!h.startsWith('#')) h='#'+h; if(/^#[0-9A-Fa-f]{6}$/.test(h)) applyClockColor(h); }
function onDateColorInput(h)  { applyDateColor(h); }
function onDateHexInput(v)    { let h=v.trim(); if(!h.startsWith('#')) h='#'+h; if(/^#[0-9A-Fa-f]{6}$/.test(h)) applyDateColor(h); }

buildSwatches('clockSwatches', CLOCK_SWATCHES, applyClockColor);
buildSwatches('dateSwatches',  DATE_SWATCHES,  applyDateColor);
applyClockColor('#FFFFFF'); applyDateColor('#AAAAAA');

async function loadDisplaySettings() {
  try {
    const r = await fetch('/getsettings');
    if (!r.ok) return;
    const d = await r.json();
    if (d.brightness != null) {
      const pct = Math.round(d.brightness * 100 / 255);
      document.getElementById('brightnessSlider').value = pct;
      document.getElementById('brightnessVal').textContent = pct + '%';
    }
    if (d.clockColor) applyClockColor(d.clockColor);
    if (d.dateColor)  applyDateColor(d.dateColor);
    if (d.hasWallpaper) loadWallpaperPreview();
  } catch {}
}

async function loadWallpaperPreview() {
  try {
    const r = await fetch('/getwallpaper');
    if (!r.ok) return;
    const blob = await r.blob();
    const url  = URL.createObjectURL(blob);
    const image = new Image();
    image.onload = () => {
      img = image;
      canvas.width  = Math.min(image.naturalWidth, 960);
      canvas.height = Math.round(canvas.width * CROP_H / CROP_W);
      ctx = canvas.getContext('2d');
      placeholder.style.display = 'none';
      canvas.classList.add('active');
      cropWrap.classList.add('has-img');
      zoomBar.classList.add('active');
      initCrop();
      uploadBtn.disabled = false;
      const badge = document.getElementById('currentWpBadge');
      if (badge) badge.style.display = 'inline-flex';
    };
    image.src = url;
  } catch {}
}

async function saveTheme() {
  const ch = document.getElementById('clockColorHex').value.trim();
  const dh = document.getElementById('dateColorHex').value.trim();
  if (!/^#[0-9A-Fa-f]{6}$/.test(ch)) { toast('❌ Warna jam tidak valid', 'err'); return; }
  if (!/^#[0-9A-Fa-f]{6}$/.test(dh)) { toast('❌ Warna tanggal tidak valid', 'err'); return; }
  try {
    const r = await fetch('/settheme', {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:new URLSearchParams({color:ch,dateColor:dh})});
    if (r.ok) toast('🎨 Warna berhasil disimpan!');
    else      toast('❌ Gagal simpan warna', 'err');
  } catch { toast('❌ Tidak bisa terhubung', 'err'); }
}

// ── WiFi AP ──────────────────────────────────────────────────────
async function saveAP() {
  const ssid = document.getElementById('apSsid').value.trim();
  const pass = document.getElementById('apPass').value;
  if (!ssid)         { toast('❌ SSID tidak boleh kosong', 'err'); return; }
  if (pass.length<8) { toast('❌ Password minimal 8 karakter', 'err'); return; }
  try {
    const r = await fetch('/setap', {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:new URLSearchParams({ssid,pass})});
    if (r.ok) { toast('✅ Tersimpan! Restart…'); startRestartCountdown(); }
    else      toast('❌ Gagal simpan AP', 'err');
  } catch { toast('❌ Tidak bisa terhubung', 'err'); }
}
function startRestartCountdown() {
  const notice=document.getElementById('restartNotice'), cd=document.getElementById('countdown');
  notice.classList.add('show'); let n=5; cd.textContent=n;
  const iv=setInterval(()=>{ n--; cd.textContent=n; if(n<=0) clearInterval(iv); },1000);
}

// ── Greeting ─────────────────────────────────────────────────────
window._greetEnabled = false;
function updateToggleUI() {
  const track=document.getElementById('toggleTrack'), thumb=document.getElementById('toggleThumb');
  track.style.background = window._greetEnabled ? 'linear-gradient(135deg,var(--accentDark),var(--accent))' : '#ddd';
  thumb.style.transform  = window._greetEnabled ? 'translateX(22px)' : 'translateX(0)';
}
window.toggleGreeting = () => { window._greetEnabled = !window._greetEnabled; updateToggleUI(); buildGreetPreview(); };
function buildGreetPreview() {
  const en=window._greetEnabled, msg=document.getElementById('greetMsg').value.trim();
  const el=document.getElementById('greetPreview');
  el.textContent = en ? (msg||'—') : '(sapaan dinonaktifkan)';
  el.style.color = en ? 'var(--ink)' : 'var(--muted)';
}
document.getElementById('greetMsg').addEventListener('input', buildGreetPreview);
updateToggleUI(); buildGreetPreview();

async function saveGreeting() {
  const enabled = window._greetEnabled ? '1' : '0';
  const message = document.getElementById('greetMsg').value.trim();
  try {
    const r = await fetch('/setgreeting', {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:new URLSearchParams({enabled,message})});
    if (r.ok) toast('💌 Sapaan disimpan!');
    else      toast('❌ Gagal simpan sapaan', 'err');
  } catch { toast('❌ Tidak bisa terhubung', 'err'); }
}
async function loadGreeting() {
  try {
    const r = await fetch('/getgreeting'); if(!r.ok) return;
    const d = await r.json();
    window._greetEnabled = !!d.enabled; updateToggleUI();
    document.getElementById('greetMsg').value = d.message || '';
    buildGreetPreview();
  } catch {}
}

// ── Toast helper ─────────────────────────────────────────────────
function toast(msg, type='ok') {
  const wrap = document.getElementById('toastWrap');
  const el   = document.createElement('div');
  el.className = 'toast ' + type;
  el.textContent = msg;
  wrap.appendChild(el);
  setTimeout(() => { el.classList.add('leave'); setTimeout(() => el.remove(), 300); }, 2800);
}

// ── Init ─────────────────────────────────────────────────────────
loadDisplaySettings();
loadGreeting();
</script>
</body>
</html>
)rawhtml";