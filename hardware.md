# Air Sensor — Hardware

## Board

Seeed XIAO ESP32-C6

## Sensor

KOOBOOK SHT30-D temperature/humidity breakout (I2C)

### Sensor wiring

| Signal | XIAO ESP32-C3 Pin | Wire   |
|--------|-------------------|--------|
| VCC    | 3.3V              | Red    |
| GND    | GND               | Black  |
| SCL    | D5 (GPIO7)        | Yellow |
| SDA    | D4 (GPIO6)        | White  |

SHT30 I2C address: `0x44` (ADDR pin low/floating). Pull ADDR high for `0x45`.

## Display

Waveshare 2.9inch e-Paper Module — 296×128, black/white, SPI, partial refresh  
Module PCB dimensions: 100mm × 36.8mm

### Display wiring

| Signal | XIAO ESP32-C3 Pin      | Wire   |
|--------|------------------------|--------|
| DIN    | D10 (GPIO10, SPI MOSI) | Blue   |
| CLK    | D8 (GPIO8, SPI SCK)    | Yellow |
| CS     | D3 (GPIO5)             | Orange |
| DC     | D2 (GPIO4)             | Green  |
| RST    | D1 (GPIO3)             | White  |
| BUSY   | D6 (GPIO21)            | Purple |
| VCC    | 3.3V                   | Grey   |
| GND    | GND                    | Brown  |

## Enclosure

Laser-cut box, 3.2mm birch plywood, internal dimensions 95 × 40 × 55mm (W × D × H)

Display cutout in front face: **79.0mm × 36.7mm**

boxes.py URL:

```text
https://boxes.hackerspace-bamberg.de/ClosedBox?FingerJoint_style=rectangular&FingerJoint_surroundingspaces=2.0&FingerJoint_bottom_lip=0.0&FingerJoint_edge_width=1.0&FingerJoint_extra_length=0.0&FingerJoint_finger=1.0&FingerJoint_play=0.1&FingerJoint_space=1.0&FingerJoint_width=1.0&x=95&y=40&h=55&outside=0&thickness=3.2&burn=0.05&format=svg&labels=0&reference=100.0&tabs=0.0&qr_code=0&inner_corners=corner&spacing=0.5&debug=0&language=en&render=0
```

## Notes

- No external pull-up resistors needed; the Wire library enables internal pull-ups.
- The XIAO's USB-C port powers the board and sensor from 3.3V regulated output.
- BUSY pin moved to GPIO21 (D6) — GPIO2 is a strapping pin, unreliable as input.
- E-paper module uses 4-line SPI (jumper confirmed on module).

## Needed Updates

Window for the humidity box turns out to be the wrong size. Needs to be:

- 32 mm short-side
- 70 mm long-side

An updated humidity box url is at:

```text
https://boxes.hackerspace-bamberg.de/ClosedBox?FingerJoint_style=rectangular&FingerJoint_surroundingspaces=2.0&FingerJoint_bottom_lip=0.0&FingerJoint_edge_width=1.0&FingerJoint_extra_length=0.0&FingerJoint_finger=1.0&FingerJoint_play=0.1&FingerJoint_space=1.0&FingerJoint_width=1.0&x=98&y=35&h=55&outside=0&thickness=3.2&burn=0&format=svg&labels=0&reference=100.0&tabs=0.0&qr_code=0&inner_corners=corner&spacing=0.5&debug=0&language=en&render=0
```

and it's inside this directory as HumidityBox-2.svg
