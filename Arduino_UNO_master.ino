/*
  Arduino Uno  -  I2C master that reads the ATtiny85 light sensor
  SDA = A4, SCL = A5 (pull-ups already in your circuit)
  Open the Serial Monitor at 9600 baud.
*/

#include <Wire.h>

const uint8_t TINY_ADDR = 0x20;   // must match I2C_ADDR in the ATtiny85 firmware

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  uint8_t got = Wire.requestFrom(TINY_ADDR, (uint8_t)3);

  if (got == 3) {
    uint8_t  status = Wire.read();
    uint16_t adc    = (uint16_t)Wire.read() << 8;
    adc            |= Wire.read();

    Serial.print("LDR: ");
    Serial.print(adc);
    Serial.print("  |  LED: ");
    Serial.print((status & 0x01) ? "ON " : "OFF");
    Serial.print("  |  ");
    Serial.println((status & 0x02) ? "dark" : "bright");
  } else {
    Serial.println("ATtiny85 did not answer - check wiring, address and pull-ups");
  }

  delay(500);
}
