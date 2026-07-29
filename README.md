# node_reset
## Sends a periodic reset, based on attiny13a

Sends a reset every 15 days. Value can be changed via constant at beginning of code.

Connect MCU + to battery +, MCU - to battery -

IF a BMS is used, connect after the BMS

Connect target MCU reset pin to node_reset MCU physical pin 3 (PB4)

## Serial debug output

Serial output is available on pin 5 at 2400bps via software bit-bang serial routines

Open or resistor status, ADC and maxwakes output at startup

Message output at reset

## Setting reset delay via resistor

If physical pin 2 is shorted to ground, reset time will be ~24 seconds, used for testing.

If physical pin 2 is open, reset time will be maximum of 15 days.

Any resistor you place in between pin 2 and ground will form a voltage divider to shorten the delay from the max of 15 days

### Reference Guide for Common Resistors
Approximate delay baed on resistor size:

| Resistor Value | maxWakes | Resulting Reset Delay |
| --- | --- | ---|
| 0  (Wire link)   |    3   |                 24 Seconds   (Great for testing!)|
| 330Ω |          3,000 |                  ~6.5 hours|
| ∞ (Open) | 158,040 |                   168 Hours (code override to 168 hours max)|
