#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <base64.h> // For base64 encoding

// RFID Reader Setup
#define TXD1 19
#define RXD1 21
HardwareSerial mySerial(1); // Use UART1 for RFID reader

// Camera Serial Communication
#define CAM_TXD 17
#define CAM_RXD 16
HardwareSerial camSerial(2); // Use UART2 for ESP32-CAM

// Tag mapping
struct TagNickname {
  const char* tagID;
  const char* nickname;
};

TagNickname tagMap[] = {
  {"4A00D3D01D54", "Billy"},
  {"4B00B4F65158", "Sally"},
  {"4A00DD414096", "Charlie"}
};
const int tagMapSize = sizeof(tagMap) / sizeof(tagMap[0]);

// Weight variables
float previousWeight = 0.0;
float currentWeight = 1; // Will be updated via UDP

// Wi-Fi credentials
const char* ssid = "eduroam";
const char* username = "username@wf.uct.ac.za"; // replace username with your uct username
const char* password = ""; // put your uct password

// UDP settings
WiFiUDP udp;
const int udpPort = 12345;
char incomingPacket[255];

// Node.js Server Configuration
const char* serverUrl = "http://196.24.134.8:3000/api/weight";

// System variables
unsigned long lastTagReadTime = 0;
String lastTagID = "";

void setup() {
  Serial.begin(115200);

  // Initialize RFID reader
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1);

  // Initialize communication with ESP32-CAM
  camSerial.begin(115200, SERIAL_8N1, CAM_RXD, CAM_TXD);

  Serial.println("Main controller ready - waiting for camera module...");
  delay(5000);

  // Retry PING up to 3 times
  bool cameraConnected = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("PING attempt ");
    Serial.println(attempt);
    camSerial.println("PING");

    unsigned long start = millis();
    while (millis() - start < 5000) {
      if (camSerial.available()) {
        String response = camSerial.readStringUntil('\n');
        if (response.indexOf("PONG") >= 0) {
          Serial.println("Camera module connected!");
          cameraConnected = true;
          break;
        }
      }
    }
    if (cameraConnected) break;
    delay(1000);
  }

  if (!cameraConnected) {
    Serial.println("Failed to connect to camera module after 3 attempts. Check wiring and power.");
    while (true);
  }

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
  Serial.print("Main Board IP Address: ");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);

  // Synchronize time with NTP
  setupTime();
}

void setupTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized!");
}

String getISOTime() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  gmtime_r(&now, &timeinfo);
  char isoTime[30];
  strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(isoTime);
}

String readRFID() {
  String tagData = "";
  bool reading = false;
  unsigned long startTime = millis();

  while (millis() - startTime < 500) {
    if (mySerial.available()) {
      char c = mySerial.read();
      if (c == 0x02) {
        reading = true;
        continue;
      }
      if (reading && (c == 0x0D || c == 0x0A || c == 0x03)) {
        break;
      }
      if (reading && isPrintable(c) && tagData.length() < 20) {
        tagData += c;
      }
    }
  }
  return tagData.length() >= 10 ? tagData : "";
}

String getNickname(String tagID) {
  for (int i = 0; i < tagMapSize; i++) {
    if (tagID == tagMap[i].tagID) {
      return String(tagMap[i].nickname);
    }
  }
  return "UnknownPenguin";
}

String getImageFromCamera() {
  camSerial.println("GET_IMAGE");
  String imageData = "";
  unsigned long start = millis();
  while (millis() - start < 10000) {
    if (camSerial.available()) {
      imageData = camSerial.readStringUntil('\n');
      if (imageData.startsWith("BASE64:")) {
        imageData.remove(0, 7);
        break;
      } else if (imageData == "NO_IMAGE") {
        break;
      }
    }
  }
  return imageData;
}

void takePhoto() {
  camSerial.println("CAPTURE");

  unsigned long start = millis();
  while (millis() - start < 5000) {
    if (camSerial.available()) {
      String response = camSerial.readStringUntil('\n');
      if (response.indexOf("DONE") >= 0) {
        Serial.println("Image Captured");
        break;
      }
    }
  }
}

void sendDataToNodeServer(const char* penguinID, float weight, String timestamp, String imageData) {
  if (weight < 0) weight = 0;

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String condition;
  if (weight < 2.5) {
    condition = "underweight";
  } else if (weight > 3.5) {
    condition = "overweight";
  } else {
    condition = "normal";
  }

  StaticJsonDocument<32768> doc;
  doc["penguin_id"] = penguinID;
  //doc["species"] = "African Penguin";

  JsonArray measurements = doc.createNestedArray("measurements");
  JsonObject measurement = measurements.createNestedObject();
  measurement["date"] = timestamp;
  measurement["weight_kg"] = weight; // Changed from weight_kg to weight
  measurement["condition"] = condition;
  measurement["location"] = "Boulders Beach";

  doc["image"] = imageData.isEmpty() ? "" : imageData;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.println("\nSending data to Node.js server...");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("Response Code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println("Server Response:");
    Serial.println(response);
  } else {
    Serial.printf("Failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void checkWeightChange(String rfidData) {
  if (currentWeight > 1.5 && rfidData == "") {
    Serial.println("Weight change detected in range 1.5 - 5 kg!");
    takePhoto();
    Serial.print("Weight is ");
    Serial.println(currentWeight, 2);
    //Serial.println("Image Captured");

    String timestamp = getISOTime();
    String imageData = getImageFromCamera();
    sendDataToNodeServer("UnknownPenguin", currentWeight, timestamp, imageData);
  }
  previousWeight = currentWeight;
}

void loop() {
  // Check for UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
      currentWeight = atof(incomingPacket);
    }
  }

  String rfidData = readRFID();

  if (rfidData != "") {
    String nickname = getNickname(rfidData);
    if (nickname != "UnknownPenguin") {
      Serial.println("Tag Detected!");
      Serial.print("Tag ID: ");
      Serial.println(rfidData);
      Serial.print("Nickname: ");
      Serial.println(nickname);
      takePhoto();
      Serial.print("Associated Weight for ");
      Serial.print(nickname);
      Serial.print(": ");
      Serial.print(currentWeight, 2);
      Serial.println(" kg");

      String timestamp = getISOTime();
      String imageData = getImageFromCamera();
      sendDataToNodeServer(nickname.c_str(), currentWeight, timestamp, imageData);
    }
    lastTagID = rfidData;
  }

  // Check weight change and take photo if significant
  checkWeightChange(rfidData);

  delay(100);
}