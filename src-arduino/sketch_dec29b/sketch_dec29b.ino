#include <WiFi.h>
#include <WiFiUdp.h>

// --- CẤU HÌNH WIFI (ESP32 TỰ PHÁT RA) ---
const char* ssid = "MOCAP_SYSTEM";   // Tên WiFi bạn muốn đặt
const char* password = "12345678";   // Mật khẩu (ít nhất 8 ký tự)

// Địa chỉ IP của máy tính khi kết nối vào ESP32
// Mặc định ESP32 là 192.168.4.1, thiết bị đầu tiên kết nối vào (máy tính) sẽ là 192.168.4.2
const char* udpAddress = "192.168.4.2"; 
const int udpPort = 8080;

// Cấu hình Serial nhận từ Uno
#define RXD2 16
#define TXD2 17

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo Serial nhận từ Uno
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // BẮT ĐẦU PHÁT WIFI (Access Point Mode)
  Serial.println("Dang tao mang WiFi...");
  WiFi.softAP(ssid, password);
  
  Serial.println("\n--- DA PHAT SONG WIFI ---");
  Serial.print("Ten WiFi: "); Serial.println(ssid);
  Serial.print("IP cua ESP32: "); Serial.println(WiFi.softAPIP());
  Serial.println("Hay ket noi may tinh vao WiFi nay!");
}

void loop() {
  // Logic nhận và gửi giữ nguyên, chỉ khác là gửi thẳng cho client 192.168.4.2
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      // Bắn UDP về máy tính (Client)
      udp.beginPacket(udpAddress, udpPort);
      udp.print(data);
      udp.endPacket();
    }
  }
}