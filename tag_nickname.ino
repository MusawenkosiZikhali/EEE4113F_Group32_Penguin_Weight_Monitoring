#define TXD1 19
#define RXD1 21
HardwareSerial mySerial(2);

// Define a structure for tag-to-nickname mapping
struct TagNickname {
  const char* tagID;
  const char* nickname;
};

// Define the mapping table based on the actual scanned tags
TagNickname tagMap[] = {
  {"4A00D3D01D54", "Billy"},
  {"4B00B4F65158", "Sally"},
  {"4A00DD414096", "Charlie"}  // Updated to match the actual tag ID
};
const int tagMapSize = sizeof(tagMap) / sizeof(tagMap[0]);

void RFIDUARTSetup() {
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1); // UART setup
}

String readRFID() {
  String tagData = "";
  bool reading = false;
  unsigned long startTime = millis();

  while (millis() - startTime < 500) { // 500ms timeout
    if (mySerial.available()) {
      char c = mySerial.read();
      // Start reading after STX (0x02)
      if (c == 0x02) {
        reading = true;
        continue;
      }
      // Stop reading at CR (0x0D) or LF (0x0A), ignore ETX (0x03)
      if (reading && (c == 0x0D || c == 0x0A || c == 0x03)) {
        break;
      }
      // Collect printable characters for the tag ID
      if (reading && isPrintable(c) && tagData.length() < 20) {
        tagData += c;
      }
    }
  }

  // Return the tag ID only if it matches the expected length (e.g., 11-12 chars)
  return tagData.length() >= 10 ? tagData : "";
}

String getNickname(String tagID) {
  for (int i = 0; i < tagMapSize; i++) {
    if (tagID == tagMap[i].tagID) {
      return String(tagMap[i].nickname);
    }
  }
  return "UnknownPenguin"; // Default for unmapped tags
}

void setup() {
  Serial.begin(115200);  // Initialize Serial Monitor
  RFIDUARTSetup();       // Initialize RFID UART
  Serial.println("RFID Reader Ready...");
}

void loop() {
  String rfidData = readRFID();
  if (rfidData != "") {
    String nickname = getNickname(rfidData);
    Serial.print("Tag ID: ");
    Serial.println(rfidData);
    Serial.print("Nickname: ");
    Serial.println(nickname);
  }
  delay(100);  // Small delay to prevent overwhelming the serial monitor
}