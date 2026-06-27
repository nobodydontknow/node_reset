# node_reset
Sends a periodic reset, based on attiny13a

To achieve this without adding bulk or extra components to your project, you can take advantage of a clever hardware trick: using the ATtiny13A's internal pull-up resistor to create an on-chip voltage divider. By enabling the internal pull-up on an analog input pin, the ATtiny13A provides its own internal reference resistor (roughly $35\text{k}\Omega$). You only need to attach your timing resistor between Physical Pin 2 (PB3) and GND.How the Scaling Works NaturallyNo Resistor (Open Circuit): The internal pull-up pulls the pin all the way up to VCC. The ADC reads 1023, which sets the timer to 48 hours.Shorted to GND ($0\Omega$): The pin is pulled completely to Ground. The ADC reads 0, which sets the timer to 8 seconds.

Common Resistors: Any resistor you place in between will form a voltage divider following the formula:

$$V_{adc} = V_{cc} \times \frac{R_{ext}}{R_{pu} + R_{ext}}$$This creates a smooth, logarithmic scaling curve where smaller resistors give you fine-tuned shorter delays, and larger resistors scale up toward 48 hours.

Reference Guide for Common Resistors
Because the internal pull-up is roughly $35\text{k}\Omega$, here is what you can expect when using everyday resistor sizes:

Resistor Value  Approximate ADC Reading  Resulting Reset Delay
0  (Wire link)                           08 Seconds   (Great for testing!)
10k Ω           227                      ~10.5 Hours
33k Ω           496                      ~23.5 Hours (Perfect 24-hour target)
47k Ω           586                      ~27.5 Hours
100k Ω          757                      ~35.5 Hours
∞ (Open)                                 10,2348 Hours (code override to 48 hours max)
