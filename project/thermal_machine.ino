#include <AccelStepper.h>

// Define motor interface
const uint8_t M1_STEP_PIN = 2;
const uint8_t M2_STEP_PIN = 3;

const uint8_t M1_DIR_PIN = 5;
const uint8_t M2_DIR_PIN = 6;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper M1_Stepper(motorInterfaceType, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper M2_Stepper(motorInterfaceType, M1_STEP_PIN, M1_DIR_PIN);

void setup()
{
    /*
    // set the maximum speed, acceleration factor,
    // initial speed and the target position
    M1_Stepper.setMaxSpeed(1000);
    M1_Stepper.setAcceleration(50);
    M1_Stepper.setSpeed(200);
    M1_Stepper.moveTo(200);
    */
    M1_Stepper.setMaxSpeed(400);
    M2_Stepper.setMaxSpeed(400);

    M1_Stepper.setSpeed(45);
    M2_Stepper.setSpeed(25);
}

void loop()
{
    /*
    // Change direction once the motor reaches target position
    if (M1_Stepper.distanceToGo() == 0)
        M1_Stepper.moveTo(-M1_Stepper.currentPosition());

    // Move the motor one step
    M1_Stepper.run();
    */
    M1_Stepper.runSpeed();
    M2_Stepper.runSpeed();
}
