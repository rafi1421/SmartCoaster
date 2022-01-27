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
  #if Board_DigiSpark
    WDTCR = _BV(WDCE) | _BV(WDE);
  #else
    WDTCSR = _BV(WDCE) | _BV(WDE);
  #endif
  // set interrupt mode and an interval
  #if Board_DigiSpark
    WDTCR = _BV(WDIE) | interval;    // set WDIE, and requested delay
  #else
    WDTCSR = _BV(WDIE) | interval;    // set WDIE, and requested delay
  #endif
  wdt_reset();  // pat the dog
}
ISR(WDT_vect)
{
  wdt_disable();  // disable watchdog
}

void EnablePinChangeInt(byte const ShiftBy) {
//  sensorActive = false; 
  // Pin Change Interrupt setup
  GIMSK = 0b00100000;               // turns on pin change interrupts
  PCMSK = 0b00000001 << ShiftBy;    // turn on interrupts on pins PB#
}
void DisablePinChangeInt() {
  // Pin Change Interrupt setup
  GIMSK = 0b00000000;    // turns off pin change interrupts
  PCMSK = 0b00000000;    // turn off interrupts on pins PB#
}


ISR(PCINT0_vect)
{
    //sensorActive = true;             // Increment volatile variable
}


//
// Well apparently attchInterrupt() & detachInterrupt() work for arduino & trinket, but not the digispark. So need to use PCINT, which is fine for my use.
// Maybe those functions can be replaced with more hardware specific code.
//
// Use:   attachInterrupt(LightSensorInt, WakeUp, LOW); // HW INT0
void WakeUp() {
  //maybe leave this empty here so i dont need to change the pin variable in future projects, and just use this line directly in the code where is resumed.
  //detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
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
  #ifndef Board_DigiSpark 
  sleep_bod_disable();
  #endif
  interrupts();
  sleep_mode();
  // --- end timed sleep sequence (order of events matter) --- //
  // - WAKEUP FROM SLEEP - //
  sleep_disable();
  //noInterrupts(); //Whoa, so actually having this here is what was causing the problem "reseting" during/after sleep!
  power_all_enable();
  ADCSRA = old_ADCSRA;  //return ACD enabled
}
