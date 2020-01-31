#include <SoftwareSerial.h>

#include <Pixy2.h>
#include <PIDLoop.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>

#define ZUMO_FAST        200
#define ZUMO_SLOW        150
#define X_CENTER         (pixy.frameWidth/2)
int intrsectcounter = 0;
int instruct = 3;

Pixy2 pixy;
ZumoMotors motors;
ZumoBuzzer buzzer;

PIDLoop headingLoop(5000, 0, 0, false);

int bluetoothTx = 4;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 5;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

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
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600


  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  Serial.print("Starting...\n");
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);
  pixy.init();
  // Turn on both lamps, upper and lower for maximum exposure
  pixy.setLamp(1, 1);
  // change to the line_tracking program.  Note, changeProg can use partial strings, so for example,
  // you can change to the line_tracking program by calling changeProg("line") instead of the whole
  // string changeProg("line_tracking")
  pixy.changeProg("line");
  // look straight and down
  pixy.setServos(500, 850);
  pixy.line.setDefaultTurn(-90);
}

void loop()
{
  if (bluetooth.available()) // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    //Serial.print((char)bluetooth.read());
    char check = (char)bluetooth.read();
    if (check == 'S')
    {
      char buff[10];
      for (int x = 0; x < 10; x++)
      {
        buff[x] = (char)bluetooth.read();
      }
    }
    else if (check == '0')
    {
      Serial.println("Left");
      instruct = 0;
      left();
      delay(1000);
      
    }
    else if (check == '1')
    {
      Serial.println("Forward");
      instruct = 1;
      forward();
      delay(1000);
      
    }
    else if (check == '2')
    {
      Serial.println("Right");
      instruct = 2;
      right();
      delay(1000);
      
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

  if (res <= 0)
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

    if (pixy.line.vectors->m_y0 > pixy.line.vectors->m_y1) // Cehcks if y-coord of head is less than tail, which means vector is pointing forward.
    {
      if (pixy.line.vectors->m_flags & LINE_FLAG_INTERSECTION_PRESENT) // Slows down when intersection present.
      {
        left += ZUMO_SLOW;
        right += ZUMO_SLOW;
        Serial.println("Slowing down for intersection.");
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
  }

  delay(5000);

  instruct == 3;
  stopMoving();
}

void right()
{
  Serial.println("Turning right.");
  motors.setLeftSpeed(175);
  motors.setRightSpeed(-175);
  delay(500);
  keepTurning(instruct);
  instruct == 3;
  stopMoving();
}

void left()
{
  Serial.println("Turning left");
  motors.setLeftSpeed(-175);
  motors.setRightSpeed(175);
  delay(550);
  keepTurning(instruct);
  instruct == 3;
  stopMoving();
}

void stopMoving() // Function created to stop motors
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
      motors.setLeftSpeed(-175);
      motors.setRightSpeed(175);
    }
    if (direc == 2) // Checks if previously turning right.
    {
      Serial.println("Continuing right turn.");
      motors.setLeftSpeed(175);
      motors.setRightSpeed(-175);
    }
  }

  if (res & LINE_VECTOR) // Robot will turn 90 degrees before it stops.
  {
    if (direc == 0) // Checks if previously turning left.
    {
      while (pixy.line.vectors->m_x0 > pixy.line.vectors->m_x1) // Continues to turn until tail cordinate of vector is greater than the coordinate for the head.
      { // Signifies that robot fully turned 90 degrees.
        Serial.println("Finishing left turn.");
        motors.setLeftSpeed(-175);
        motors.setRightSpeed(175);
      }
    }
    if (direc == 2) // Checks if previously turning left.
    {
      while (pixy.line.vectors->m_x0 > pixy.line.vectors->m_x1) // Continues to turn until tail cordinate of vector is less than the coordinate for the head.
      { // Signifies that robot fully turned 90 degrees.
        Serial.println("Finishing right turn.");
        motors.setLeftSpeed(175);
        motors.setRightSpeed(-175);
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
