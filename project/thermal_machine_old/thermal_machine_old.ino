#include <Adafruit_MAX31856.h>
#include <AccelStepper.h>

// Define motor interface
const uint8_t M1_STEP_PIN = 3;
const uint8_t M2_STEP_PIN = 4;

const uint8_t M1_DIR_PIN = 6;
const uint8_t M2_DIR_PIN = 7;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper M1_Stepper(motorInterfaceType, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper M2_Stepper(motorInterfaceType, M2_STEP_PIN, M2_DIR_PIN);

#define mode
void setup()
{
  Serial.begin(9600);
  Serial.println("HELLO");
#ifdef mode
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  M1_Stepper.setMaxSpeed(4000);
  M1_Stepper.setAcceleration(4000);
  M1_Stepper.setSpeed(4000);
  M1_Stepper.moveTo(800);

  M2_Stepper.setMaxSpeed(2000);
  M2_Stepper.setAcceleration(2000);
  M2_Stepper.setSpeed(2000);
  M2_Stepper.moveTo(400);
#else
  M1_Stepper.setMaxSpeed(400);
  M2_Stepper.setMaxSpeed(400);

  M1_Stepper.setSpeed(45);
  M2_Stepper.setSpeed(25);
#endif
}

void loop()
{
#ifdef mode
  // Change direction once the motor reaches target position
  if (M1_Stepper.distanceToGo() == 0 && M2_Stepper.distanceToGo() == 0)
  {
    // delay(100);
    if (Serial.available())
    {
      char c;
      c = Serial.read();
      while (Serial.read() > 0)
      {
        ;
      }
      if (c == '1')
        M1_Stepper.moveTo(-M1_Stepper.currentPosition());
      else if (c == '2')
        M2_Stepper.moveTo(-M2_Stepper.currentPosition());
    }
  }
  // Move the motor one step
  M1_Stepper.run();
  // Move the motor one step
  M2_Stepper.run();
#else
  M1_Stepper.runSpeed();
  M2_Stepper.runSpeed();
#endif
}
