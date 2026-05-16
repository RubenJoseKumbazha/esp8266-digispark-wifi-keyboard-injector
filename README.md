# esp8266-digispark-wifi-keyboard-injector 📡⌨️

A wireless remote keyboard injector using an **ESP8266 D1 Mini** and a **Digispark ATtiny85**.  
Paste any amount of text into a phone/browser web page → the Digispark types it on the target computer via USB HID.

---

## How it works

```
Your phone/browser
      │  WiFi (HTTP POST)
      ▼
ESP8266 D1 Mini          ← hosts the web page
      │  Serial TX → P2  (4800 baud, bit-bang UART)
      ▼
Digispark ATtiny85       ← appears as a USB keyboard
      │  USB HID
      ▼
Target computer          ← text gets typed here
```

Text is split into 30-character chunks on the ESP8266 so the ATtiny85's 512-byte SRAM is never overwhelmed, allowing unlimited paste length.

---

## Hardware

| Part | Role |
|---|---|
| ESP8266 D1 Mini | WiFi + web server |
| Digispark ATtiny85 | USB HID keyboard emulator |

### Wiring

| ESP8266 D1 Mini | Digispark ATtiny85 |
|---|---|
| TX (GPIO1) | P2 |
| GND | GND |

> **Power:** The Digispark is powered by the target computer's USB port. The ESP8266 can be powered separately (USB power bank, phone charger, etc.).

---

## Software setup

### ESP8266 (`esp_typer/esp_typer.ino`)

1. Install board: **ESP8266 by ESP8266 Community** via Arduino Board Manager
2. Install libraries: `ESP8266WiFi`, `ESP8266WebServer` (both included with the board package)
3. Open `esp_typer.ino` and set your WiFi credentials:
   ```cpp
   const char* ssid     = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
4. Flash to ESP8266 D1 Mini (board: *LOLIN(WEMOS) D1 mini*)

### Digispark (`digispark_typer/digispark_typer.ino`)

1. Install board: **Digistump AVR** via Arduino Board Manager  
   URL: `http://digistump.com/package_digistump_index.json`
2. No extra libraries needed — uses `DigisparkKeyboard` (bundled) and `avr/io.h`
3. Flash to Digispark (board: *Digispark (Default - 16.5mhz)*)  
   > Plug in the Digispark **within 60 seconds** of clicking Upload when prompted

---

## Usage

1. Power on the ESP8266 — the onboard LED blinks while connecting to WiFi, then goes solid
2. Find the ESP8266's IP address from your router's device list
3. Plug the Digispark into the target computer's USB port (wait ~6 seconds for it to enumerate)
4. Open `http://<ESP-IP>` in your browser
5. Paste your text → tap **Type it!**

---

## Configuration

Both constants are at the top of `esp_typer.ino`:

| Constant | Default | Effect |
|---|---|---|
| `CHUNK_SIZE` | `30` | Characters sent per batch — lower if you see drops |

In `digispark_typer.ino`:

| Constant | Default | Effect |
|---|---|---|
| `FLUSH_MS` | `300 ms` | of silence before auto-typing buffered chars |
| `DigiKeyboard.delay(20)` | `20 ms` | Delay between keystrokes — raise if keys are missed |

---

## Troubleshooting

| Problem | Fix |
|---|---|
| ESP won't connect | Double-check SSID/password, ensure 2.4 GHz network |
| Digispark shows Code 43 in Windows | Reflash Digispark | Open Device Manager → uninstall all hidden Digispark entries → reboot → replug |
| Characters missing or wrong | Increase `DigiKeyboard.delay()` from 20 to 30 in `digispark_typer.ino` |
| Digispark not detected | Wait the full 6 s after plugging in for the bootloader phase to finish |
| Wrong characters (symbols, punctuation) | Target PC keyboard layout must be **US QWERTY** |

---

## Limitations

- Target computer must use **US QWERTY** keyboard layout (DigiKeyboard uses fixed HID keycodes)
- Typing speed is intentionally slow (~20 ms/key) for reliability; not suitable for huge files
- No encryption on the web interface — use on a trusted local network only

---

## License

MIT — see [LICENSE](LICENSE)
