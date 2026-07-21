#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef WDTIE
#define WDTIE WDIE  
#endif

const int RESET_PIN = 4;        // Physical Leg 3 (PB4) -> Reset output

// CRITICAL: Must use uint32_t to hold numbers above 65,535
uint32_t wakeCount = 0;    
uint32_t maxWakes = 75600UL;    // 7 days default (75600UL)

ISR(WDT_vect) {
  wakeCount++; 
}

// --- LIGHTWEIGHT BLINK DEBUGGER ROUTINE ---
// void flashNumber(uint32_t val) {
//   uint32_t divisor = 100000UL;  // Handle up to 6 digits
//   bool started = false;

//   digitalWrite(RESET_PIN, LOW);
//   delay(500); 

//   while (divisor > 0) {
//     uint32_t digit = 0;
    
//     while (val >= divisor) {
//       val -= divisor;
//       digit++;
//     }

//     if (digit > 0 || started || divisor == 1) {
//       started = true;
      
//       if (digit == 0) {
//         digitalWrite(RESET_PIN, HIGH); delay(25);
//         digitalWrite(RESET_PIN, LOW);  delay(100);
//       } else {
//         for (uint32_t i = 0; i < digit; i++) {
//           digitalWrite(RESET_PIN, HIGH); delay(100);
//           digitalWrite(RESET_PIN, LOW);  delay(50);
//         }
//       }
//       delay(500); 
//     }
//     divisor /= 10;
//   }
  
//   digitalWrite(RESET_PIN, HIGH);
//   delay(1000);
// }

void setup() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); 

  pinMode(3, INPUT_PULLUP);     
  delay(20);                    

  // FIX 1: Pass A3 instead of plain integer 3 to satisfy MicroCore's analog_pin_t
  int16_t adcValue = analogRead(A3);  

  // Turn off ADC peripherals to save power
  ADCSRA &= ~(1 << ADEN);        
  ACSR |= (1 << ACD);            

  // FIX 2: Explicit 32-bit math using 'UL' (Unsigned Long) constants
  maxWakes = 1 + (((uint32_t)adcValue * 75599UL) / 1023);

  // Scaled clamping logic
  if (maxWakes < 1000) {
    maxWakes = 2; 
  } else if (maxWakes > 45000) {
    maxWakes = 75600UL; // 168 hours
  }

  // Debug output
  //flashNumber(maxWakes);

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
    digitalWrite(RESET_PIN, LOW);       
    delay(200);                          
    digitalWrite(RESET_PIN, HIGH);      
    
    wakeCount = 0;                      
  }

  sleep_enable();
  sleep_cpu();
  sleep_disable(); 
}