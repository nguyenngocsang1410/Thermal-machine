#include <Adafruit_MAX31856.h>
#include <AccelStepper.h>
#include <SPI.h>

// Define motor interface
const uint8_t M1_STEP_PIN = 4; // Y
const uint8_t M2_STEP_PIN = 2; // Z

const uint8_t M1_DIR_PIN = 7;
const uint8_t M2_DIR_PIN = 5;

// Define motor interface type
#define motorInterfaceType 1
// Creates an instance
AccelStepper M1_Stepper(motorInterfaceType, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper M2_Stepper(motorInterfaceType, M2_STEP_PIN, M2_DIR_PIN);

#define DRDY_PIN 3
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10);

float tempe_buffer = 0;

float v1_max = 1; // rps
float v1 = 0.25;
float a1 = 0.25;
float v2_max = 1;
float v2 = 0.5;
float a2 = 0.25;

#define stepPerRev 200
#define M1_MODE 16
#define M2_MODE 16
uint32_t step1_Total = stepPerRev * M1_MODE;
uint32_t step2_Total = stepPerRev * M2_MODE;

void motorSetup()
{
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  M1_Stepper.setMaxSpeed(v1_max * step1_Total);
  M1_Stepper.setAcceleration(a1 * step1_Total);
  M1_Stepper.setSpeed(v1 * step1_Total);

  M2_Stepper.setMaxSpeed(v2_max * step2_Total);
  M2_Stepper.setAcceleration(a2 * step2_Total);
  M2_Stepper.setSpeed(v2 * step2_Total);
}
char handleSerial()
{
  if (Serial.available() > 0)
  {
    char c = Serial.read();

    clearSerialInput();
    // Serial.flush();
    // Serial.println(c);  // echo back
    return c;
  }
  return 0;
}
void clearSerialInput()
{
  while (Serial.read() > 0)
    ;
}
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  // Serial.println("HELLO");
  motorSetup();
  M2_Stepper.setCurrentPosition(0);
  M1_Stepper.setCurrentPosition(0);

  pinMode(DRDY_PIN, INPUT);

  if (!maxthermo.begin())
  {
    while (1)
    {
      Serial.println("Could not initialize thermocouple.");
      delay(100);
      break;
    }
  }
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_T);
  maxthermo.setConversionMode(MAX31856_CONTINUOUS);

  attachInterrupt(digitalPinToInterrupt(DRDY_PIN), readSensor, FALLING);
}
char c;
bool sendFlag = 0, continueMode = 0;

void loop()
{
  c = handleSerial();
  if (c == 'a')
    M1_IN();
  else if (c == 'b')
    M1_OUT();
  else if (c == 'c')
    M2_IN();
  else if (c == 'd')
    M2_OUT();
  else if (c == 't')
    tempRead();

  else if (c == 'f')
    continueMode = true;
  else if (c == 'g')
    continueMode = false;

  else
    c = 0;
  runMotor();
  if (continueMode)
  {
    if (sendFlag)
    {
      Serial.println(tempe_buffer);
    }
  }
}

bool M1_run_CMD = false, M2_run_CMD = false;
const float kp1 = 1, ki1 = 0, kd1 = 0;
const float kp2 = 1, ki2 = 0, kd2 = 0;
long e1_pre = 0, e2_pre = 0;
long e1_sum = 0, e2_sum = 0;
// #define pid_tuning
// #define modify
bool slowDown = false;
long M2_lastPos = 0;
float a2_slow = 0.25;
float v2_slow = 0.25;
void runMotor()
{

  if (M1_Stepper.distanceToGo() != 0)
  {
    M1_Stepper.run();
    // Serial.println(M1_Stepper.distanceToGo());
  }
  else
    M1_Stepper.stop();
  if (M2_Stepper.distanceToGo() != 0)
  {
    M2_Stepper.run();
    // Serial.println(M1_Stepper.distanceToGo());
  }
  else
    M2_Stepper.stop();
#ifdef pid_tuning
  if (M1_run_CMD)
  {
    long e1 = M1_Stepper.distanceToGo();
    long e1_delta = e1 - e1_pre;
    e1_sum += e1_delta;
    float u1 = kp1 * e1 + ki1 * e1_sum + kd1 * e1_delta;
    M1_Stepper.setSpeed(u1);
  }
  // else M1_Stepper.stop();

  if (M2_run_CMD)
  {
    long e2 = M2_Stepper.distanceToGo();
    long e2_delta = e2 - e2_pre;
    e2_sum += e2_delta;
    float u2 = kp2 * e2 + ki2 * e2_sum + kd2 * e2_delta;
    M2_Stepper.setSpeed(u2);
  }
  // else M2_Stepper.stop();
#endif
}
void M1_IN()
{
  M1_Stepper.moveTo(0);
#ifdef pid_tuning
  e1_pre = M1_Stepper.distanceToGo();
  e1_sum = 0;
  M1_run_CMD = true;
#endif
}
void M1_OUT()
{
  M1_Stepper.moveTo(step1_Total / 4.0);
#ifdef pid_tuning
  e1_pre = M1_Stepper.distanceToGo();
  e1_sum = 0;
  M1_run_CMD = true;
#endif
}
void M2_IN()
{
  M2_Stepper.moveTo(0);
#ifdef pid_tuning
  e2_pre = M2_Stepper.distanceToGo();
  e2_sum = 0;
  M2_run_CMD = true;
#endif
}
void M2_OUT()
{
  M2_Stepper.moveTo(-(step2_Total / 4.0));
#ifdef pid_tuning
  e2_pre = M2_Stepper.distanceToGo();
  e2_sum = 0;
  M2_run_CMD = true;
#endif
}
void tempRead()
{
  tempe_buffer = maxthermo.readThermocoupleTemperature();
  Serial.print(tempe_buffer);
  Serial.flush();
}
void readSensor()
{
  tempe_buffer = maxthermo.readThermocoupleTemperature();
  sendFlag = 1;
}
