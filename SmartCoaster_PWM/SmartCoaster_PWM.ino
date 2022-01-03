#define DEBUG_serial true		// serial print messages

#define RPIN 2
#define GPIN 3
#define BPIN 4
#define BUTTON 22
#define ADJUSTMENT 55

int counter = 0;

int red = 255;
int green = 255;
int blue = 255;

int adjustmentValue = 0;
int buttonValue = 0;
bool buttonDown = false;

int thresh_min=100;
int thresh_mid=150;
int thresh_max=200;

void setup() {

  #if DEBUG_serial
	Serial.begin(9600);
  #endif // DEBUG
  
  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

}

void loop() {

  buttonValue = digitalRead(BUTTON);
  adjustmentValue = analogRead(ADJUSTMENT);
  int adjustmentRange = map(adjustmentValue, 0, 1024, 1, 25);

#if DEBUG_serial
  Serial.print("ButtonValue: ");
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
  //delay(100* adjustmentRange);
  delay(1);

}


void evaluateColors() {

  if (buttonDown == true && counter >= thresh_min && counter <= thresh_mid) {
    red = 0;
    green = 0;
    blue = counter-thresh_min;
  } else if (buttonDown == true && counter > thresh_mid && counter <= thresh_max) {
    red = counter-thresh_mid;
    green = 0;
    blue = thresh_max-counter;
  } else if (buttonDown == true && counter > thresh_max && counter <= thresh_max+50) {
  } else if (buttonDown == true && counter > thresh_max+50) {
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
