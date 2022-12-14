# 1 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\continous\\max31856_continuous.ino"
// This example demonstrates continuous conversion mode using the
// DRDY pin to check for conversion completion.

# 5 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\continous\\max31856_continuous.ino" 2



// Use software SPI: CS, DI, DO, CLK
// Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("MAX31856 thermocouple test");

  pinMode(3, 0x0);

  if (!maxthermo.begin())
  {
    Serial.println("Could not initialize thermocouple.");
    while (1)
      delay(10);
  }

  maxthermo.setThermocoupleType(MAX31856_TCTYPE_T);

  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType())
  {
  case MAX31856_TCTYPE_B:
    Serial.println("B Type");
    break;
  case MAX31856_TCTYPE_E:
    Serial.println("E Type");
    break;
  case MAX31856_TCTYPE_J:
    Serial.println("J Type");
    break;
  case MAX31856_TCTYPE_K:
    Serial.println("K Type");
    break;
  case MAX31856_TCTYPE_N:
    Serial.println("N Type");
    break;
  case MAX31856_TCTYPE_R:
    Serial.println("R Type");
    break;
  case MAX31856_TCTYPE_S:
    Serial.println("S Type");
    break;
  case MAX31856_TCTYPE_T:
    Serial.println("T Type");
    break;
  case MAX31856_VMODE_G8:
    Serial.println("Voltage x8 Gain mode");
    break;
  case MAX31856_VMODE_G32:
    Serial.println("Voltage x8 Gain mode");
    break;
  default:
    Serial.println("Unknown");
    break;
  }

  maxthermo.setConversionMode(MAX31856_CONTINUOUS);
}

void loop()
{
  // The DRDY output goes low when a new conversion result is available
  int count = 0;
  while (digitalRead(3))
  {
    if (count++ > 200)
    {
      count = 0;
      Serial.print(".");
    }
  }
  Serial.println(maxthermo.readThermocoupleTemperature());
}
