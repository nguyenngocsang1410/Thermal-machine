#include <Wire.h> //This is for i2C
#include <SSD1306Ascii.h> //i2C OLED
#include <SSD1306AsciiWire.h> //i2C OLED

#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiWire oled;
float OLEDTimer = 0; //Timer for the display refresh interval
//I2C pins:
//STM32F103C8T6: SDA: PB7 SCL: PB6
//Arduino: SDA: A4 SCL: A5
//---------------------------------------------------------------------------

//Input and output pins
//Motor encoder
const int encoderPin1 = 3; //this pin is also the interrupt pin!
const int encoderPin2 = 4; //this pin is a normal pin, read upon the interrupt
int encoderPin2Value = 0; //value of the encoder pin (0 or 1), this pin is read inside the interrupt - used for direction determination
//----------------------------------
//PWM motor driver
const int PWMPin = 11; //this is an analog pin (with the tilde (~) symbol), this pin can also do higher frequency + independent from millis()'s timer
int PWMValue = 0; //0-255 PWM value for speed, external PWM boards can go higher (e.g. PCA9685: 12-bit => 0-4095)
const int directionPin1 = 10; //digital pin, output, sets the direction
const int directionPin2 = 9; //digital pin, output, sets the direction
const int standByPin = 6; //STDBY pin, must be active high. Stops the H-bridge.
int motorDirection = 0; //direction value 0: CCW, 1: CW. - Stored value
//----------------------------------
//Rotary encoder
const int RotaryCLK = 2; //CLK pin on the rotary encoder, interrupt pin!
const int RotaryDT = 7; //DT pin on the rotary encoder, read inside the interrupt
const int RotarySW = 8; //SW pin on the rotary encoder (Button function)
int RotaryButtonValue = 0; //0 or 1 (pressed or not)
float RotaryTime; //timer for debouncing
volatile int rotaryValue = 0; //value manipulated by the encoder
int previousRotaryValue = -1; //a variable which stores the previous value - easy to follow changes
//----------------------

//Target values - Also called as setpoint!
float targetPosition = 0; //the PID will try to reach this value

//-----------------------
//Measured values
volatile float motorPosition = 0; //position based on the encoder
float previousMotorPosition = -1; //helps to keep track of changes (useful for the display update)
//-----------------------------------
//-----------------------------------
//PID parameters - tuned by the user
float proportional = 1.35; //k_p = 0.5
float integral = 0.00005; //k_i = 3
float derivative = 0.01; //k_d = 1
float controlSignal = 0; //u - Also called as process variable (PV)
//-----------------------------------
//-----------------------------------
//PID-related
float previousTime = 0; //for calculating delta t
float previousError = 0; //for calculating the derivative (edot)
float errorIntegral = 0; //integral error
float currentTime = 0; //time in the moment of calculation
float deltaTime = 0; //time difference
float errorValue = 0; //error
float edot = 0; //derivative (de/dt)

//Statuses of the DT and CLK pins on the encoder
int CLKNow;
int CLKPrevious;
int DTNow;
int DTPrevious;

void setup()
{
  Serial.begin(115200);
  Wire.begin(); //start i2C
  Wire.setClock(800000L); //faster clock

  //Motor encoder-related
  pinMode(encoderPin1, INPUT); //A
  pinMode(encoderPin2, INPUT); //B
  attachInterrupt(digitalPinToInterrupt(encoderPin1), checkEncoder, RISING);
  pinMode(standByPin, OUTPUT); //

  //Definition of the pins, remember where you need
  pinMode(RotaryCLK, INPUT_PULLUP); //CLK
  pinMode(RotaryDT, INPUT_PULLUP); //DT
  pinMode(RotarySW, INPUT_PULLUP); //SW
  attachInterrupt(digitalPinToInterrupt(RotaryCLK), RotaryEncoder, CHANGE);
  //Store states
  CLKPrevious = digitalRead(RotaryCLK);
  DTPrevious = digitalRead(RotaryDT);

  //------------------------------------------------------------------------------
  //OLED part
#if RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);
  oled.clear(); //clear display
  oled.set2X(); //
  oled.println("   PID    "); //print some welcome message
  oled.println("Controller");
  oled.set1X();
  delay(1000);
  OLEDTimer = millis(); //start the timer
  oled.clear();
  displayPermanentItems();
  refreshDisplay();
}

void loop()
{
  calculatePID();

  driveMotor();

  //printValues();

  CheckRotaryButton();

  refreshDisplay();
}

void checkEncoder()
{
  //We need to read the other pin of the encoder which will be either 1 or 0 depending on the direction
  encoderPin2Value = digitalRead(encoderPin2);

  if (encoderPin2Value == 1) //CW direction
  {
    motorPosition++;
  }
  else //else, it is zero... -> CCW direction
  {
    motorPosition--;
  }
}

void driveMotor()
{
  //Determine speed and direction based on the value of the control signal
  //direction
  if (controlSignal < 0) //negative value: CCW
  {
    motorDirection = -1;
  }
  else if (controlSignal > 0) //positive: CW
  {
    motorDirection = 1;
  }
  else //0: STOP - this might be a bad practice when you overshoot the setpoint
  {
    motorDirection = 0;
  }
  //---------------------------------------------------------------------------
  //Speed
  PWMValue = (int)fabs(controlSignal); //PWM values cannot be negative and have to be integers
  if (PWMValue > 255) //fabs() = floating point absolute value
  {
    PWMValue = 255; //capping the PWM signal - 8 bit
  }

  if (PWMValue < 30 && errorValue != 0)
  {
    PWMValue = 30;
  }
  //A little explanation for the "bottom capping":
  //Under a certain PWM value, there won't be enough current flowing through the coils of the motor
  //Therefore, despite the fact that the PWM value is set to the "correct" value, the motor will not move
  //The above value is an empirical value, it depends on the motors perhaps, but 30 seems to work well in my case

  //we set the direction - this is a user-defined value, adjusted for TB6612FNG driver
  if (motorDirection == -1) //-1 == CCW
  {
    digitalWrite(directionPin1, LOW);
    digitalWrite(directionPin2, HIGH);
  }
  else if (motorDirection == 1) // == 1, CW
  {
    digitalWrite(directionPin1, HIGH);
    digitalWrite(directionPin2, LOW);
  }
  else // == 0, stop/break
  {
    digitalWrite(directionPin1, LOW);
    digitalWrite(directionPin2, LOW);
    digitalWrite(standByPin, LOW);
    PWMValue = 0;
    //In this block we also shut down the motor and set the PWM to zero
  }
  //----------------------------------------------------
  //Then we set the motor speed
  analogWrite(PWMPin, PWMValue);

  //Optional printing on the terminal to check what's up
  /*
    Serial.print(errorValue);
    Serial.print(" ");
    Serial.print(PWMValue);
    Serial.print(" ");
    Serial.print(targetPosition);
    Serial.print(" ");
    Serial.print(motorPosition);
    Serial.println();
  */
}

void calculatePID()
{
  //Determining the elapsed time
  currentTime = micros(); //current time
  deltaTime = (currentTime - previousTime) / 1000000.0; //time difference in seconds
  previousTime = currentTime; //save the current time for the next iteration to get the time difference
  //---
  errorValue = motorPosition - targetPosition; //Current position - target position (or setpoint)

  edot = (errorValue - previousError) / deltaTime; //edot = de/dt - derivative term

  errorIntegral = errorIntegral + (errorValue * deltaTime); //integral term - Newton-Leibniz, notice, this is a running sum!

  controlSignal = (proportional * errorValue) + (derivative * edot) + (integral * errorIntegral); //final sum, proportional term also calculated here

  previousError = errorValue; //save the error for the next iteration to get the difference (for edot)
}

void printValues()
{
  //Serial.print("Position: ");
  Serial.println(motorPosition);
}

void displayPermanentItems()
{
  //print the permanent items on the display
  oled.setCursor(0, 0); //(x [pixels], y[lines])
  oled.print("Target");

  oled.setCursor(0, 2);
  oled.print("Position");
}

void refreshDisplay()
{
  if (millis() - OLEDTimer > 100) //check if we will update at every 100 ms
  {
    if (previousRotaryValue != rotaryValue)
    {
      oled.setCursor(0, 1);
      oled.print("      ");
      oled.setCursor(0, 1);
      oled.print(rotaryValue); //print the target value set by the rotary encoder

      previousRotaryValue = rotaryValue;
      OLEDTimer = millis(); //reset timer
    }

    if (motorPosition != previousMotorPosition)
    {
      oled.setCursor(0, 3);
      oled.print("      ");
      oled.setCursor(0, 3);
      oled.print(motorPosition, 0); //print the new absolute position

      previousMotorPosition = motorPosition;
      OLEDTimer = millis(); //reset timer
    }
  }
  else
  {
    //skip
  }
}
void RotaryEncoder()
{
  CLKNow = digitalRead(RotaryCLK); //Read the state of the CLK pin
  // If last and current state of CLK are different, then a pulse occurred
  if (CLKNow != CLKPrevious  && CLKNow == 1)
  {
    if (digitalRead(RotaryDT) != CLKNow) //the increment/decrement can depend on the actual polarity of CLK and DT
    {
      rotaryValue = rotaryValue - 515 ; //1
    }
    else
    {
      rotaryValue = rotaryValue + 515 ; //1
    }
  }
  CLKPrevious = CLKNow;  // Store last CLK state
}

void CheckRotaryButton()
{
  RotaryButtonValue = digitalRead(RotarySW); //read the button state

  if (RotaryButtonValue == 0) //0 and 1 can differ based on the wiring
  {
    if (millis() - RotaryTime > 1000)
    {
      targetPosition = rotaryValue; //pass the setpoint (target value) to the corresponding variable
      digitalWrite(standByPin, HIGH); //enable motor driver
      RotaryTime = millis(); //save time
    }
  }
}