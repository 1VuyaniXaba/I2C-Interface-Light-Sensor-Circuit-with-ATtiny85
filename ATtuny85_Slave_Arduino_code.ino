/*
  ATtiny85  -  LDR night light + I2C slave

  Wiring (from your schematic)
    PB0 (pin 5) = SDA  -> Uno A4   (4.7k pull-up to 5V)
    PB2 (pin 7) = SCL  -> Uno A5   (4.7k pull-up to 5V)
    PB3 (pin 2) = LDR divider node (5V - LDR - node - 10k - GND)
    PB4 (pin 3) = 220R ->LED ->  GND   (active HIGH)
    Vcc = 5V, GND = common with the Uno
    PB5 (RESET): add a 10k pull-up to Vcc for reliability

  Arduino IDE setup
    Board package : ATTinyCore (Spence Konde)
    Board         : ATtiny25/45/85 (no bootloader)
    Chip          : ATtiny85
    Clock         : 8 MHz (internal)
    Programmer    : USBasp / Arduino as ISP  ->  Burn Bootloader once (sets the fuses), then Upload

  Behaviour
    - Dark   -> LED ON,  Bright -> LED OFF  (with hysteresis so it does not flicker)
    - The I2C master can read 3 bytes at any time:
        byte 0 : status   bit0 = LED is ON, bit1 = it is dark
        byte 1 : LDR reading, high byte (0..1023)
        byte 2 : LDR reading, low byte
*/

#include <Wire.h>

// ---------------- settings ----------------
const uint8_t  I2C_ADDR     = 0x20;   // slave address (0x08..0x77)
const uint8_t  LDR_PIN      = A3;     // PB3
const uint8_t  LED_PIN      = 4;      // PB4

// ADC = 1023 * 10k / (R_ldr + 10k)  ->  low value = dark
const uint16_t DARK_BELOW   = 400;    // turn LED ON  when reading falls below this
const uint16_t BRIGHT_ABOVE = 500;    // turn LED OFF when reading rises above this
                                      // (keep BRIGHT_ABOVE > DARK_BELOW)

const uint8_t  NUM_SAMPLES  = 8;      // averaging
const uint16_t SAMPLE_MS    = 50;     // how often the LDR is read
// ------------------------------------------

volatile uint8_t packet[3] = {0, 0, 0};   // what the master receives
bool ledOn = false;

// Runs in interrupt context when the master reads: keep it short.
void onRequest() {
  Wire.write(packet[0]);
  Wire.write(packet[1]);
  Wire.write(packet[2]);
}

uint16_t readLdr() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < NUM_SAMPLES; i++) sum += analogRead(LDR_PIN);
  return (uint16_t)(sum / NUM_SAMPLES);
}

void updateLight() {
  uint16_t v = readLdr();

  if (!ledOn && v < DARK_BELOW)        ledOn = true;    // got dark
  else if (ledOn && v > BRIGHT_ABOVE)  ledOn = false;   // got bright
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  uint8_t status = ledOn ? 0x03 : 0x00;   // bit0 LED on, bit1 dark (same thing here)

  noInterrupts();                          // so the master never reads half-updated data
  packet[0] = status;
  packet[1] = v >> 8;
  packet[2] = v & 0xFF;
  interrupts();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  updateLight();                           // valid data before the first request

  Wire.begin(I2C_ADDR);                    // join the bus as a slave
  Wire.onRequest(onRequest);
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last >= SAMPLE_MS) {
    last = millis();
    updateLight();
  }
}
