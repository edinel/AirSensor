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

## Display

Waveshare 2.9inch e-Paper Module — 296×128, black/white, SPI, partial refresh

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
