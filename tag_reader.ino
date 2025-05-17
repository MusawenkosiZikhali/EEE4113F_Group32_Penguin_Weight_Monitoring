#define TXD1 19
#define RXD1 21
HardwareSerial mySerial(2);
 
void RFIDUARTSetup() {
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1);  // UART setup
}
 
String readRFID() {
  if (mySerial.available()) {
    // Read data and display it
    String message = mySerial.readStringUntil('\n');
    return message;
  }
  return "";  // Return empty string if no data is available
}
 
void setup() {
  Serial.begin(115200);  // Initialize Serial Monitor
  RFIDUARTSetup();       // Initialize RFID UART
}
 
void loop() {
  String rfidData = readRFID();
  if (rfidData != "") {
    Serial.println("RFID Data: " + rfidData);
  }
  delay(100);  // Small delay to prevent overwhelming the serial monitor
}