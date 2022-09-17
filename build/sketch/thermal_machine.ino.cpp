#include <Arduino.h>
#line 1 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
#include <Adafruit_MAX31856.h>
#include <AccelStepper.h>
#include <SPI.h>

// Define motor interface
const uint8_t M1_STEP_PIN = 3; // Y
const uint8_t M2_STEP_PIN = 4; // Z

const uint8_t M1_DIR_PIN = 5;
const uint8_t M2_DIR_PIN = 7;

// Define motor interface type
#define motorInterfaceType 1
#define M1_MODE 32
#define M2_MODE 16
// Creates an instance
AccelStepper M1_Stepper(motorInterfaceType, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper M2_Stepper(motorInterfaceType, M2_STEP_PIN, M2_DIR_PIN);

#define DRDY_PIN 2
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10);

float tempe_buffer = 0;

#line 25 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void motorSetup();
#line 37 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
char handleSerial();
#line 51 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void setup();
#line 77 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void loop();
#line 107 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void runMotor();
#line 112 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void M1_IN();
#line 116 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void M1_OUT();
#line 120 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void M2_IN();
#line 124 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void M2_OUT();
#line 128 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void tempRead();
#line 133 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void readSensor();
#line 25 "e:\\Documents\\Lab\\Thermal machine\\Thermal-machine\\project\\thermal_machine\\thermal_machine.ino"
void motorSetup()
{
    // set the maximum speed, acceleration factor,
    // initial speed and the target position
    M1_Stepper.setMaxSpeed(4000);
    M1_Stepper.setAcceleration(8000);
    M1_Stepper.setSpeed(4000);

    M2_Stepper.setMaxSpeed(200);
    M2_Stepper.setAcceleration(400);
    M2_Stepper.setSpeed(200);
}
char handleSerial()
{
    if (Serial.available() > 0)
    {
        char c = Serial.read();
        // Serial.println(c); // echo back

        // clearSerialInput();
        Serial.flush();
        return c;
    }
    return 0;
}

void setup()
{
    Serial.begin(115200);
    motorSetup();
    // M2_Stepper.setCurrentPosition();
    // M1_Stepper.setCurrentPosition();

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

    attachInterrupt(digitalPinToInterrupt(2), readSensor, FALLING);
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
void runMotor()
{
    M1_Stepper.run();
    M2_Stepper.run();
}
void M1_IN()
{
    M1_Stepper.move(0);
}
void M1_OUT()
{
    M1_Stepper.move(M1_MODE*(100/4));
}
void M2_IN()
{
    M2_Stepper.move(0);
}
void M2_OUT()
{
    M2_Stepper.move(-M2_MODE*(100/4));
}
void tempRead()
{
    Serial.print(tempe_buffer);
    Serial.flush();
}
void readSensor()
{
    tempe_buffer = maxthermo.readThermocoupleTemperature();
    sendFlag = 1;
}
