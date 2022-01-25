
#define Board_DigiSpark true

#if Board_DigiSpark
  //DigiSpark Pins
  #define RPIN 0
  #define GPIN 1
  #define BPIN 4
  #define BUTTON 2
  #define DEBUG_serial false    // serial print messages
#else
  //Arduino Mega Pins
  #define RPIN 2
  #define GPIN 3
  #define BPIN 4
  #define BUTTON 22
  #define DEBUG_serial true    // serial print messages
#endif 

// delay(x) where x=1000=1second. 1000*60=60000=1minute. 60000/50(50=default_delay)=1200
//actually, max delay is 32476. so lets divide by 2 for now and dowuble
#define DELAY_MULTIPLIER 600

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

void setup() {

  #if DEBUG_serial
	Serial.begin(115200);
  #endif // DEBUG

  
  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  // PowerOn glow for fun
  fadeOn(BPIN,50);
  fadeOn(GPIN,50);
  fadeOut(BPIN,50);
  fadeOut(GPIN,50);
}

void loop() {

  buttonValue = digitalRead(BUTTON);
  int maxconstrain = constrain(counter,0,60);
  int adjustmentRange = map(maxconstrain, 0, 60, 1, 255);

#if DEBUG_serial
  Serial.print("adjustmentRange: ");
  Serial.print(adjustmentRange);
  
  Serial.print(" ButtonValue: ");
  Serial.print(buttonValue);
  Serial.print(", Button: ");
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
  } else if (buttonValue == LOW && buttonDown == true) {
    counter ++;
  } else if (buttonValue == HIGH && buttonDown == true) {
    counter = 0;
    buttonDown = false;
  } else if (buttonValue == HIGH && buttonDown == false) {
    counter++;
  }

  evaluateColors();
  updateLights();
  delay(50* DELAY_MULTIPLIER);  //Serialprint creates a delay, so adjust delay if not using it. 
  delay(50* DELAY_MULTIPLIER);
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
