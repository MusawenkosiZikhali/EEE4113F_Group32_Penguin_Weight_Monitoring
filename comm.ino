// RFID Reader Setup
#define TXD1 19
#define RXD1 21
HardwareSerial mySerial(1); // Use UART1 for RFID reader

// Camera Serial Communication
#define CAM_TXD 17 // ESP32-WROOM TX2 (connected to ESP32-CAM RX/I012)
#define CAM_RXD 16 // ESP32-WROOM RX2 (connected to ESP32-CAM TX/I013)
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
float currentWeight = 3.0; // Random initial value, adjust manually between 0.0 and 5.0

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
  delay(5000); // 5 seconds delay

  // Retry PING up to 3 times
  bool cameraConnected = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("PING attempt ");
    Serial.println(attempt);
    camSerial.println("PING");

    unsigned long start = millis();
    while (millis() - start < 5000) { // 5 second timeout per attempt
      if (camSerial.available()) {
        String response = camSerial.readStringUntil('\n');
        Serial.println("Ping response: " + response);
        if (response.indexOf("PONG") >= 0) {
          Serial.println("Camera module connected!");
          cameraConnected = true;
          break;
        }
      }
    }
    if (cameraConnected) break;
    delay(1000); // Wait 1 second before retrying
  }

  if (!cameraConnected) {
    Serial.println("Failed to connect to camera module after 3 attempts. Check wiring and power.");
    while (true); // Halt execution to allow debugging
  }
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

void takePhoto() {
  Serial.println("Requesting photo from camera module...");
  camSerial.println("CAPTURE");

  // Wait for response
  unsigned long start = millis();
  while (millis() - start < 5000) { // 5 second timeout
    if (camSerial.available()) {
      String response = camSerial.readStringUntil('\n');
      Serial.println("Camera response: " + response);
      if (response.indexOf("DONE") >= 0) {
        break;
      }
    }
  }
}

void checkWeightChange() {
  Serial.printf("Previous weight: %.1f kg, Current weight: %.1f kg\n", previousWeight, currentWeight);

  if ((previousWeight < 2.5 || previousWeight > 3.5) && currentWeight >= 2.5 && currentWeight <= 3.5) {
    Serial.println("Weight change detected in range 2.5-3.5 kg!");
    takePhoto();
  } else {
    Serial.println("No significant weight change in range 2.5-3.5 kg");
  }

  previousWeight = currentWeight;
}

void loop() {
  String rfidData = readRFID();

  if (rfidData != "") {
    lastTagReadTime = millis();

    if (rfidData != lastTagID) {
      String nickname = getNickname(rfidData);
      Serial.print("Tag ID: ");
      Serial.println(rfidData);
      Serial.print("Nickname: ");
      Serial.println(nickname);

      if (nickname != "UnknownPenguin") {
        takePhoto();
      }

      lastTagID = rfidData;
    }
  }

  // Check weight change every cycle
  checkWeightChange();

  delay(100);
}