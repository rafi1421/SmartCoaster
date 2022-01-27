
#define Board_DigiSpark true

#if Board_DigiSpark
  //DigiSpark Pins
  #define RPIN 0
  #define GPIN 1
  #define BPIN 4
  #define BUTTON_PIN 2
  byte BUTTON_INT = 2;  // however arduino reference says now to use -> digitalPinToInterrupt(BUTTON_PIN). However this function doesnt exist on digispark?
  #define DEBUG_serial false    // serial print messages
#else
  //Arduino Mega Pins
  #define RPIN 2
  #define GPIN 3
  #define BPIN 4
  #define BUTTON_PIN 21
  byte BUTTON_INT = 2; // however arduino reference says now to use -> digitalPinToInterrupt(BUTTON_PIN).
  #define DEBUG_serial true    // serial print messages
#endif 

// delay(x) where x=1000=1second. 1000*60=60000=1minute. 60000/50(50=default_delay)=1200
//actually, max delay is 32476. so lets divide by 2 for now and dowuble
#define DELAY_MULTIPLIER 1

int counter = 0;

int red = 255;
int green = 255;
int blue = 255;

int adjustmentValue = 0;
int buttonValue = 0;
bool buttonDown = false;

#define thresh_min 25
#define thresh_mid 40
#define thresh_max 60
#define thresh_max_hold 50
#define LEDMAXBRIGHT 250


//// Watchdog intervals
//// sleep bit patterns for WDTCSR/
enum {
  WDT_16_MS = 0b000000,
  WDT_32_MS = 0b000001,
  WDT_64_MS = 0b000010,
  WDT_128_MS = 0b000011,
  WDT_256_MS = 0b000100,
  WDT_512_MS = 0b000101,
  WDT_1_SEC = 0b000110,
  WDT_2_SEC = 0b000111,
  WDT_4_SEC = 0b100000,
  WDT_8_SEC = 0b100001,
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

  // PowerOn glow for fun
  fadeOn(BPIN,50);
  fadeOn(GPIN,50);
  fadeOut(BPIN,50);
  fadeOut(GPIN,50);
}

void loop() {

  buttonValue = digitalRead(BUTTON_PIN);
//  int maxconstrain = constrain(counter,0,60);
//  int adjustmentRange = map(maxconstrain, 0, 60, 1, 255);

#if DEBUG_serial
//  Serial.print("adjustmentRange: ");
//  Serial.print(adjustmentRange);
  
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
    counter++;
    #if DEBUG_serial
    Serial.println("+++ Button Pressed. Starting Watchdog +++ ");
    delay(100);
    #endif
    EnableWatchdog(WDT_128_MS);
    GoToSleep(SLEEP_MODE_IDLE);
    #if DEBUG_serial
    Serial.println("--- Watchdog woke up --- ");
    delay(100);
    #endif
  } else if (buttonValue == LOW && buttonDown == true) {
    counter ++;
    #if DEBUG_serial
    Serial.println("--- Button Still Pressed. Watchdog keeping guard --- ");
    delay(100);
    #endif
    EnableWatchdog(WDT_128_MS);
    GoToSleep(SLEEP_MODE_IDLE);
  } else if (buttonValue == HIGH && buttonDown == true) {
    counter = 0;
    buttonDown = false;
    #if DEBUG_serial
    Serial.println("+++ Button Raised. Clearing counter +++ ");
    delay(100);
    #endif
  } else if (buttonValue == HIGH && buttonDown == false) {
    counter++;
    #if DEBUG_serial
    Serial.println("--- Button Not Pressed. Going into pwd_down --- ");
    delay(100);
    #endif
    // Enable ISR and Sleep
    //attachInterrupt(BUTTON_INT, WakeUp, CHANGE); //digispark doesnt support this method
    EnablePinChangeInt(BUTTON_INT);
    GoToSleep();
    DisablePinChangeInt();
    //detachInterrupt(BUTTON_INT);  //digispark doesnt support this method
    #if DEBUG_serial
    Serial.println("--- Button Pressed Down -> Woke up from pwd_down --- ");
    delay(100);
    #endif
  }

  evaluateColors();
  updateLights();
//  delay(50* DELAY_MULTIPLIER);  //Serialprint creates a delay, so adjust delay if not using it. 
//  delay(50* DELAY_MULTIPLIER);
}


void evaluateColors() {
//  if (buttonDown == true && counter == thresh_min) {
//    fadeOn(BPIN, thresh_min);
//  } else 
  if (buttonDown == true && counter > thresh_min && counter <= thresh_mid) { //blue
    int adjustmentRange = map(counter, thresh_min, thresh_mid, 0, LEDMAXBRIGHT);
    red = 0;
    green = 0;
    blue = adjustmentRange;
  } else if (buttonDown == true && counter > thresh_mid && counter <= thresh_max) { //blue to purple to red
    int adjustmentRange_r = map(counter, thresh_mid, thresh_max, 1, LEDMAXBRIGHT);
    int adjustmentRange_b = map(counter, thresh_mid, thresh_max, LEDMAXBRIGHT, 0);
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
    analogWrite(GPIN, green);
    analogWrite(BPIN, blue);

}

void fadeOn(int LEDPIN, int LedBrightness) {
      int fadeLed;
      for (int x = 0; x < LedBrightness; x++) {
        fadeLed = .015*x*x; // Final value is [80=96]; [73=80], close to old version
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
}
