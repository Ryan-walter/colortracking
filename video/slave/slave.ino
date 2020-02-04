// Necessary for bluetooth connection
#include <SoftwareSerial.h>

int bluetoothTx = 4;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 5;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

// Necessary for robot
#include <Pixy2.h>
#include <PIDLoop.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>

#define ZUMO_FAST        190
#define ZUMO_SLOW        140
#define TURN_SPEED       174
#define X_CENTER         (pixy.frameWidth/2)
int intrsectcounter = 0;
int instruct = 3;
bool intr = false;

Pixy2 pixy;
ZumoMotors motors;
ZumoBuzzer buzzer;

PIDLoop headingLoop(5000, 0, 0, false);

struct robot_commands
{
  byte box_1;
  byte box_2;
  byte box_3;
  byte box_4;
  byte box_5;
  byte box_6;
  byte box_7;
  byte box_8;
  byte box_9;
  byte box_10;
};

robot_commands CommandList;

void setup()
{
  // Set up for bluetooth communication
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.println("CFR"); // This command causes the device to connect and immediately go into fast data mode using the stored remote address (master)
  bluetooth.begin(9600);  // Start bluetooth serial at 9600


  //Set up for robot to execute commands
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  Serial.print("Starting...\n");
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);
  pixy.init();
  pixy.setLamp(1, 1); // Turn on both lamps, upper and lower for maximum exposure
  pixy.changeProg("line"); // change to the line_tracking program.
  pixy.setServos(500, 850); // look straight and down
  pixy.line.setDefaultTurn(-90);
}

void loop()
{
  if (bluetooth.available()) // If the bluetooth sent any characters
  {
    // Checks any characters the bluetooth prints
    char check = (char)bluetooth.read();
    if (check == 'S') // Denotes start marker for recieved commands
    {
      char buff[10];
      for (int x = 0; x < 10; x++)
      {
        buff[x] = (char)bluetooth.read();
      }
    }
    else if (check == '0') // Checks if it received command to turn left
    {
      Serial.println("Left");
      instruct = 0;
      left();
      delay(2000);

    }
    else if (check == '1') // Checks if it received command to continue forward.
    {
      Serial.println("Forward");
      instruct = 1;
      forward();
      delay(2000);

    }
    else if (check == '2') // Checks if it received command to turn right
    {
      Serial.println("Right");
      instruct = 2;
      right();
      delay(2000);
    }
  }
  if (Serial.available()) // If stuff was typed in the serial monitor
  {
    // Send any characters the Serial monitor prints to the bluetooth
    bluetooth.print((char)Serial.read());
  }

  // and loop forever and ever!
}

void forward()
{
  int8_t res;
  int32_t error;
  int left, right;
  char buf[96];
  res = pixy.line.getMainFeatures();
  if (res <= 0) // If it does not see a line in front, it does nothing and returns.
  {
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);
    Serial.print("I see nothing. ");
    Serial.println(res);
    return;
  }

  if (res & LINE_VECTOR) // Follows a line and corrects itself as neccessary.
  {
    error = (int32_t)pixy.line.vectors->m_x1 - (int32_t)X_CENTER;
    pixy.line.vectors->print();
    headingLoop.update(error);
    left = headingLoop.m_command;
    right = -headingLoop.m_command;

    Serial.print("Following a line. ");
    Serial.println(res);

    if (pixy.line.vectors->m_y0 > pixy.line.vectors->m_y1) // Checks if y-coord of head is less than tail, which means vector is pointing forward.
    {
      if (pixy.line.vectors->m_flags & LINE_FLAG_INTERSECTION_PRESENT) // Slows down when intersection present.
      {
        left += ZUMO_SLOW;
        right += ZUMO_SLOW;
        Serial.println("Slowing down for intersection.");
        intr = true;
      }
      else // Else continues forward.
      {
        left += ZUMO_FAST;
        right += ZUMO_FAST;
      }
    }
    else  // If the vector is pointing down, or down-ish, we need to go backwards to follow.
    {
      left -= ZUMO_SLOW;
      right -= ZUMO_SLOW;
      Serial.println("Going backwards.");
    }
    motors.setLeftSpeed(left);
    motors.setRightSpeed(right);
    delay(200);

    if (intr)
    {
      delay(250);
      instruct == 3; // Resets intruct variable
      stopMoving(); // Stops motors
      intr = false;
      return;
    }
    else
    forward();
  }
}

void right() // Function to turn right
{
  Serial.println("Turning right.");
  motors.setLeftSpeed(TURN_SPEED);
  motors.setRightSpeed(-TURN_SPEED);
  delay(600);
  keepTurning(instruct);
  instruct == 3; // Resets intruct variable
  stopMoving(); // Stops motors
  return;
}

void left() // Function to turn left
{
  Serial.println("Turning left");
  motors.setLeftSpeed(-TURN_SPEED);
  motors.setRightSpeed(TURN_SPEED);
  delay(600);
  keepTurning(instruct);
  instruct == 3; // Resets intruct variable
  stopMoving(); // Stops motors
  return;
}

void stopMoving() // Function to stop motors
{
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);
  Serial.println("Stopped motors.");
}

void keepTurning(int x) // Function meant to override parts of code so robot behaves differently when turning.
{
  int8_t res;
  int direc = x;
  res = pixy.line.getMainFeatures();

  while (res <= 0) // While turning, if the Pixy camera sees nothing, robot will continue turning until it sees something (a line).
  {
    res = pixy.line.getMainFeatures();
    Serial.println(res);
    if (direc == 0) // Checks if previously turning left.
    {
      Serial.println("Continuing left turn.");
      motors.setLeftSpeed(-TURN_SPEED);
      motors.setRightSpeed(TURN_SPEED);
    }
    if (direc == 2) // Checks if previously turning right.
    {
      Serial.println("Continuing right turn.");
      motors.setLeftSpeed(TURN_SPEED);
      motors.setRightSpeed(-TURN_SPEED);
    }
  }

  if (res & LINE_VECTOR) // Robot will turn 90 degrees before it stops.
  {
    if (direc == 0) // Checks if previously turning left.
    {
      while (pixy.line.vectors->m_x1 < 39) // Continues to turn until tail cordinate of vector is greater than the coordinate for the head.
      { // Signifies that robot fully turned 90 degrees.
        res = pixy.line.getMainFeatures();
        Serial.println("Finishing left turn.");
        motors.setLeftSpeed(-TURN_SPEED);
        motors.setRightSpeed(TURN_SPEED);
      }
    }
    if (direc == 2) // Checks if previously turning left.
    {
      while (39 < pixy.line.vectors->m_x1) // Continues to turn until tail cordinate of vector is less than the coordinate for the head.
      { // Signifies that robot fully turned 90 degrees.
        res = pixy.line.getMainFeatures();
        Serial.println("Finishing right turn.");
        motors.setLeftSpeed(TURN_SPEED);
        motors.setRightSpeed(-TURN_SPEED);
        //Serial.println(pixy.line.vectors->m_x0);
        //Serial.println(pixy.line.vectors->m_x1);
      }
    }
    stopMoving();
    delay(250);
    Serial.println("Finished.");
    return;
  }
}
