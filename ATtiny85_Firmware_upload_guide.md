# ATtiny85 Night-Light: Programming Circuit, Final Circuit and Setup Steps

This document describes the two circuits used in the project and everything done to get the firmware onto the ATtiny85:

1. **Circuit 1 – Programming circuit:** an Arduino Uno used as an ISP (In-System Programmer) to load firmware onto the ATtiny85.
2. **Circuit 2 – Final circuit:** the finished system, with the ATtiny85 as an I2C slave, an LDR-controlled LED, and the Arduino Uno as I2C master.

---

## 1. Software and libraries

| Item | Purpose | Where from |
|------|---------|------------|
| Arduino IDE | Writing, compiling and uploading code | arduino.cc |
| **ATTinyCore** board package (Spence Konde) | Adds ATtiny85 support, including a `Wire` library that works as an I2C slave on the ATtiny85's USI hardware | Boards Manager (see below) |
| `Wire` library | I2C on both chips | Built in: the Arduino AVR core's version for the Uno, ATTinyCore's own version for the ATtiny85. No separate download needed |
| **ArduinoISP** example sketch | Turns the Uno into a programmer | Built into the IDE (File → Examples → 11.ArduinoISP) |

### 1.1 Installing ATTinyCore

1. Open **File → Preferences**.
2. In **Additional Boards Manager URLs**, add:
   `http://drazzy.com/package_drazzy.com_index.json`
3. Open **Tools → Board → Boards Manager**, search for **ATTinyCore** and click **Install**.

### 1.2 Board settings for the ATtiny85

Under the **Tools** menu:

| Setting | Value |
|---------|-------|
| Board | ATtiny25/45/85 (No bootloader) |
| Chip | ATtiny85 |
| Clock | 8 MHz (internal) |
| Programmer | Arduino as ISP |

> **Important:** the board must come from the *ATTinyCore* group. If a different core is selected, the compiler uses the standard Arduino `Wire` library and fails with errors such as `'TWCR' undeclared`, `'TWSR' undeclared`, `'SDA' undeclared` and `'SCL' undeclared`, because the ATtiny85 does not have the hardware TWI registers that library needs. Selecting ATTinyCore fixed this.

---

## 2. Circuit 1 – Programming the ATtiny85 (Arduino as ISP)

### 2.1 Step A: turn the Uno into a programmer

1. Connect the Uno by USB. Set **Tools → Board → Arduino Uno** and select the correct **Port**.
2. Open **File → Examples → 11.ArduinoISP → ArduinoISP**.
3. Click **Upload**. The Uno is now an ISP programmer.

Do this *before* adding the capacitor in the next step.

### 2.2 Step B: wire the ATtiny85 to the Uno

| Arduino Uno | ATtiny85 pin | Signal |
|-------------|--------------|--------|
| Pin 10 | Pin 1 (PB5) | RESET |
| Pin 11 | Pin 5 (PB0) | MOSI |
| Pin 12 | Pin 6 (PB1) | MISO |
| Pin 13 | Pin 7 (PB2) | SCK |
| 5 V | Pin 8 | Vcc |
| GND | Pin 4 | GND |

**Reset capacitor:** a 10 µF electrolytic capacitor is connected between the Uno's **RESET** pin (positive leg) and **GND** (negative leg, the side with the stripe). It stops the Uno from auto-resetting every time the IDE opens the serial port to talk to the ISP sketch, which would otherwise break programming.

Notes:
- Pin 4 (GND) must be connected to the Uno's GND. A common ground is required for programming to work.
- The wire colours in the diagram: orange = SCK, green = MISO, magenta = MOSI, blue = RESET, red = 5 V.

### 2.3 Step C: set the fuses and upload

1. Select the settings from section 1.2 (Programmer: **Arduino as ISP**).
2. Run **Tools → Burn Bootloader** once. For this core with no bootloader, this step simply writes the fuse settings for the internal 8 MHz clock.
3. Open `attiny85_ldr_led_i2c.ino`.
4. Upload with **Sketch → Upload Using Programmer** (Ctrl+Shift+U). The normal Upload button does not work, because there is no bootloader on the ATtiny85.
5. Wait for "Done uploading".

### 2.4 Problems met and fixes

| Problem | Cause | Fix |
|---------|-------|-----|
| Compile errors about `TWCR`, `TWSR`, `SDA`, `SCL` in `twi.c` | Wrong core: the standard Arduino AVR `Wire` library was being used | Install ATTinyCore and select ATtiny25/45/85 (No bootloader) |

---

## 3. Circuit 2 – The final circuit

After programming, the ATtiny85 is removed from the programming wiring and connected as follows. The programming capacitor is also removed from the Uno (see the note at the end of this section).

### 3.1 Connections

| ATtiny85 pin | Name | Connects to | Purpose |
|--------------|------|-------------|---------|
| 8 | Vcc | Uno 5 V | Power |
| 4 | GND | Uno GND | Common ground |
| 5 | PB0 | Uno A4 (SDA) | I2C data |
| 7 | PB2 | Uno A5 (SCL) | I2C clock |
| 2 | PB3 (ADC3) | LDR divider node | Light measurement |
| 3 | PB4 | LED, then 220 Ω to GND | Output LED |
| 1 | PB5 (RESET) | Not connected in the diagram (a 10 kΩ pull-up to Vcc is recommended) | |

### 3.2 Sub-circuits

**LDR divider:** `5 V → LDR → node → 10 kΩ → GND`, with the node connected to PB3. The schematic uses a variable resistor (shown at 4.11 kΩ) as the simulator stand-in for the LDR. In bright light the LDR resistance is low, so the node voltage is high; in the dark it is low.

**LED:** PB4 → LED → 220 Ω → GND (active HIGH).

**I2C bus:** SDA and SCL each have a 4.7 kΩ pull-up resistor to 5 V. The Uno's A4 and A5 and the ATtiny85's PB0 and PB2 share these lines. The ATtiny85 is the slave at address `0x20`; the Uno is the master.

**Power:** the ATtiny85 is powered from the Uno's 5 V, with grounds shared. A 100 nF capacitor between Vcc and GND close to the ATtiny85 is recommended.

### 3.3 Master sketch and testing

1. Remove the 10 µF capacitor from the Uno's RESET and GND. If it stays in place, the Uno cannot be programmed normally because it blocks the auto-reset.
2. Select **Board → Arduino Uno**, then upload `uno_master_reader.ino`.
3. Open the **Serial Monitor** at 9600 baud.
4. Cover the LDR: the LED should switch on and the reading should fall. Uncover it: the LED should switch off and the reading should rise.

Each reply from the ATtiny85 is 3 bytes: a status byte (bit 0 = LED on, bit 1 = dark) and the LDR reading as a high and low byte (0–1023).

### 3.4 Tuning

If the LED switches at the wrong light level, change these values in the ATtiny85 sketch and upload again:

```cpp
const uint16_t DARK_BELOW   = 400;  // LED on below this reading
const uint16_t BRIGHT_ABOVE = 500;  // LED off above this reading
```

---

## 4. Differences between the two circuits

| | Circuit 1 – Programming | Circuit 2 – Final |
|---|---|---|
| Uno's role | ISP programmer (ArduinoISP sketch) | I2C master (`uno_master_reader.ino`) |
| Uno pins used | 10, 11, 12, 13, 5 V, GND | A4 (SDA), A5 (SCL), 5 V, GND |
| ATtiny85 pins used | 1, 5, 6, 7 (programming), 4, 8 (power) | 2, 3, 5, 7 (signals), 4, 8 (power) |
| RESET capacitor | Needed (10 µF, RESET to GND) | Removed |
| Pull-ups | Not needed | 4.7 kΩ on SDA and SCL |
| LDR and LED | Not connected | Connected |

PB0, PB1 and PB2 do double duty: they are MOSI, MISO and SCK while programming, and PB0 and PB2 become SDA and SCL in the final circuit. This is why the ATtiny85 is programmed first and the I2C wiring is added afterwards.

---

## 5. Step-by-step summary

1. Install the Arduino IDE.
2. Add the ATTinyCore URL and install ATTinyCore from the Boards Manager.
3. Upload the **ArduinoISP** example to the Uno.
4. Wire Circuit 1 and fit the 10 µF capacitor between RESET and GND.
5. Select ATtiny25/45/85 (No bootloader), ATtiny85, 8 MHz internal, Arduino as ISP.
6. Run **Burn Bootloader** once.
7. Upload the firmware using **Upload Using Programmer**.
8. Remove the capacitor and the programming wires.
9. Wire Circuit 2 (I2C, LDR divider, LED, pull-ups).
10. Upload `uno_master_reader.ino` to the Uno and test with the Serial Monitor.
