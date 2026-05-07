# Air Sensor — Hardware

## Board
Seeed XIAO ESP32-C3

## Sensor
SHT30 temperature/humidity breakout (I2C)

## Wiring

| Wire   | Signal | XIAO ESP32-C3 Pin |
|--------|--------|-------------------|
| Red    | VCC    | 3.3V              |
| Black  | GND    | GND               |
| Yellow | SCL    | D5 (GPIO7)        |
| White  | SDA    | D4 (GPIO6)        |

SHT30 I2C address: `0x44` (ADDR pin low/floating). Pull ADDR high for `0x45`.

## Notes
- No external pull-up resistors needed; the Wire library enables internal pull-ups.
- The XIAO's USB-C port powers the board and sensor from 3.3V regulated output.
