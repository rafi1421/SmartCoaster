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

#if Board_DigiSpark
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
#endif

ISR(PCINT0_vect)
{
    skipNap = true;             // Increment volatile variable
}


//
// Well apparently attchInterrupt() & detachInterrupt() work for arduino & trinket, but not the digispark. So need to use PCINT, which is fine for my use.
// Maybe those functions can be replaced with more hardware specific code.
//
// Use:   attachInterrupt(LightSensorInt, WakeUp, LOW); // HW INT0
void WakeUp() {
  skipNap = true;
  //maybe leave this empty here so i dont need to change the pin variable in future projects, and just use this line directly in the code where is resumed.
  //detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
}

//void GoToSleep() {
//  GoToSleep(SLEEP_MODE_PWR_DOWN);
//}
void GoToSleep(const byte mode) {
  // disable ADC
  byte old_ADCSRA = ADCSRA;
  ADCSRA = 0;

  // turn off various modules
  if (mode == SLEEP_MODE_PWR_DOWN) {
    power_all_disable();
  }
  else {
    power_adc_disable();
    // These keep PWM while IDLE sleeping
    #if Board_DigiSpark
       // DigiSpark/Digistump appears to set or use this register(bit 2) for timer overflow ISR which was causing the CPU to immediately wake up from sleep while using PMW.
      //  This turns off that bit and allows PWM to work while in SLEEP_MODE_IDLE
      int v_TIMSK = TIMSK;
      bitClear(v_TIMSK,2);
      TIMSK =v_TIMSK;
      
      power_timer1_disable();
      //power_timer0_disable(); // WELL APARENTLY this is what was screwing up causing the led's to flicker on the digispark! but it wouldnt flicker on the arduino! Strange! Probably uses different timers or modes or something like that.
      //// im guessing pin 4 runs on timer 1 while pins 0 & 1 run on timer 0. in that case since im only using blue and red, i can swap the green pin with blue so i can disable clock 1 and keep clock 0 only without problem so i can at least save some power.
      //// side note, that power draw warning on windows only happens when a timer is disabled? could it being off while trying to toggle the pin cause an internal short somewhere? interesting.
      //// back to that flicker issue, ran a test only disabling timer 1 while keeping timer 0 alive, so R&B pwm can work but G wont, then after 30 seconds, i got the windows error, and now its flickering white, because in the code i have all 3 going. so its definetly got to do with the internals & registers that is different in the digispark compiler compared to trinket/arduino. Well at least in a plain pwm/sleep sketch, stillshows that windows error in this project.
      #endif
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
