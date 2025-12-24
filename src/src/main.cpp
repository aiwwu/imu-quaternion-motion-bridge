#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ap_ssid = "ESP32";
const char* ap_password = "12345678";
const char* udpAddress = "192.168.4.2";
const int udpPortRaw = 12347;

WiFiUDP udpRaw;
MPU6050 mpu68(0x68);
MPU6050 mpu69(0x69);
bool dmp_ready = false;
uint16_t packet_size;

void setup() {
  Wire.begin();
  Wire.setClock(100000);  
  Serial.begin(115200);
  while (!Serial);

  // wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("{\"key\": \"/log\", \"value\": \"Đang chờ kết nối WiFi...\", \"level\": \"DEBUG\"}");
  while (WiFi.softAPgetStationNum() == 0) {
    delay(500);
  }
  Serial.println("{\"key\": \"/log\", \"value\": \"WiFi kết nối thành công\", \"level\": \"INFO\"}");

  // mpu
  Serial.println("{\"key\": \"/log\", \"value\": \"Khởi tạo MPU 0x68...\", \"level\": \"DEBUG\"}");
  mpu68.initialize();
  uint8_t error_code = mpu68.dmpInitialize();
  if (error_code == 1) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x68 lỗi: initial memory load failed\", \"level\": \"ERROR\"}");
    while (1);
  }
  if (error_code == 2) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x68 lỗi: DMP configuration updates failed\", \"level\": \"ERROR\"}");
    while (1);
  }

  Serial.println("{\"key\": \"/log\", \"value\": \"Khởi tạo MPU 0x69...\", \"level\": \"DEBUG\"}");
  mpu69.initialize();
  error_code = mpu69.dmpInitialize();
  if (error_code == 1) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x69 lỗi: initial memory load failed\", \"level\": \"ERROR\"}");
    while (1);
  }
  if (error_code == 2) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x69 lỗi: DMP configuration updates failed\", \"level\": \"ERROR\"}");
    while (1);
  }

  // connect
  if (!mpu68.testConnection()) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x68 kết nối thất bại\", \"level\": \"ERROR\"}");
    while (1);
  }
  if (!mpu69.testConnection()) {
    Serial.println("{\"key\": \"/log\", \"value\": \"MPU 0x69 kết nối thất bại\", \"level\": \"ERROR\"}");
    while (1);
  }

  // Hiệu chỉnh MPU6050
  mpu68.setXGyroOffset(0);
  mpu68.setYGyroOffset(0);
  mpu68.setZGyroOffset(0);
  mpu68.setXAccelOffset(0);
  mpu68.setYAccelOffset(0);
  mpu68.setZAccelOffset(0);

  mpu69.setXGyroOffset(0);
  mpu69.setYGyroOffset(0);
  mpu69.setZGyroOffset(0);
  mpu69.setXAccelOffset(0);
  mpu69.setYAccelOffset(0);
  mpu69.setZAccelOffset(0);

  mpu68.CalibrateAccel(6);
  mpu68.CalibrateGyro(6);
  mpu69.CalibrateAccel(6);
  mpu69.CalibrateGyro(6);
  Serial.println("\n");

  // Bật DMP
  mpu68.setDMPEnabled(true);
  mpu69.setDMPEnabled(true);
  packet_size = mpu68.dmpGetFIFOPacketSize();
  dmp_ready = true;
  Serial.println("{\"key\": \"/log\", \"value\": \"MPU6050 sẵn sàng\", \"level\": \"INFO\"}");
}

void loop() {
  if (!dmp_ready || WiFi.softAPgetStationNum() == 0) {
    Serial.println("{\"key\": \"/log\", \"value\": \"WiFi ngắt kết nối\", \"level\": \"ERROR\"}");
    delay(500);
    return;
  }

  uint8_t fifo_buffer68[64];
  uint8_t fifo_buffer69[64];

  if (mpu68.dmpGetCurrentFIFOPacket(fifo_buffer68) && 
      mpu69.dmpGetCurrentFIFOPacket(fifo_buffer69)) {
    Quaternion q68_raw, q69_raw;
    mpu68.dmpGetQuaternion(&q68_raw, fifo_buffer68);
    mpu69.dmpGetQuaternion(&q69_raw, fifo_buffer69);

    int16_t ax68, ay68, az68, gx68, gy68, gz68;
    mpu68.getMotion6(&ax68, &ay68, &az68, &gx68, &gy68, &gz68);

    // sent file
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "{\"data\": \"/raw\", \"value\": {\"q68\": [%.6f, %.6f, %.6f, %.6f], \"q69\": [%.6f, %.6f, %.6f, %.6f], \"az68\": %d}}\n",
             q68_raw.w, q68_raw.x, q68_raw.y, q68_raw.z,
             q69_raw.w, q69_raw.x, q69_raw.y, q69_raw.z,
             az68);

    udpRaw.beginPacket(udpAddress, udpPortRaw);
    udpRaw.write((const uint8_t*)buffer, strlen(buffer));
    if (udpRaw.endPacket()) {
      Serial.println("{\"key\": \"/log\", \"value\": \"Gửi UDP thành công\", \"level\": \"DEBUG\"}");
    } else {
      Serial.println("{\"key\": \"/log\", \"value\": \"Lỗi gửi UDP\", \"level\": \"ERROR\"}");
    }
  }
}
