# I2C-Interface-Light-Sensor-Circuit-with-ATtiny85
An ATtiny85 night-light controller. An LDR measures ambient light, and the ATtiny85 switches an LED on in the dark and off in the light, using two thresholds to prevent flicker. It also works as an I2C slave (address 0x20), so an Arduino Uno master can read the light level (0-1023) and LED state on demand. Built with ATTinyCore, 3-byte protocol.
