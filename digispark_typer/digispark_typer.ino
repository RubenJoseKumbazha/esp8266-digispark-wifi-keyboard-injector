/*  WiFiTyper — Digispark ATtiny85 receiver
 *
 *  Receives text from ESP8266 over a bit-bang UART on P2 at 4800 baud
 *  and types it on the target computer as a USB HID keyboard.
 *
 *  No SoftwareSerial needed — uses avr/io.h directly (compatible with
 *  the Digistump tiny core which does not ship SoftwareSerial).
 *
 *  Wiring:
 *    ESP8266 D1 Mini TX  →  Digispark P2
 *    ESP8266 D1 Mini GND →  Digispark GND
 *
 *  Board:   digistump:avr:digispark-tiny
 *  Library: DigisparkKeyboard (bundled with Digistump package)
 */

#include <DigiKeyboard.h>
#include <avr/io.h>
#include <util/delay.h>

#define RX_BIT PB2   // Digispark P2

// ── 4800 baud timing at F_CPU = 16 500 000 Hz ─────────────────────────────
// 1 bit = 208.33 µs | subtract ~6 µs for loop overhead to stay centred
static inline void waitFullBit() { _delay_us(202); }
static inline void waitHalfBit() { _delay_us(104); }

uint8_t uartReadByte() {
  // Called on falling start-bit edge.
  // Skip 1.5 bit periods to land in the centre of data bit 0.
  waitFullBit();
  waitHalfBit();

  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (PINB & (1 << RX_BIT)) data |= (1 << i);
    waitFullBit();
  }
  return data;
}

// ── State ──────────────────────────────────────────────────────────────────
String   buf       = "";
bool     receiving = false;
uint32_t lastByte  = 0;

const uint16_t FLUSH_MS = 300;

void typeBuffer() {
  if (buf.length() == 0) return;
  for (int i = 0; i < (int)buf.length(); i++) {
    DigiKeyboard.update();
    DigiKeyboard.print(buf[i]);
    DigiKeyboard.delay(20);
  }
  buf       = "";
  receiving = false;
}

void setup() {
  DDRB  &= ~(1 << RX_BIT);   // P2 as input
  PORTB |=  (1 << RX_BIT);   // internal pull-up → line idles HIGH
  DigiKeyboard.delay(2000);   // wait for USB host to enumerate HID device
}

void loop() {
  DigiKeyboard.update();

  if (!(PINB & (1 << RX_BIT))) {
    uint8_t c = uartReadByte();
    if (c == 0x00) {
      typeBuffer();             // null sentinel from ESP → type now
    } else {
      buf      += (char)c;
      lastByte  = millis();
      receiving = true;
    }
  }

  if (receiving && (millis() - lastByte > FLUSH_MS)) {
    typeBuffer();
  }
}
