# I2C-Interface-Light-Sensor-Circuit-with-ATtiny85
An ATtiny85 night-light controller. An LDR measures ambient light, and the ATtiny85 switches an LED on in the dark and off in the light, using two thresholds to prevent flicker. It also works as an I2C slave (address 0x20), so an Arduino Uno master can read the light level (0-1023) and LED state on demand. Built with ATTinyCore, 3-byte protocol.
# ATtiny85 Light Sensor with I2C Slave Output

An ATtiny85 that switches an LED on in the dark and off in the light using an LDR, and reports the light level and LED state to an I2C master (an Arduino Uno in this project).

## Features

- Automatic LED control: ON when dark, OFF when bright
- Hysteresis (two thresholds) so the LED does not flicker at dusk
- Averaged ADC readings (8 samples) for stable values
- I2C slave using the ATtiny85's USI hardware, address `0x20`
- Master can read the raw LDR value (0–1023) and the LED state at any time

## Hardware

| Part | Qty | Notes |
|------|-----|-------|
| ATtiny85 | 1 | Runs at 8 MHz (internal oscillator) |
| LDR (photoresistor) | 1 | |
| 10 kΩ resistor | 1 | LDR divider |
| 220 Ω resistor | 1 | LED current limiting |
| LED | 1 | Any colour |
| 4.7 kΩ resistors | 2 | I2C pull-ups (SDA and SCL to 5 V) |
| 100 nF capacitor | 1 | Between Vcc and GND, close to the chip |
| Arduino Uno | 1 | I2C master |
| Programmer | 1 | USBasp, or an Arduino as ISP |

## Wiring

| ATtiny85 pin | Name | Connects to |
|--------------|------|-------------|
| 1 | PB5 (RESET) | 10 kΩ pull-up to Vcc (recommended) |
| 2 | PB3 (ADC3) | LDR divider node |
| 3 | PB4 | LED anode (via 220 Ω to GND) |
| 4 | GND | Common ground with the Uno |
| 5 | PB0 (SDA) | Uno A4 (4.7 kΩ pull-up to 5 V) |
| 7 | PB2 (SCL) | Uno A5 (4.7 kΩ pull-up to 5 V) |
| 8 | Vcc | 5 V |

LDR divider: `5 V → LDR → node (PB3) → 10 kΩ → GND`. A bright room gives a high reading and darkness gives a low one.

## Software setup

1. In the Arduino IDE, add this URL under **File → Preferences → Additional Boards Manager URLs**:
   `http://drazzy.com/package_drazzy.com_index.json`
2. Install **ATTinyCore** from the Boards Manager.
3. Select **ATtiny25/45/85 (No bootloader)**, chip **ATtiny85**, clock **8 MHz (internal)**.
4. Select your programmer and run **Burn Bootloader** once (this sets the fuses).
5. Upload `attiny85_ldr_led_i2c.ino` to the ATtiny85.
6. Upload `uno_master_reader.ino` to the Uno and open the Serial Monitor at 9600 baud.

> The standard Arduino AVR core's `Wire` library does not support the ATtiny85 and gives `TWCR`/`TWSR` undeclared errors. Use ATTinyCore, which provides a USI-based `Wire`.

## I2C protocol

The master requests 3 bytes from address `0x20`:

| Byte | Content |
|------|---------|
| 0 | Status: bit 0 = LED on, bit 1 = dark |
| 1 | LDR reading, high byte |
| 2 | LDR reading, low byte (value is 0–1023) |

Example on the master:

```cpp
Wire.requestFrom(0x20, 3);
uint8_t  status = Wire.read();
uint16_t adc    = (uint16_t)Wire.read() << 8;
adc |= Wire.read();
```

## Tuning

Edit these constants in `attiny85_ldr_led_i2c.ino`:

```cpp
const uint16_t DARK_BELOW   = 400;  // LED turns ON below this reading
const uint16_t BRIGHT_ABOVE = 500;  // LED turns OFF above this reading
```

Watch the values in the Serial Monitor in your own lighting, then place the two thresholds between your dark and bright readings. Keep `BRIGHT_ABOVE` larger than `DARK_BELOW`.

## Power

The ATtiny85 runs from 2.7 V to 5.5 V. Use a regulated 5 V supply (or about 4.5 V from 3×AA) and share ground with the master. Do not connect a 9 V battery directly to Vcc.

## Troubleshooting

- **Compile error about `TWCR`, `TWSR`, `SDA` or `SCL`:** the wrong board package is selected. Use ATTinyCore (see above).
- **"ATtiny85 did not answer":** check SDA/SCL wiring, the 4.7 kΩ pull-ups, the common ground, and that the address matches in both sketches.
- **LED switches at the wrong light level:** adjust `DARK_BELOW` and `BRIGHT_ABOVE`.

## Repository contents

```
attiny85_ldr_led_i2c/   ATtiny85 firmware (I2C slave + LDR/LED logic)
uno_master_reader/      Arduino Uno I2C master example
```

## License

