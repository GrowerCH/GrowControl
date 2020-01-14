#include <EEPROM.h>
#include <PWM.h>

#define ledPin 13
#define ecfanPin 4
#define fanPin 8
#define fanFrequency 25000

#define bluetooth Serial1

void setup() {
  InitTimersSafe();
  SetPinFrequencySafe(fanPin, fanFrequency);
  
  Serial.begin(9600);
  bluetooth.begin(9600);

  updateLED(loadLED());
  updateECFan(loadECFan());
  updateFan(loadFan());
}

void loop() {
  handleConsole();
  handleBluetooth();
}

void handleConsole() {
  if(Serial.available()) {
    bluetooth.write(Serial.read());
  }
}

char command[20];
boolean inProgress = false;
int i = 0;
void handleBluetooth() {
  if(bluetooth.available()) {
    char c = bluetooth.read();
    if(c == '<') {
      inProgress = true;
    } else if(c == '>') {
      command[i] = '\0';
      inProgress = false;
      i = 0;
      handleCommand();
    } else if(inProgress) {
      command[i++] = c;
    }
  }
}

void handleCommand() {
    int device = command[0] - '0';
    int power = atoi(command + 1);
    
    if(device == 0) {
      updateLED(power);
    } else if(device == 1) {
      updateECFan(power);
    } else if(device == 2) {
      updateFan(power);
    } else if(device == 9) {
      sendData();
    }
}

void sendData() {
  int led = loadLED();
  int ecfan = loadECFan();
  int fan = loadFan();
  bluetooth.print("<");
  bluetooth.print(led);
  bluetooth.print("-");
  bluetooth.print(ecfan);
  bluetooth.print("-");
  bluetooth.print(fan);
  bluetooth.println(">");
}

void updateLED(int ledPower) {
  analogWrite(ledPin, 255 - calculatePWM(ledPower));
  EEPROM.update(0, ledPower);
  Serial.print("LED is now at ");
  Serial.print(ledPower);
  Serial.println("%");
}

void updateECFan(int ecfanPower) {
  analogWrite(ecfanPin, calculatePWM(ecfanPower));
  EEPROM.update(1, ecfanPower);
  Serial.print("ECFan is now at ");
  Serial.print(ecfanPower);
  Serial.println("%");
}

void updateFan(int fanPower) {
  pwmWrite(fanPin, calculatePWM(fanPower));
  EEPROM.update(2, fanPower);
  Serial.print("Fan is now at ");
  Serial.print(fanPower);
  Serial.println("%");
}

int loadLED() {
  int ledPower = EEPROM.read(0);
  if (ledPower == 255) {
    ledPower = 40;
  }
  return ledPower;
}

int loadECFan() {
  int ecfanPower = EEPROM.read(1);
  if (ecfanPower == 255) {
    ecfanPower = 20;
  }
  return ecfanPower;
}

int loadFan() {
  int fanPower = EEPROM.read(2);
  if (fanPower == 255) {
    fanPower = 20;
  }
  return fanPower;
}

int calculatePWM(int percent) {
  return (double) percent / (double) 100 * 255;
}
