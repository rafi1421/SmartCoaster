// Trinket Sleep code

#include <avr/sleep.h> // Needed for sleep_mode
#include <avr/wdt.h> // Needed to enable/disable watchdog timer
#include <avr/power.h> // Needed to enable/disable power modes
#include <avr/interrupt.h>




void EnableWatchdog() {
  EnableWatchdog(WDT_1_SEC);
}
void EnableWatchdog(const byte interval) {
  MCUSR = 0;
  // allow changes, disable reset
  WDTCR = _BV(WDCE) | _BV(WDE);
  // set interrupt mode and an interval
  WDTCR = _BV(WDIE) | interval;    // set WDIE, and requested delay
  wdt_reset();  // pat the dog
}
ISR(WDT_vect)
{
  wdt_disable();  // disable watchdog
}

void EnablePinChangeInt() {
  sensorActive = false; 
  // Pin Change Interrupt setup
  GIMSK = 0b00100000;    // turns on pin change interrupts
  PCMSK = 0b00000001;    // turn on interrupts on pins PB0
}
void DisablePinChangeInt() {
  // Pin Change Interrupt setup
  GIMSK = 0b00000000;    // turns off pin change interrupts
  PCMSK = 0b00000000;    // turn off interrupts on pins PB0
}
/*
void EnablePin2ChangeInt() {
  sensorActive = false; 
  // Pin Change Interrupt setup
  GIMSK = 0b00100000;    // turns on pin change interrupts
  PCMSK = 0b00000100;    // turn on interrupts on pins PB0
}
void DisablePin2ChangeInt() {
  // Pin Change Interrupt setup
  GIMSK = 0b00000000;    // turns off pin change interrupts
  PCMSK = 0b00000000;    // turn off interrupts on pins PB0
}
*/
ISR(PCINT0_vect)
{
    //sensorActive = true;             // Increment volatile variable
}

  // Flag to indicate that the sensors have been triggered, 
  // so that it will run the cod. Because I am using interrupts, i had to
  // structure the code so that the function is available but will only run when triggered.
  // If i tried to turn on the leds via the interrupt function, the chip would go back to 
  // sleep because its running the previous code from where it left off, and sleep before the led function finishes.

// Use:   attachInterrupt(LightSensorInt, wakeLight, LOW); // HW INT0
void WakeLight() {
  detachInterrupt(LightSensorInt);
}

void GoToSleep() {
  GoToSleep(SLEEP_MODE_PWR_DOWN);
}
void GoToSleep(const byte mode) {
  // disable ADC
  byte old_ADCSRA = ADCSRA;
  ADCSRA = 0;

  // turn off various modules
  if (mode == SLEEP_MODE_PWR_DOWN) {
    power_all_disable();
  }
  else {
    // These keep PWM while IDLE sleeping
    // Reduces power from 5.7mA to 3.4mA (led at 23% pwm)
    power_adc_disable();
    power_timer0_disable();
  }
  
  // --- start timed sleep sequence (order of events matter) --- //
  noInterrupts();
  sleep_enable();
  set_sleep_mode (mode);
  sleep_bod_disable();
  interrupts();
  sleep_mode();
  // --- end timed sleep sequence (order of events matter) --- //
  // - WAKEUP FROM SLEEP - //
  sleep_disable();
  //noInterrupts(); //Whoa, so actually having this here is what was causing the problem "reseting" during/after sleep!
  power_all_enable();
  ADCSRA = old_ADCSRA;  //return ACD enabled
}
