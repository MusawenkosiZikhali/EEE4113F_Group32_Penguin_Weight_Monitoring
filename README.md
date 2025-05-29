# Penguin Weight Monitoring Project
## Overview
The Penguin Weight Monitoring Project is an IoT-based system designed to monitor the weight of African penguins using an ESP32 microcontroller, an RFID reader, and an ESP32-CAM module. The system detects penguins via RFID tags (assigned to named penguins like Billy, Sally, and Charlie) or weight changes on a scale, captures images, and sends the data (weight, condition, location, and image) to a Node.js server for storage in a MongoDB database. The project aims to track penguin health by analyzing weight trends and associating them with specific locations.

## Key Features

- RFID Tag Detection: Identifies penguins (Billy, Sally, Charlie) by their RFID tags and records their weight immediately.
- Weight Averaging: Averages weight readings when a penguin (tagged or untagged) is on the scale, sending the average when they leave.
- Location Assignment: Assigns locations based on nickname (e.g., Billy at Boulders Beach, Sally at Stony Point, Charlie at Robben Island).
- Image Capture: Uses ESP32-CAM to take photos when weight changes are detected or tags are read.
- Data Transmission: Sends JSON data to a remote server via HTTP POST.
- Weight Filtering: Ignores unrealistic weight readings (> 5 kg) to handle scale errors.

## Hardware Requirements

- ESP32 Development Board: Main controller (e.g., ESP32-WROOM-32).
- ESP32-CAM Module: For image capture.
- RFID Reader: Connected via UART (e.g., MFRC522 or similar, configured for 9600 baud).
- Load Cell/Scale: Connected to provide weight data via UDP.
- Wires and Power Supply: For connecting components.

## Software Requirements

- Arduino IDE: For programming the ESP32 (with ESP32 board support installed).
- Libraries:
  - WiFi.h
  - WiFiUdp.h
  - esp_wpa2.h
  - HTTPClient.h
  - ArduinoJson.h
  - base64.h


- Node.js Server: Running at http://196.24.134.8:3000/api/weight with MongoDB for data storage.
- Wi-Fi Network: Eduroam (or similar WPA2-Enterprise network) with valid credentials.

# Setup Instructions
1. Hardware Setup

- Connect the RFID reader to the ESP32:
  - TXD1 to GPIO 19
  - RXD1 to GPIO 21

- Connect the ESP32-CAM to the ESP32:
  - CAM_TXD to GPIO 17
  - CAM_RXD to GPIO 16


- Connect the load cell/scale to send weight data via UDP to port 12345.
- Power all components and ensure stable connections.

2. Software Configuration

- Install the Arduino IDE and add ESP32 board support via the Boards Manager.
- Install the required libraries via the Arduino Library Manager.
- Open linking.ino in the Arduino IDE.
- Update the following credentials in the code:
  - ssid: Set to "eduroam" (or your Wi-Fi network name).
  - username: Replace with your Eduroam username (e.g., username@wf.uct.ac.za).
  - password: Replace with your Eduroam password.
  - serverUrl: Ensure it points to your Node.js server (default: http://196.24.134.8:3000/api/weight).


- Update the tagMap array in the code if you have different RFID tags or nicknames.

3. Upload and Run

- Select your ESP32 board and port in the Arduino IDE.
- Upload the code to the ESP32.
- Open the Serial Monitor (115200 baud) to view debug output.
- Ensure the ESP32-CAM and RFID reader are powered and communicating.

4. Server Setup

- Set up a Node.js server with an endpoint (/api/weight) to receive POST requests and store data in MongoDB.
- The server should expect JSON data with fields: penguin_id, measurements (array with date, weight, condition, location), and image (base64-encoded).

# Usage

- With RFID Tag: Place a tagged penguin on the scale. The system will detect the tag, record the current weight, take a photo, and send the data to the server with the assigned location.
- Without RFID Tag: If a significant weight change (1.5–5 kg) is detected, the system will average weights until the weight drops below 1.5 kg, then send the average with the default location ("Boulders Beach") or the last known nickname's location.
- Monitoring: Check the Serial Monitor for real-time updates and the MongoDB database for stored data.

# Example Output
**Serial Monitor (Tag Read - Billy)**
Tag Detected!
Tag ID: 4A00D3D01D54
Nickname: Billy
Image Captured
Associated Weight for Billy: 2.34 kg
Sending data to Node.js server...
{"penguin_id":"Billy","measurements":[{"date":"2025-05-27T12:53:00Z","weight":2.34,"condition":"normal","location":"Boulders Beach"}],"image":"..."}
Response Code: 201
Server Response:
{"success":true,"message":"📦 Data saved to MongoDB with image!"}

**Serial Monitor (No Tag - Averaging)**
Significant weight detected - penguin on scale!
Accumulated weight sum: 2.45, Count: 1
[...]
Weight dropped to insignificant - penguin has left!
Average weight during visit: 2.56 kg
Sending data to Node.js server...
{"penguin_id":"UnknownPenguin","measurements":[{"date":"2025-05-27T12:53:00Z","weight":2.56,"condition":"normal","location":"Boulders Beach"}],"image":"..."}
Response Code: 201
Server Response:
{"success":true,"message":"📦 Data saved to MongoDB with image!"}


