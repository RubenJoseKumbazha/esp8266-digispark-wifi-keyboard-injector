#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid     = "YOUR_WIFI_SSID";      // <-- change this
const char* password = "YOUR_WIFI_PASSWORD";   // <-- change this

ESP8266WebServer server(80);

// Chunk size — ESP sends this many chars at a time, waits for Digispark to
// finish typing before sending the next chunk. Keeps ATtiny85 SRAM safe.
#define CHUNK_SIZE 30

const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html><html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Remote Typer</title>
  <style>
    body{font-family:sans-serif;max-width:520px;margin:40px auto;padding:0 16px}
    h2{margin-bottom:8px}
    textarea{width:100%;height:220px;font-size:15px;padding:8px;
             box-sizing:border-box;border:1px solid #ccc;border-radius:6px}
    .row{display:flex;gap:8px;margin-top:8px}
    button{flex:1;padding:10px;font-size:15px;border:none;border-radius:6px;
           background:#2d6be4;color:#fff;cursor:pointer}
    button:active{opacity:.8}
    #status{margin-top:10px;font-size:13px;color:#555}
  </style>
</head>
<body>
  <h2>Remote Typer</h2>
  <textarea id="txt" placeholder="Paste your text here…"></textarea>
  <div class="row">
    <button onclick="sendText()">⌨ Type it!</button>
    <button onclick="document.getElementById('txt').value=''">Clear</button>
  </div>
  <div id="status"></div>
  <script>
    async function sendText(){
      const txt = document.getElementById('txt').value;
      if(!txt) return;
      const st = document.getElementById('status');
      st.textContent = 'Sending…';
      const r = await fetch('/type',{method:'POST',
        headers:{'Content-Type':'application/x-www-form-urlencoded'},
        body:'text='+encodeURIComponent(txt)});
      const msg = await r.text();
      st.textContent = msg;
    }
  </script>
</body>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", PAGE);
}

void handleType() {
  if (!server.hasArg("text") || server.arg("text").length() == 0) {
    server.send(400, "text/plain", "No text received.");
    return;
  }

  String text = server.arg("text");
  int total   = text.length();
  int sent    = 0;

  while (sent < total) {
    int chunkLen = min(CHUNK_SIZE, total - sent);

    for (int i = 0; i < chunkLen; i++) {
      Serial.write(text[sent + i]);
      delay(5);
    }
    Serial.write('\x00');   // null sentinel — signals Digispark to type this chunk

    sent += chunkLen;

    if (sent < total) {
      delay(chunkLen * 25 + 100);   // wait for Digispark to finish typing
    }
  }

  server.send(200, "text/plain",
    "Typed " + String(total) + " characters in " +
    String((total + CHUNK_SIZE - 1) / CHUNK_SIZE) + " chunk(s).");
}

void setup() {
  Serial.begin(4800);
  delay(100);

  WiFi.begin(ssid, password);
  pinMode(LED_BUILTIN, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  server.on("/",     HTTP_GET,  handleRoot);
  server.on("/type", HTTP_POST, handleType);
  server.begin();
}

void loop() {
  server.handleClient();
}
