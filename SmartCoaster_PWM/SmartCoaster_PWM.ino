#include <avr/power.h> // Needed to enable/disable power modes

#if defined(ARDUINO_AVR_MEGA2560) 
  #define Board_DigiSpark false
#else 
  #define Board_DigiSpark true
#endif

#if Board_DigiSpark
  //DigiSpark Pins
  #define RPIN 0
  #define GPIN 4
  #define BPIN 1
  #define BUTTON_PIN 2
  byte BUTTON_INT = 2;  // however arduino reference says now to use -> digitalPinToInterrupt(BUTTON_PIN). However this function doesnt exist on digispark?
  #define DEBUG_serial false    // serial print messages
#else
  //Arduino Mega Pins
  #define RPIN 5
  #define GPIN 6
  #define BPIN 7
  #define BUTTON_PIN 21
  byte BUTTON_INT = 2; // however arduino reference says now to use -> digitalPinToInterrupt(BUTTON_PIN).
  #define DEBUG_serial true    // serial print messages
#endif 


int counter = 0;

int red = 255;
int green = 255;
int blue = 255;
int blue_old = 0;
int red_old = 0;

int adjustmentValue = 0;
int buttonValue = 0;
bool buttonDown = false;
bool skipNap = false;

#define thresh_min 25
#define thresh_mid 40
#define thresh_max 60
#define thresh_max_hold 50
#define LEDMAXBRIGHT 200


//// Watchdog intervals
//// sleep bit patterns for WDTCSR/
enum {
  WDT_16_MS  = 0b000000,
  WDT_32_MS  = 0b000001,
  WDT_64_MS  = 0b000010,
  WDT_128_MS = 0b000011,
  WDT_256_MS = 0b000100,
  WDT_512_MS = 0b000101,
  WDT_1_SEC  = 0b000110,
  WDT_2_SEC  = 0b000111,
  WDT_4_SEC  = 0b100000,
  WDT_8_SEC  = 0b100001,
};  // end of WDT intervals enum



void setup() {

  #if DEBUG_serial
	Serial.begin(115200);
  //Serial.print(" BUTTON_PIN to Intterupt: ");
  //Serial.println(digitalPinToInterrupt(BUTTON_PIN));
  #endif // DEBUG

  
  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(GPIN,0);
  
  // PowerOn glow for fun
  fadeOn(BPIN,50);
  fadeOn(RPIN,50);
  fadeOut(BPIN,50);
  fadeOut(RPIN,50);
  
  power_adc_disable(); // wont need adc in a normal operation #include power.h
}

void loop() {

  buttonValue = digitalRead(BUTTON_PIN);

#if DEBUG_serial
  Serial.print(" ButtonValue: ");
  Serial.print(buttonValue);
  Serial.print(", ButtonDown: ");
  Serial.print(buttonDown);  
  Serial.print(", Counter: ");
  Serial.print(counter);
  Serial.print(", red: ");
  Serial.print(red);
  Serial.print(", green: ");
  Serial.print(green);
  Serial.print(", blue: ");
  Serial.println(blue);
#endif
  
  if (buttonValue == LOW && buttonDown == false) {
    counter = 0;
    buttonDown = true;
    //counter++;
    #if DEBUG_serial
      Serial.println("+++ Button Pressed. Starting Watchdog +++ ");
      delay(10);
    #endif
  } 
  else if (buttonValue == LOW && buttonDown == true) {
    counter ++;
    #if DEBUG_serial
      Serial.println("--- Button Still Pressed. Watchdog keeping guard --- ");
      delay(10);
    #endif
    
    //Enable ISR to allow wake up during watchdog and skip the reset of the sleep cycles in the routine with skipNap.
    #if Board_DigiSpark
      EnablePinChangeInt(BUTTON_INT);
    #else
      attachInterrupt(BUTTON_INT, WakeUp, CHANGE); //digispark doesnt support this method
    #endif
    // Repeat 8 second watchdog to elapse a minute's length. 
    // Should be 7.5 times for a minute, but i think because of the reduced 1KHz clock, it runs a bit slower? So eight 8 second sleep cycles covers close to 58 seconds on my board.
    for (int x = 0; x < 7; x++) {
      if (!skipNap) {
        EnableWatchdog(WDT_8_SEC);
        GoToSleep(SLEEP_MODE_IDLE);
      } else {
          #if DEBUG_serial
            Serial.println(" *.* Skipping Nap .*. ");
            delay(10);
          #endif
      }
    }
//    if (!skipNap) {
//      EnableWatchdog(WDT_4_SEC);
//      GoToSleep(SLEEP_MODE_IDLE);
//      #if DEBUG_serial
//        Serial.print(" // ");
//        Serial.print(counter);
//        Serial.println(" minutes elapsed // ");
//        delay(10);
//      #endif
//    }
    #if Board_DigiSpark
      DisablePinChangeInt();
    #else
      detachInterrupt(BUTTON_INT);  //digispark doesnt support this method
    #endif
    
  }
  else if (buttonValue == HIGH && buttonDown == true) {
    counter = 0;
    buttonDown = false;
    #if DEBUG_serial
      Serial.println("+++ Button Raised. Clearing counter +++ ");
      delay(10);
    #endif
  }
  else if (buttonValue == HIGH && buttonDown == false) {
    counter++;
    #if DEBUG_serial
      Serial.println("--- Button Not Pressed. Going into pwd_down --- ");
      delay(10);
    #endif
    
    // Enable ISR and Sleep
    #if Board_DigiSpark
      EnablePinChangeInt(BUTTON_INT);
    #else
      attachInterrupt(BUTTON_INT, WakeUp, CHANGE); //digispark doesnt support this method
    #endif    
    GoToSleep(SLEEP_MODE_PWR_DOWN);
    #if Board_DigiSpark
      DisablePinChangeInt();
    #else
      detachInterrupt(BUTTON_INT);  //digispark doesnt support this method
    #endif
    
    #if DEBUG_serial
      Serial.println("--- Button Pressed Down -> Woke up from pwd_down --- ");
      delay(10);
    #endif
  }
  
  skipNap = false;
  evaluateColors();
  updateLights();
}


void evaluateColors() {
  if (buttonDown == true && counter > thresh_min && counter <= thresh_mid) { //blue
    int adjustmentRange = map(counter, thresh_min, thresh_mid, 0, LEDMAXBRIGHT);
    fader(BPIN, blue_old, adjustmentRange, 700);
    red = 0;
    green = 0;
    blue = adjustmentRange;
  } else if (buttonDown == true && counter > thresh_mid && counter <= thresh_max) { //blue to purple to red
    int adjustmentRange_r = map(counter, thresh_mid, thresh_max, 1, LEDMAXBRIGHT);
    int adjustmentRange_b = map(counter, thresh_mid, thresh_max, LEDMAXBRIGHT, 0);
    fader(RPIN, red_old, adjustmentRange_r, 200);
    red = adjustmentRange_r;
    green = 0;
    blue = adjustmentRange_b;
  } else if (buttonDown == true && counter > thresh_max && counter <= thresh_max+thresh_max_hold) { //hold red for a bit
  } else if (buttonDown == true && counter > thresh_max+thresh_max_hold) { //turn off leds
    red = 0;
    green = 0;
    blue = 0;
  }

  if (buttonDown == false) {
    red = 0;
    green = 0;
    blue = 0;
  }

}

void updateLights() {
    analogWrite(RPIN, red);
    //analogWrite(GPIN, green);
    analogWrite(BPIN, blue);
    blue_old = blue;
    red_old = red;
}

void fadeOn(int LEDPIN, int LedBrightness) {
    int fadeLed;
    for (int x = 0; x < LedBrightness; x++) {
      fadeLed = .015*x*x;
      analogWrite(LEDPIN, fadeLed);
      delay(30);
    }
}
void fadeOut(int LEDPIN, int LedBrightness) {
    int fadeLed;
    for (int x = LedBrightness; x > 0; x--) {
      fadeLed = .015*x*x; 
      analogWrite(LEDPIN, fadeLed);
      delay(10);
    }
    digitalWrite(LEDPIN, LOW); //turn off pwm bits
}
void fader(int LEDPIN, int LedLast, int LedTo, int incDelay) {
    for (int x = LedLast; x < LedTo; x++) {
      analogWrite(LEDPIN, x);
      delay(incDelay);
  }
}
