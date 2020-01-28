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
#define minWidth 0
#define maxWidth 315
#define minHeight 0
#define maxHeight 207
#define oneBlockMin 45
#define oneBlockMax 55
#define stepSize 60

Block candidates[9];
Block topRow[4];
Block bottomRow[4];


void setup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");

  stopMotors();
 
  // We need to initialize the pixy object 
  pixy.init();
  // Use color connected components program for the pan tilt to track 
  pixy.changeProg("color_connected_components");
  pixy.setLamp(1,1);
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
  int posc = 0;
  int post = 0;
  int posb = 0;
  int minTopIndex = 0;
  int minBottomIndex = 0;
  int numb = 0;
  Block highestBlock, temp;
  
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

    for(int bnum = 0; bnum < pixy.ccc.numBlocks; bnum++)
    {
      if(minWidth < (uint16_t)pixy.ccc.blocks[bnum].m_width < maxWidth && minHeight < (uint16_t)pixy.ccc.blocks[bnum].m_height < maxHeight)
      {
        candidates[posc] = pixy.ccc.blocks[bnum];
        posc++;
        Serial.println("found");
      }
    } //never leaves this loop
    
    for(int x = 0; x < sizeof(candidates) - 1; x++)
    {
      if(candidates[x].m_y > candidates[x + 1].m_y)
        highestBlock = candidates[x];
    }

    for(int x = 0; x < sizeof(candidates); x++)
    {
      if(candidates[x].m_y > (highestBlock.m_y - maxHeight/2))
      {
        topRow[post] = candidates[x];
        post++;
      }
      else
      {
        bottomRow[posb] = candidates[x];
        posb++;
      }
    }


    for(int x = 0; x < sizeof(topRow) - 1; x++)                                    
    {
      minTopIndex = x;
      for(int y = x + 1; y < sizeof(topRow); y++)
      {
        if(topRow[y].m_x < topRow[minTopIndex].m_x)
          minTopIndex = y;
      }
      temp = topRow[minTopIndex];
      topRow[minTopIndex] = topRow[x];
      topRow[x] = temp;
    }
    
    for(int x = 0; x < sizeof(bottomRow); x++)
    {
      minBottomIndex = x;
      for(int y = x + 1; y < sizeof(bottomRow); y++)
      {
        if(topRow[y].m_x < topRow[minBottomIndex].m_x)
          minBottomIndex = y;
      }
      temp = bottomRow[minBottomIndex];
      bottomRow[minBottomIndex] = bottomRow[x];
      bottomRow[x] = temp;
    }
    
    for(int x = 0; x < sizeof(topRow); x++)
    {
      numb = 0;
      if(topRow[x].m_signature == 1)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("B ");
      }
      if(topRow[x].m_signature == 2)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("R ");
      }
      if(topRow[x].m_signature == 3)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("Y ");
      }
    }
    Serial.println(" ");
    for(int x = 0; x < sizeof(bottomRow); x++)
    {
      numb = 0;
      if(bottomRow[x].m_signature == 1)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("B ");
      }
      if(bottomRow[x].m_signature == 2)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("R ");
      }
      if(bottomRow[x].m_signature == 3)
      {
        for(int y = 0; y <= 300; y += stepSize)
        {
          if(oneBlockMin + y < topRow[x].m_width < oneBlockMax + y)
            numb++;
        }
        for(int z = 0; z <= numb; z++)
          Serial.print("Y ");
      }
    }
    Serial.println("--------------------");
    
    
  
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
