#include <SoftwareSerial.h>  

int bluetoothTx = 4;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 5;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

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

robot_commands CommandList;

void setup()
{
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
}

void loop()
{
  if(bluetooth.available())  // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    //Serial.print((char)bluetooth.read());
    char check = (char)bluetooth.read();
    if (check == 'S')
    {
      char buff[10];
      for(int x = 0; x < 10; x++)
      {
        buff[x] = (char)bluetooth.read();
      }
    }
    else if (check == '0')
    {
      Serial.println("Turn Left");
    }
    else if (check == '1')
    {
      Serial.println("Move Forward");
    }
    else if (check == '2')
    {
      Serial.println("Turn Right");
    }
  }
  if(Serial.available())  // If stuff was typed in the serial monitor
  {
    // Send any characters the Serial monitor prints to the bluetooth
    bluetooth.print((char)Serial.read());
  }
  // and loop forever and ever!
}
