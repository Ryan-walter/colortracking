#include <Pixy2.h>
#include <PIDLoop.h>

Pixy2 pixy;
PIDLoop panLoop(400, 0, 400, true);
PIDLoop tiltLoop(500, 0, 500, true);

void setup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");
 
  // We need to initialize the pixy object 
  pixy.init();
  // Use color connected components program for the pan tilt to track 
  pixy.changeProg("color_connected_components");
  pixy.setLamp(1,1);
  pixy.setServos(500,200);
}

void loop()
{  
  static int i = 0;
  int j;
  char buf[64]; 
  int32_t panOffset, tiltOffset;
  int bnum = 0;
  Block blue, red;
  
  // get active blocks from Pixy
  pixy.ccc.getBlocks();
  
  if (pixy.ccc.numBlocks)
  {        
    i++;
    
    if (i%60==0)
      Serial.println(i);   
    
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
        red = pixy.ccc.blocks[bnum];
  
      if((int32_t)pixy.ccc.blocks[bnum].m_signature == 2)
        blue = pixy.ccc.blocks[bnum];
        
      bnum++;
    }

    /*Serial.println("First Object:");
    Serial.print("x: ");
    Serial.print((int32_t)pixy.ccc.blocks[0].m_x);
    Serial.print("   ");
    Serial.print("y: ");
    Serial.println((int32_t)pixy.ccc.blocks[0].m_y);
    Serial.println("Second Object:");
    Serial.print("x: ");
    Serial.print((int32_t)pixy.ccc.blocks[1].m_x);
    Serial.print("   ");
    Serial.print("y: ");
    Serial.println((int32_t)pixy.ccc.blocks[1].m_y);
    Serial.println("-----------------------");*/

    Serial.println("Blue:");
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
    Serial.println("-----------------------");
  
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
