#include <Pixy2.h>
#include <PIDLoop.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>

Pixy2 pixy;
ZumoMotors motors;
ZumoBuzzer buzzer;
PIDLoop panLoop(400, 0, 400, true);
PIDLoop tiltLoop(500, 0, 500, true);

#define leftSpeed 200
#define rightSpeed 200


void setup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");

  stopMotors();
 
  // We need to initialize the pixy object 
  pixy.init();
  // Use color connected components program for the pan tilt to track 
  pixy.changeProg("color_connected_components");
  pixy.setLamp(1,0);
  pixy.setServos(500,200);
  /*motors.setLeftSpeed(leftSpeed);
  motors.setRightSpeed(-rightSpeed);
  delay(500);
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);*/
}

void loop()
{  
  static int i = 0;
  int j;
  char buf[64]; 
  int32_t panOffset, tiltOffset;
  int bnum = 0;
  Block blue, red, purple;
  
  // get active blocks from Pixy
  pixy.ccc.getBlocks();
  
  if (pixy.ccc.numBlocks)
  {        
     
    // calculate pan and tilt "errors" with respect to first object (blocks[0]), 
    // which is the biggest object (they are sorted by size).  
    panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
    tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  
  
    // update loops
    panLoop.update(panOffset);
    tiltLoop.update(tiltOffset);

    if(bnum < 3)
    {
      if((int32_t)pixy.ccc.blocks[bnum].m_signature == 1)
        blue = pixy.ccc.blocks[bnum];
  
      if((int32_t)pixy.ccc.blocks[bnum].m_signature == 2)
        red = pixy.ccc.blocks[bnum];

      if((int32_t)pixy.ccc.blocks[bnum].m_signature == 3)
        purple = pixy.ccc.blocks[bnum];
      
        
      bnum++;
    }

    if(blue.m_x < red.m_x)
    {
      Serial.println("Right");
      /*motors.setLeftSpeed(leftSpeed);
      motors.setRightSpeed(-rightSpeed);
      delay(500);
      stopMotors();*/
    }

    if(blue.m_x < red.m_x < purple.m_x)
    {
      Serial.println("purple biggest");
    }
  
    if(red.m_x < blue.m_x)
    {
      Serial.println("Left");
      /*motors.setLeftSpeed(-leftSpeed);
      motors.setRightSpeed(rightSpeed);
      delay(500);
      stopMotors();*/
    }
      

    /*Serial.println("Blue:");
    Serial.print("x: ");
    Serial.print(blue.m_x);
    Serial.print("   ");
    Serial.print("y: ");
    Serial.println(blue.m_y);
    Serial.println("Red:");
    Serial.print("x: ");
    Serial.print(red.m_x);
    Serial.print("   ");
    Serial.print("y: ");
    Serial.println(red.m_y);
    Serial.println("-----------------------");*/
  
    // set pan and tilt servos  
    //pixy.setServos(panLoop.m_command, tiltLoop.m_command);
   
#if 0 // for debugging
    sprintf(buf, "%ld %ld %ld %ld", rotateLoop.m_command, translateLoop.m_command, left, right);
    Serial.println(buf);   
#endif

  }  
  /*else // no object detected, go into reset state
  {
    panLoop.reset();
    tiltLoop.reset();
    pixy.setServos(panLoop.m_command, tiltLoop.m_command);
  }*/
}

void stopMotors()
{
  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);
}
