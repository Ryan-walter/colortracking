#include <Pixy2.h>
#include <stdio.h>

#include <SoftwareSerial.h>  

int bluetoothTx = 4;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 5;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);


// This is the main Pixy object
Pixy2 pixy;

struct robot_commands {
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

// Center coordinates of each tile
const int box1[2] = {31, 49};
const int box2[2] = {90, 46};
const int box3[2] = {150, 44};
const int box4[2] = {208, 42};
const int box5[2] = {260, 42};
const int box6[2] = {28, 147};
const int box7[2] = {94, 141};
const int box8[2] = {157, 126};
const int box9[2] = {218, 132};
const int box10[2] = {274, 132};

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
  
  Serial.begin(115200);
  Serial.print("Starting...\n");
  // We must initialize the pixy object
  pixy.init();
  // Getting the RGB pixel values requires the 'video' program
  pixy.changeProg("video");
  if (getSnapshot())
  {
    /*Serial.println(CommandList.box_1);
    Serial.println(CommandList.box_2);
    Serial.println(CommandList.box_3);
    Serial.println(CommandList.box_4);
    Serial.println(CommandList.box_5);
    Serial.println(CommandList.box_6); 
    Serial.println(CommandList.box_7);
    Serial.println(CommandList.box_8);
    Serial.println(CommandList.box_9);
    Serial.println(CommandList.box_10); */

    bluetooth.print((char)'S');
    bluetooth.print(CommandList.box_1);
    bluetooth.print(CommandList.box_2);
    bluetooth.print(CommandList.box_3);
    bluetooth.print(CommandList.box_4);
    bluetooth.print(CommandList.box_5);
    bluetooth.print(CommandList.box_6);
    bluetooth.print(CommandList.box_7);
    bluetooth.print(CommandList.box_8);
    bluetooth.print(CommandList.box_9);
    bluetooth.print(CommandList.box_10);
  }
}

void loop() {
  // We only want it to run once and not continuously loop
}

// Obtains all the RGB values at all coordinates and sends them to colorToCommand to determine what color and consequently what command it is.
bool getSnapshot()
{
  uint8_t r, g, b;
  bool stat = true;
  if (pixy.video.getRGB(box1[0], box1[1], &r, &g, &b) == 0)
    CommandList.box_1 = colorToCommand(r, g, b);
  if (CommandList.box_1 == 3)
    stat = false;
  if (pixy.video.getRGB(box2[0], box2[1], &r, &g, &b) == 0)
  {
    CommandList.box_2 = colorToCommand(r, g, b);
    if (CommandList.box_2 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box3[0], box3[1], &r, &g, &b) == 0)
  {
    CommandList.box_3 = colorToCommand(r, g, b);
    if (CommandList.box_3 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box4[0], box4[1], &r, &g, &b) == 0)
  {
    CommandList.box_4 = colorToCommand(r, g, b);
    if (CommandList.box_4 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box5[0], box5[1], &r, &g, &b) == 0)
  {
    CommandList.box_5 = colorToCommand(r, g, b);
    if (CommandList.box_5 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box6[0], box6[1], &r, &g, &b) == 0)
  {
    CommandList.box_6 = colorToCommand(r, g, b);
    if (CommandList.box_6 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box7[0], box7[1], &r, &g, &b) == 0)
  {
    CommandList.box_7 = colorToCommand(r, g, b);
    if (CommandList.box_7 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box8[0], box8[1], &r, &g, &b) == 0)
  {
    CommandList.box_8 = colorToCommand(r, g, b);
    if (CommandList.box_8 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box9[0], box9[1], &r, &g, &b) == 0)
  {
    CommandList.box_9 = colorToCommand(r, g, b);
    if (CommandList.box_9 == 3)
      stat = false;
  }
  if (pixy.video.getRGB(box10[0], box10[1], &r, &g, &b) == 0)
  {
    CommandList.box_10 = colorToCommand(r, g, b);
    if (CommandList.box_10 == 3)
      stat = false;
  }
  return stat;
}

// Prints RGB values of each tile to serial monitor. Determines what color each tile is based off of the RGB values and the command associated with it.
byte colorToCommand(uint8_t r, uint8_t g, uint8_t b)
{
  Serial.print(r);
  Serial.print(",");
  Serial.print(g);
  Serial.print(",");
  Serial.println(b);

  if (r > 200 && g < 180 && b < 200)
  {
    Serial.println("red");
    return 0;
  }
  else if (r < 200 && g < 200 && b > 200)
  {
    Serial.println("blue");
    return 1;
  }
  else if (r > 200 && g > 180 && b < 200)
  {
    Serial.println("yellow");
    return 2;
  }
  else
    return 3;
}
