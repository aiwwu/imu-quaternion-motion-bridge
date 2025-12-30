#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <SoftwareSerial.h>

// Cấu hình SoftwareSerial để giao tiếp với ESP32
// RX = 2 (Không dùng), TX = 3 (Nối vào RX của ESP32)
SoftwareSerial linkSerial(2, 3); 

#define TCAADDR 0x70
MPU6050 mpu[2];
uint8_t fifoBuffer[64];
Quaternion q;

// Biến kiểm soát tốc độ gửi (để tránh làm ngập WiFi)
unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 20; // 20ms = 50Hz (Tốc độ lý tưởng cho WiFi)

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  
  // Serial thường để debug trên màn hình máy tính (nếu cần)
  Serial.begin(115200); 
  
  // Serial này để gửi sang ESP32
  linkSerial.begin(115200);

  for (int i = 0; i < 2; i++) {
    tcaselect(i);
    mpu[i].initialize();
    mpu[i].dmpInitialize();
    
    // Offset mặc định (như code cũ)
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
        
        // Kiểm soát tốc độ gửi để WiFi không bị nghẽn
        if (millis() - lastSendTime > SEND_INTERVAL && i == 1) { // Lấy nhịp theo chip 1
           lastSendTime = millis();
           // Không cần gửi gì đặc biệt, cứ để nó chạy tự nhiên
        }

        // Gửi data qua chân số 3 sang ESP32
        // Format: id,w,x,y,z
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