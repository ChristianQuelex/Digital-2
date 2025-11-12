/*
  ESP32 WebServer (STA) + AJAX + I2C Slave (0x28)
  UI nivel 2 con tema claro/arreglado.
  FIX: Servir HTML grande desde PROGMEM usando server.send_P() para evitar problemas de RAM.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

/* ===== Wi-Fi (STA) ===== */
const char* WIFI_SSID = "moto g11";
const char* WIFI_PASS = "Digital2";

/* ===== I2C Slave ===== */
static const uint8_t I2C_SLAVE_ADDR = 0x28;
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;

/* ===== HTTP ===== */
WebServer server(80);

/* ===== Estado ===== */
volatile uint8_t  g_parking_state = 0x00;   // bit0=slot1 .. bit7=slot8 (1=ocupado)
volatile uint32_t g_last_update_ms = 0;

/* ===== HTML/CSS/JS en PROGMEM ===== */
static const char PAGE_INDEX[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>Parqueos ESP32</title>
<style>
  :root{
    --bg:#0f141b; --card:#151b23; --muted:#9aa4b2; --text:#e6eaf0;
    --ok:#2ecc71; --bad:#e74c3c; --chip:#1f2630; --chipText:#cbd5e1; --accent:#3b82f6;
    --shadow:0 6px 22px rgba(0,0,0,.35);
  }
  :root.light{
    --bg:#f3f4f7; --card:#ffffff; --muted:#5c6671; --text:#1f2937;
    --ok:#22c55e; --bad:#ef4444; --chip:#e5e7eb; --chipText:#334155; --accent:#2563eb;
    --shadow:0 8px 24px rgba(0,0,0,.08);
  }
  *{box-sizing:border-box;font-family:Inter,system-ui,Segoe UI,Roboto,Arial,sans-serif}
  body{margin:0;background:var(--bg);color:var(--text)}
  .wrap{max-width:1100px;margin:18px auto 28px;padding:0 16px}
  header{display:flex;gap:10px;align-items:center;justify-content:space-between;margin-bottom:14px}
  h1{margin:0;font-size:clamp(20px,2.6vw,28px);letter-spacing:.2px}
  .right{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
  .chip{background:var(--chip);color:var(--chipText);padding:6px 10px;border-radius:999px;font-size:.86rem}
  .toolbar{display:flex;gap:8px;flex-wrap:wrap}
  .btn{border:1px solid transparent;background:var(--chip);color:var(--text);padding:8px 12px;border-radius:10px;cursor:pointer;transition:.2s}
  .btn:hover{border-color:var(--accent)}
  .btn.active{outline:2px solid var(--accent)}
  .grid{display:grid;gap:14px}
  .grid.c42{grid-template-columns:repeat(4,1fr)}
  .grid.c24{grid-template-columns:repeat(2,1fr)}
  .grid.list{grid-template-columns:1fr}
  @media (max-width: 860px){ .grid.c42{grid-template-columns:repeat(2,1fr)} }
  @media (max-width: 520px){ .grid.c42,.grid.c24{grid-template-columns:1fr} }
  .card{
    background:var(--card); border-radius:16px; padding:18px 16px; box-shadow:var(--shadow);
    display:flex; align-items:center; justify-content:space-between; gap:14px; min-height:92px;
  }
  .name{font-weight:700;font-size:1.02rem;letter-spacing:.2px}
  .state{display:flex;align-items:center;gap:10px}
  .led{ width:18px;height:18px;border-radius:50%;
        box-shadow:0 0 0 3px rgba(255,255,255,.06), inset 0 2px 4px rgba(0,0,0,.5); }
  .led.ok{background:linear-gradient(180deg,#47e88a,#17984e)}
  .led.bad{background:linear-gradient(180deg,#ff7c74,#b31210)}
  .pill{padding:6px 10px;border-radius:999px;color:#fff;font-size:.88rem;min-width:84px;text-align:center}
  .pill.ok{background:var(--ok)}
  .pill.bad{background:var(--bad)}
  .icon{font-size:28px;filter:drop-shadow(0 2px 1px rgba(0,0,0,.25))}
  .footer{margin-top:12px;color:var(--muted);font-size:.9rem}
</style>
</head>
<body>
<div class="wrap">
  <header>
    <h1>Parqueos ESP32</h1>
    <div class="right">
      <span id="ssid" class="chip">SSID: —</span>
      <span id="ip"   class="chip">IP: —</span>
      <span id="time" class="chip">Última: —</span>
      <div class="toolbar">
        <button id="b42"   class="btn">Vista 4×2</button>
        <button id="b24"   class="btn">Vista 2×4</button>
        <button id="blist" class="btn">Lista</button>
        <button id="theme" class="btn">Tema: Oscuro</button>
      </div>
    </div>
  </header>

  <div id="grid" class="grid c42"></div>

  <div class="footer">Actualización AJAX 500 ms · Maestro → ESP32 por I²C (1 byte = 8 parqueos)</div>
</div>

<script>
  const grid  = document.getElementById('grid');
  const ssid  = document.getElementById('ssid');
  const ip    = document.getElementById('ip');
  const timeE = document.getElementById('time');
  const root  = document.documentElement; // <html>

  const pref = {
    layout: localStorage.getItem('pk_layout') || 'c42',
    theme:  localStorage.getItem('pk_theme')  || 'dark'
  };

  const b42   = document.getElementById('b42');
  const b24   = document.getElementById('b24');
  const blist = document.getElementById('blist');
  const btheme= document.getElementById('theme');

  function applyLayout(l){
    grid.classList.remove('c42','c24','list');
    grid.classList.add(l);
    [b42,b24,blist].forEach(b=>b.classList.remove('active'));
    ({'c42':b42,'c24':b24,'list':blist})[l].classList.add('active');
    pref.layout = l; localStorage.setItem('pk_layout', l);
  }

  function applyTheme(t){
    if(t==='light'){ root.classList.add('light'); btheme.textContent='Tema: Claro'; }
    else { root.classList.remove('light'); btheme.textContent='Tema: Oscuro'; }
    pref.theme=t; localStorage.setItem('pk_theme', t);
  }

  b42.onclick   = ()=>applyLayout('c42');
  b24.onclick   = ()=>applyLayout('c24');
  blist.onclick = ()=>applyLayout('list');
  btheme.onclick= ()=>applyTheme(pref.theme==='dark'?'light':'dark');

  applyLayout(pref.layout);
  applyTheme(pref.theme);

  const slots = Array.from({length:8}, (_,i)=>({ id:i+1, occupied:false }));

  function render(){
    grid.innerHTML = slots.map(s=>{
      const led  = `<span class="led ${s.occupied?'bad':'ok'}"></span>`;
      const pill = `<span class="pill ${s.occupied?'bad':'ok'}">${s.occupied?'OCUPADO':'LIBRE'}</span>`;
      const ico  = `<span class="icon">${s.occupied?'🚗':'🅿️'}</span>`;
      return `
        <div class="card">
          <div class="name">Parqueo ${s.id}</div>
          <div class="state">${ico}${led}${pill}</div>
        </div>
      `;
    }).join('');
  }

  async function poll(){
    try{
      const r = await fetch('/state',{cache:'no-store'});
      if(!r.ok) return;
      const d = await r.json();
      const byte = d.byte & 0xFF;
      for(let i=0;i<8;i++){ slots[i].occupied = ((byte>>i)&1)===1; }
      render();
      ssid.textContent = 'SSID: ' + (d.ssid || '—');
      ip.textContent   = 'IP: '   + (d.ip   || '—');
      timeE.textContent= 'Última: ' + new Date(d.ts_ms||Date.now()).toLocaleTimeString();
    }catch(e){}
  }

  render();
  setInterval(poll,500);
  poll();
</script>
</body>
</html>)rawliteral";

/* ===== Handlers HTTP ===== */
void handleRoot(){ server.send_P(200,"text/html; charset=utf-8",PAGE_INDEX); }
void handleState(){
  String json="{";
  json+="\"ssid\":\""+String(WiFi.SSID())+"\",";
  json+="\"ip\":\""+WiFi.localIP().toString()+"\",";
  json+="\"byte\":"+String(g_parking_state)+",";
  json+="\"ts_ms\":"+String((uint32_t)millis());
  json+="}";
  server.send(200,"application/json; charset=utf-8",json);
}
void handleNotFound(){ server.send(404,"text/plain","404 "+server.uri()); }
void handleHealth(){ server.send(200,"text/plain","OK"); }

/* ===== I2C callbacks ===== */
void onI2CReceive(int len){
  while(Wire.available()>0){
    int b = Wire.read();
    if(b>=0){ g_parking_state = (uint8_t)b; g_last_update_ms = millis(); }
  }
}
void onI2CRequest(){ /* opcional: Wire.write(g_parking_state); */ }

/* ===== Wi-Fi ===== */
void connectWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0=millis();
  while(WiFi.status()!=WL_CONNECTED){
    delay(300);
    if(millis()-t0>25000){ ESP.restart(); }
  }
}

/* ===== Setup / Loop ===== */
void setup(){
  Serial.begin(115200); delay(100);

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);

  connectWiFi();

  server.on("/",handleRoot);
  server.on("/state",handleState);
  server.on("/health",handleHealth);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.print("IP: "); Serial.println(WiFi.localIP());
  Serial.println("HTTP listo. Visita http://<IP>/health para probar.");
}
void loop(){ server.handleClient(); }
