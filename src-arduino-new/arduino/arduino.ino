#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <SoftwareSerial.h>

SoftwareSerial linkSerial(2, 3); 

#define TCAADDR 0x70
MPU6050 mpu[2];
uint8_t fifoBuffer[64];
Quaternion q;


unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 20; 

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  
  Serial.begin(115200); 
  
 
  linkSerial.begin(57600);

  for (int i = 0; i < 2; i++) {
    tcaselect(i);
    mpu[i].initialize();
    mpu[i].dmpInitialize();
    

    mpu[i].setXGyroOffset(0);
    mpu[i].setYGyroOffset(0);
    mpu[i].setZGyroOffset(0);
    mpu[i].setZAccelOffset(1688); 

    if (mpu[i].testConnection()) {
      mpu[i].setDMPEnabled(true);
    }
  }
}

void loop() {
  for (int i = 0; i < 2; i++) {
    tcaselect(i);
    if (mpu[i].dmpGetCurrentFIFOPacket(fifoBuffer)) {
        mpu[i].dmpGetQuaternion(&q, fifoBuffer);
        
       
        if (millis() - lastSendTime > SEND_INTERVAL && i == 1) { 
           lastSendTime = millis();
          
        }

    
   
        linkSerial.print(i); 
        linkSerial.print(",");
        linkSerial.print(q.w, 4); 
        linkSerial.print(",");
        linkSerial.print(q.x, 4); 
        linkSerial.print(",");
        linkSerial.print(q.y, 4); 
        linkSerial.print(",");
        linkSerial.println(q.z, 4);
    }
  }
}