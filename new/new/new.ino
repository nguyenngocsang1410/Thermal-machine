#include <Adafruit_MAX31856.h>
#include <SPI.h>

#define DRDY_PIN 3
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10);
float tempe_buffer = 0;

char handleSerial() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    // Serial.println(c,HEX);  // echo back
    clearSerialInput();
    return c;
  }
  return 0;
}
void clearSerialInput() {
  while (Serial.read() > 0)
    ;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  pinMode(DRDY_PIN, INPUT_PULLUP);
  if (!maxthermo.begin()) {
    Serial.println("Could not initialize thermocouple");
    delay(1000);
  }
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_T);
  maxthermo.setConversionMode(MAX31856_CONTINUOUS);

  attachInterrupt(digitalPinToInterrupt(DRDY_PIN), readSensor, FALLING);
}

char c = 0, sendFlag = 0;
bool continueMode = false;
void loop() {
  c = handleSerial();
  switch (c) {
    case 'a':
      break;
    case 'b':
      break;
    case 'c':
      break;
    case 'd':
      break;

    case 't':
      break;

    case 'f':
      continueMode = true;
      break;
    case 'g':
      continueMode = false;
      break;
    default:
      c = 0;
  }

  if (continueMode) {
    if (sendFlag) {
      Serial.println(tempe_buffer);
      sendFlag = false;
    }
  }
}

void tempRead() {
  if (maxthermo.conversionComplete()) {
    tempe_buffer = maxthermo.readThermocoupleTemperature();
  }
  Serial.print(tempe_buffer);
  Serial.flush();
}

void readSensor() {
  tempe_buffer = maxthermo.readThermocoupleTemperature();
  sendFlag = true;
}