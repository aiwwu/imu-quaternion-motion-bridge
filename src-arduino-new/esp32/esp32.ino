#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "MOCAP_SYSTEM";
const char* password = "12345678";

const char* udpAddress = "192.168.4.255"; 
const int udpPort = 8080;

#define RXD2 16 
#define TXD2 17 
#define BAUD_RATE 57600 

MPU6050 mpu;
uint8_t fifoBuffer[64];
Quaternion q;
uint16_t packetSize;
uint16_t fifoCount;
bool dmpReady = false;

WiFiUDP udp;
char packetBuffer[64]; 

void setup() {
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, RXD2, TXD2);
  Wire.begin(21, 22);
  Wire.setClock(400000); 

  mpu.initialize();
  

  if (mpu.dmpInitialize() == 0) {
    Serial.println("Dang tu dong can chinh... De yen MPU!");
    
    
    mpu.CalibrateAccel(6); 
    mpu.CalibrateGyro(6);  
    
    mpu.PrintActiveOffsets(); 
    
    mpu.setDMPEnabled(true);
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
    Serial.println(">>> MPU ESP32 DA SAN SANG!");
  }

  WiFi.softAP(ssid, password);
  Serial.println("WiFi Ready! IP: 192.168.4.1");
}


void sendRaw(const char* data) {
  udp.beginPacket(udpAddress, udpPort);
  udp.write((const uint8_t*)data, strlen(data));
  udp.endPacket();
}

void loop() {
  
  while (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();
    if (data.length() > 5) { 
        data.toCharArray(packetBuffer, 64);
        sendRaw(packetBuffer);
    }
  }

  if (dmpReady) {
      if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { 
          mpu.dmpGetQuaternion(&q, fifoBuffer);
          
          snprintf(packetBuffer, sizeof(packetBuffer), "2,%.4f,%.4f,%.4f,%.4f", q.w, q.x, q.y, q.z);
          
          sendRaw(packetBuffer);
      }
  }
}