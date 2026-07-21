# node_reset
Sends a periodic reset, based on attiny13a

To achieve this without adding bulk or extra components to your project, you can take advantage of a clever hardware trick: using the ATtiny13A's internal pull-up resistor to create an on-chip voltage divider. By enabling the internal pull-up on an analog input pin, the ATtiny13A provides its own internal reference resistor (roughly $35\text{k}\Omega$). You only need to attach your timing resistor between Physical Pin 2 (PB3) and GND.How the Scaling Works NaturallyNo Resistor (Open Circuit): The internal pull-up pulls the pin all the way up to VCC. The ADC reads 1023, which sets the timer to 48 hours.Shorted to GND ($0\Omega$): The pin is pulled completely to Ground. The ADC reads 0, which sets the timer to 8 seconds.

Common Resistors: Any resistor you place in between will form a voltage divider to shorten the delay from the max of 168 hours

Reference Guide for Common Resistors
Because the internal pull-up is roughly $35\text{k}\Omega$, here is what you can expect when using everyday resistor sizes:

Resistor Value  maxWakes                Resulting Reset Delay
0  (Wire link)       2                    16 Seconds   (Great for testing!)
150Ω             7,686                    ~17 hours
330Ω            14,337                    ~32 hours
470Ω            18,623                    ~41.5 hours
1KΩ             30,521                    ~68 hours
∞ (Open)        75,600                    168 Hours (code override to 168 hours max)
