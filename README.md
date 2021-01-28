# Leaf inverter control
### Hardware
- Arduino
- MCP2515 shield or board

### Wiring
Follow instructions of the MCP2515 board.

Make sure to terminate the bus with a 120-ohm resistor and use twisted wiring at all points.

| Microchip MCP2515 | Arduino |
| :---------------: | :-----: |
| VCC | 5V |
| GND | GND |
| SCK | SCK |
| SO | MISO |
| SI | MOSI |
| CS | 10 |
| INT | 2 |

### Software
- [Install Platformio](https://docs.platformio.org/page/installation.html)
- `cd` to project
- `pio upload`

### How to use
- `cd` to project
- `pio device monitor`
- Use `space` to turn on/off the CAN BUS
- Use `[1-9]` to control speed
    - `1` is 0%
    - `3` is the minimum to start
    - `9` is ~10%
    - `0` is -1%.
      This is needed to slow down past ~500RPM.
      You may also press `space` which puts the inverter into idle.
