#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// --- ATtiny85 / ATtiny13A Compatibility Patch ---
#ifndef WDTIE
#define WDTIE WDIE  // If WDTIE doesn't exist (like on the '85), use WDIE instead
#endif

const int RESET_PIN = 4;
// const int RESET_PIN = PB0;     // Pin 5 on both physical chips
unsigned int wakeCount = 0;    // Tracks our 8-second intervals

ISR(WDT_vect) {
  wakeCount++; 
}

void setup() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); 

  // Disable unnecessary internal peripherals
  ADCSRA &= ~(1 << ADEN);        // Turn off ADC
  ACSR |= (1 << ACD);            // Turn off Analog Comparator
  
  // Configure Watchdog Timer for Interrupt Mode + 8-Second timeout
  MCUSR &= ~(1 << WDRF);                 
  WDTCR |= (1 << WDCE) | (1 << WDE);     
  WDTCR = (1 << WDTIE) | (1 << WDP3) | (1 << WDP0); // Patch handles WDTIE automatically
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   
}

void loop() {
  if (wakeCount >= 1) {
    // --- 24 HOURS PASSED: TRIGGER THE RESET ---
    digitalWrite(RESET_PIN, LOW);       
    delay(50);                          
    digitalWrite(RESET_PIN, HIGH);      
    
    wakeCount = 0;                      
  }

  sleep_enable();
  sleep_cpu();
  sleep_disable(); 
}