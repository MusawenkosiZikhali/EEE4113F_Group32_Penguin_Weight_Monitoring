#include "HX711.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wpa2.h"
 
#define DT1 23
#define SCK1 22
#define DT2 21
#define SCK2 19
#define DT3 18
#define SCK3 5
#define DT4 4
#define SCK4 15
 
#define SCALE_FACTOR_1 131.409
#define SCALE_FACTOR_2 93.830
#define SCALE_FACTOR_3 126.626
#define SCALE_FACTOR_4 94.195
 
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;
 
// Wi-Fi credentials
const char* ssid = "eduroam";
const char* username = "<student number>@wf.uct.ac.za";
const char* password = "";
 
// UDP settings
WiFiUDP udp;
const char* mainBoardIP = "196.42.106.41"; // Replace with Main Board's IP (to be determined)
const int udpPort = 12345;
 
void setup() {
  Serial.begin(115200);
 
  // Initialize scales
  scale1.begin(DT1, SCK1);
  scale2.begin(DT2, SCK2);
  scale3.begin(DT3, SCK3);
  scale4.begin(DT4, SCK4);
 
  while (!scale1.is_ready() || !scale2.is_ready() || !scale3.is_ready() || !scale4.is_ready()) {
    Serial.println("Waiting for HX711 modules...");
    delay(500);
  }
 
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();
 
  scale1.set_scale(SCALE_FACTOR_1);
  scale2.set_scale(SCALE_FACTOR_2);
  scale3.set_scale(SCALE_FACTOR_3);
  scale4.set_scale(SCALE_FACTOR_4);
 
  Serial.println("All scales tared and calibrated. Ready for measurements in grams.");
 
  // Connect to eduroam
  Serial.println("Connecting to eduroam...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
 
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  esp_wifi_sta_wpa2_ent_enable();
 
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Scale Board IP Address: ");
  Serial.println(WiFi.localIP());
 
  udp.begin(udpPort);
}
 
float getAverageWeight() {
  const int numReadings = 5;
  float totalWeightSum = 0.0;
 
  for (int i = 0; i < numReadings; i++) {
    if (!scale1.is_ready() || !scale2.is_ready() || !scale3.is_ready() || !scale4.is_ready()) {
      Serial.println("HX711 not ready, skipping reading...");
      return 0.0;
    }
 
    float weight1 = scale1.get_units(1);
    float weight2 = scale2.get_units(1);
    float weight3 = scale3.get_units(1);
    float weight4 = scale4.get_units(1);
 
    float totalWeight = weight1 + weight2 + weight3 + weight4;
    totalWeightSum += totalWeight;
    delay(100);
  }
 
  float averageWeight = totalWeightSum / numReadings;
  return averageWeight / 1000.0; // Convert to kg
}
 
void loop() {
  float avgWeight = getAverageWeight();
  Serial.print("Average Weight: ");
  Serial.print(avgWeight, 2);
  Serial.println(" kg");
 
  // Send weight over UDP
  udp.beginPacket(mainBoardIP, udpPort);
  udp.printf("%.2f", avgWeight);
  udp.endPacket();
  Serial.println("Sent weight over UDP");
 
  delay(500);
}
