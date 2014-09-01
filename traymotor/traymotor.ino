/*Listing DVD-Laufwerk Schubladenmotor für Arduino Pro Mini (ATMEGA328P) 
Datum: 2014-08-29
Autor: MaxPower 
Lizenz: Creative Commons Attribution 4.0 International (CC BY 4.0)http://creativecommons.org/licenses/by-nc/4.0/ 
*/ 
 
const int buttonPin = 2; 
const int trayOpenPin = 3; 
const int trayClosedPin = 4; 
const int motorEnablePin = 5; 
const int motorRunOpenPin = 9; 
const int motorRunClosePin = 10; 
const int ledPin = 13;
const int motorVoltagePin = A0;

const int trayDebounceDelay = 50; 
const int buttonDebounceDelay = 50; 
const int trayMoveErrorDelay = 5000; 
const int trayReverseDelay = 500; 
const int motorRunOpenDutyCycle = 128;
const int motorRunCloseDutyCycle = 80;
 
boolean ledState = LOW; 
boolean buttonState; 
boolean buttonPinState; 
boolean lastButtonPinState = LOW; 
boolean trayOpenState; 
boolean trayOpenPinState; 
boolean lastTrayOpenPinState = LOW; 
boolean trayClosedState; 
boolean trayClosedPinState; 
boolean lastTrayClosedPinState = LOW; 
 
boolean trayErrorState = LOW; 
boolean trayOpenCommand = LOW; 
boolean trayCloseCommand = LOW; 
boolean trayHaltCommand = HIGH; 
boolean motorRun = LOW; 
boolean motorRunOpen = LOW; 
boolean motorRunClose = LOW; 

int motorVoltage;
 
unsigned long lastButtonDebounceTime; 
unsigned long lastTrayOpenDebounceTime; 
unsigned long lastTrayClosedDebounceTime; 
unsigned long trayCommandTime; 
 
void setup() { 
  //start serial connection 
  Serial.begin(9600); 
 
  pinMode(buttonPin, INPUT); 
  pinMode(trayOpenPin, INPUT); 
  pinMode(trayClosedPin, INPUT); 
  pinMode(ledPin, OUTPUT); 
  pinMode(motorRunOpenPin, OUTPUT); 
  pinMode(motorRunClosePin, OUTPUT); 
 
} 
 
 
 
void loop() { 
 
  buttonPinState = digitalRead(buttonPin); 
  trayOpenPinState = digitalRead(trayOpenPin); 
  trayClosedPinState = digitalRead(trayClosedPin);
  
  motorVoltage = analogRead(motorVoltagePin);
 
  //Button debouncen 
  if (buttonPinState != lastButtonPinState) { 
    lastButtonDebounceTime = millis(); 
  } 
  if ((millis() - lastButtonDebounceTime) > buttonDebounceDelay) { 
    if (buttonPinState != buttonState) { 
      buttonState = buttonPinState; 
    } 
  } 
 
  //Endschalter offen debouncen 
  if (trayOpenPinState != lastTrayOpenPinState) { 
    lastTrayOpenDebounceTime = millis(); 
  } 
  if ((millis() - lastTrayOpenDebounceTime) > trayDebounceDelay) { 
    if (trayOpenPinState != trayOpenState) { 
      trayOpenState = trayOpenPinState; 
    } 
  } 
 
  //Endschalter geschlossen debouncen 
  if (trayClosedPinState != lastTrayClosedPinState) { 
    lastTrayClosedDebounceTime = millis(); 
  } 
  if ((millis() - lastTrayClosedDebounceTime) > trayDebounceDelay) { 
    if (trayClosedPinState != trayClosedState) { 
      trayClosedState = trayClosedPinState; 
    } 
  } 
 
  //Variablen mit Pin-Zuständen als Vergleichswert für nächsten Schleifendurchgang schreiben 
  lastButtonPinState = buttonPinState; 
  lastTrayOpenPinState = trayOpenPinState; 
  lastTrayClosedPinState = trayClosedPinState; 
 
 
  //Logische Verknüpfungen Ausgangszustand 
  if (motorRunOpen == HIGH || motorRunClose == HIGH) { 
    motorRun = HIGH; 
  } 
  else { 
    motorRun = LOW; 
  } 
  if (buttonState == HIGH && trayClosedState == HIGH && motorRun == LOW ) { 
    trayOpenCommand = HIGH; 
  } 
  if (buttonState == HIGH && trayOpenState == HIGH && motorRun == LOW) { 
    trayCloseCommand = HIGH; 
  } 
  if (trayOpenState == LOW && trayClosedState == LOW && motorRun == LOW && trayErrorState == LOW) { 
    trayCloseCommand = HIGH; 
  } 
  if (buttonState == HIGH && trayOpenState == LOW && trayClosedState == LOW && motorRun == LOW && trayErrorState == HIGH) { 
    trayOpenCommand = HIGH; 
  } 
 
  //Logische Verknüpfungen Motor-Kommandos 
 
  if (trayOpenCommand == HIGH) { 
    trayCommandTime = millis(); 
    motorRunOpen = HIGH; 
    digitalWrite(motorEnablePin, HIGH); 
    digitalWrite(motorRunClosePin, LOW); 
    analogWrite(motorRunOpenPin, motorRunOpenDutyCycle); 
    trayOpenCommand = LOW; 
 
  } 
  if (trayCloseCommand == HIGH) { 
    trayCommandTime = millis(); 
    motorRunClose = HIGH; 
    digitalWrite(motorEnablePin, HIGH); 
    analogWrite(motorRunClosePin, motorRunCloseDutyCycle); 
    digitalWrite(motorRunOpenPin, LOW); 
    trayCloseCommand = LOW; 
  } 
 
  //Abschalten im Fehlerfall 
  if (motorRun == HIGH && (millis() - trayCommandTime) > trayMoveErrorDelay) { 
    trayHaltCommand = HIGH; 
    trayErrorState = HIGH; 
  } 
 
  //Umschaltung Öffnen und Schließen während Schublade läuft 
  if (buttonState == HIGH && motorRunOpen == HIGH && (millis() - trayCommandTime) > trayReverseDelay) { 
    trayCloseCommand = HIGH; 
 
  } 
  if (buttonState == HIGH && motorRunClose == HIGH && (millis() - trayCommandTime) > trayReverseDelay) { 
    trayOpenCommand = HIGH; 
  } 
 
  //Logische Verknüpfung Endschalter 
  if (motorRunOpen == HIGH && trayOpenState == HIGH) { 
    trayHaltCommand = HIGH; 
    trayErrorState = LOW; 
  } 
 
  if (motorRunClose == HIGH && trayClosedState == HIGH) { 
    trayHaltCommand = HIGH; 
    trayErrorState = LOW; 
  } 
  //STOP 
  if (trayHaltCommand == HIGH) { 
    digitalWrite(motorRunClosePin, LOW); 
    digitalWrite(motorRunOpenPin, LOW); 
    digitalWrite(motorEnablePin, LOW); 
    motorRunOpen = LOW; 
    motorRunClose = LOW; 
    trayHaltCommand = LOW; 
  } 
 
  /* 
    //print out the values 
    Serial.print(buttonState); 
    Serial.print("\t"); 
    Serial.print(trayOpenState); 
    Serial.print("\t"); 
    Serial.print(trayClosedState); 
    Serial.print("\t"); 
    Serial.print(motorRunOpen); 
    Serial.print("\t"); 
    Serial.print(motorRunClose); 
    Serial.print("\t"); 
    Serial.print(trayErrorState); 
    Serial.print("\t"); 
    Serial.print(trayCommandTime); 
    Serial.print("\t"); 
    Serial.print(millis()); 
    Serial.print("\t"); 
  */ 
    Serial.println(motorVoltagePin); 
   
    if (buttonState == HIGH) { 
      digitalWrite(ledPin, HIGH); 
    } 
    else { 
      digitalWrite(ledPin, LOW); 
    } 
 
} 
 
 
 
