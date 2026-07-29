#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Debug configuration
#define DEBUG 0 // Set to 0 to disable debug output

#ifndef WDTIE
#define WDTIE WDIE  
#endif

// MicroCore built-in low-flash Serial
#define TX_PIN PB0                      // Physical Leg 5 (PB0) - Fixed macro name!
const int RESET_PIN = 4;                // Physical Leg 3 (PB4) -> Reset output
const analog_pin_t DELAY_PIN = A3;      // Physical Leg 2 (PB3) -> Delay input

// --- VOLTAGE-CALIBRATED CONSTANTS ---
// At 3.8V VCC, the Watchdog runs at ~8.2 seconds per wake cycle.
// 86,400 seconds / 8.2 seconds = ~10,536 wakes per day.
const uint32_t WAKES_PER_DAY = 10536UL;

// CRITICAL: Must use uint32_t to hold numbers above 65,535
uint32_t wakeCount = 0;
uint16_t resetDays = 15;  // Default to 15 days maximum    
uint32_t maxWakes;   // Will be calculated based on ADC reading
uint16_t adcValue;


ISR(WDT_vect) {
  wakeCount++; 
}


#if DEBUG
// --- LIGHTWEIGHT BLINK DEBUGGER ROUTINE ---
void flashNumber(uint32_t val) {
  uint32_t divisor = 100000UL;  // Handle up to 6 digits
  bool started = false;

  digitalWrite(RESET_PIN, LOW);
  delay(1500); 

  while (divisor > 0) {
    uint32_t digit = 0;
    
    while (val >= divisor) {
      val -= divisor;
      digit++;
    }

    if (digit > 0 || started || divisor == 1) {
      started = true;
      
      if (digit == 0) {
        digitalWrite(RESET_PIN, HIGH); delay(40);
        digitalWrite(RESET_PIN, LOW);  delay(300);
      } else {
        for (uint32_t i = 0; i < digit; i++) {
          digitalWrite(RESET_PIN, HIGH); delay(800);
          digitalWrite(RESET_PIN, LOW);  delay(300);
        }
      }
      delay(1500); 
    }
    divisor /= 10;
  }
  
  digitalWrite(RESET_PIN, HIGH);
  delay(1000);
}
#else
#include <util/delay.h> // Native AVR cycle-accurate delay header

#define TX_PIN PB0

// --- 2400 BAUD SERIAL TRANSMITTER ---
void send_char(char c) {
  uint8_t oldSREG = SREG;
  cli(); // Disable interrupts

  // Start Bit (LOW)
  PORTB &= ~(1 << TX_PIN);
  _delay_us(412); // 1/2400 baud = 416.67us, rounded to 408us for timing

  // 8 Data Bits
  for (uint8_t i = 0; i < 8; i++) {
    if (c & 1) PORTB |= (1 << TX_PIN);
    else       PORTB &= ~(1 << TX_PIN);
    c >>= 1;
    _delay_us(412); 
  }

  // Stop Bit (HIGH)
  PORTB |= (1 << TX_PIN);
  _delay_us(416); // Stop bit duration

  SREG = oldSREG; // Restore interrupts
}

// Flash-based String Printer
void print_str_p(const char* str) {
  char c;
  while ((c = pgm_read_byte(str++))) {
    send_char(c);
    _delay_us(50);
  }
}

// Subtraction-Based Number Printer
void print_num(uint32_t val) {
  static const uint32_t divisors[] = {100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL};
  bool started = false;
  
  for (uint8_t i = 0; i < 6; i++) {
    uint32_t d = divisors[i];
    uint8_t digit = 0;
    while (val >= d) {
      val -= d;
      digit++;
    }
    if (digit > 0 || started || i == 5) {
      started = true;
      send_char('0' + digit);
    }
  }
}
#endif

bool isOpenPin() {
  // 1. Force ADC Channel A3 (PB3) and CLEAR REFS0 (Guarantees VCC Reference)
  ADMUX = (0 << REFS0) | (3 & 0x03);
  // Step 1: Turn on internal pull-up (~35k-50k ohm)
  pinMode(DELAY_PIN, INPUT_PULLUP);
  delayMicroseconds(500); // Give pull-up time to settle pin voltage
  int pullupRead = analogRead(DELAY_PIN);
  // flashNumber(pullupRead); // Flash ADC value for debugging
  // Step 2: Turn off internal pull-up for normal ADC sampling later
  pinMode(DELAY_PIN, INPUT);
  
  // EVALUATION:
  // - Open Pin: Internal pull-up easily pulls pin to VCC -> ADC reads ~1010-1023.
  // - Resistor to GND: Resistor forms a voltage divider with the internal pull-up,
  //   pulling the voltage down significantly (< 950 even with a large 100k resistor).
  
  return (pullupRead > 620); 
}

void setup() {
  #if !DEBUG
  DDRB |= (1 << TX_PIN);   
  PORTB |= (1 << TX_PIN);  
  _delay_ms(50);
  // print_str_p(PSTR("\nStart\n"));
  #endif

  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); 

  if (isOpenPin()) {
    maxWakes = (uint32_t)resetDays * WAKES_PER_DAY;
    #if DEBUG
    delay(20);
    flashNumber(101010); // Flash maxWakes for debugging
    #else
    print_str_p(PSTR("\nOpen "));
    print_num(resetDays);
    print_str_p(PSTR("\n"));
    adcValue = analogRead(DELAY_PIN); // get ADC value for printing later
    #endif
    } else {
    // 1. Read pin value to determine resistor scaling
    adcValue = analogRead(DELAY_PIN);       
    // 2. Calculate target wakes based on adcValue and scaling factor
    maxWakes = (uint32_t)resetDays * WAKES_PER_DAY * adcValue / 1023;
    if (adcValue < 15) maxWakes = 3; // Shorted = test mode, 3 wakes before reset
    if (maxWakes < 3) maxWakes = 3; // Safeguard: Ensure at least 3 wakes before reset
    #if !DEBUG
    print_str_p(PSTR("\nScaled "));
    print_num(resetDays);
    print_str_p(PSTR("\n"));
    #endif
  }

  #if DEBUG
    delay(20);
    flashNumber(adcValue); // Flash ADC value for debugging
    // flashNumber(maxWakes); // Flash maxWakes for debugging
  #else
    print_str_p(PSTR("ADC "));
    print_num(adcValue);
    print_str_p(PSTR("\nWakes "));
    print_num(maxWakes);
    print_str_p(PSTR("\n\n"));
  #endif 
  // Turn off ADC peripherals to save power
  ADCSRA &= ~(1 << ADEN);        
  ACSR |= (1 << ACD);            

  // Watchdog Configuration
  cli();                                 
  MCUSR &= ~(1 << WDRF);                 
  WDTCR = (1 << WDCE) | (1 << WDE);      
  WDTCR = (1 << WDTIE) | (1 << WDP3) | (1 << WDP0); 
  sei();                                 
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   
}

void loop() {
  if (wakeCount >= maxWakes) {
    #if !DEBUG
    print_str_p(PSTR("Rst\n"));
    #endif
    digitalWrite(RESET_PIN, LOW);       
    delay(200);                          
    digitalWrite(RESET_PIN, HIGH);      
    
    wakeCount = 0;                      
  }

  sleep_enable();
  sleep_cpu();
  sleep_disable(); 
}