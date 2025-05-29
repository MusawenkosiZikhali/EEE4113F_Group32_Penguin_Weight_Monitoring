# Penguin Weight Monitoring Project
## Overview
The Penguin Weight Monitoring Project is a comprehensive IoT solution designed to monitor the health of African penguins by tracking their weights, capturing images, and associating data with specific locations. The project consists of three subsystems working together: data collection (scale), data processing and communication (ESP32 with RFID and camera), and data storage and analysis (Node.js server with MongoDB). It aims to support conservation efforts by providing researchers with detailed weight trends and health conditions of penguins at locations like Boulders Beach, Stony Point, and Robben Island.
## Project Goals

- Monitor penguin weights to assess health (underweight, normal, overweight).
- Track individual penguins using RFID tags (e.g., Billy, Sally, Charlie).
- Associate data with specific locations based on penguin identity.
- Store data securely for long-term analysis and research.

## Subsystems
### Subsystem 1: Data Collection
This subsystem gathers raw weight data from the physical environment using a scale.

- **Components**:
Load cell/scale: Measures penguin weight and sends data via UDP.


- **Functionality**:
The scale continuously sends weight readings to the ESP32 via UDP on port 12345.


- **Output**:
Weight data (in kg) sent as UDP packets.



### Subsystem 2: Data Processing and Communication
This subsystem processes the collected weight data, identifies penguins via RFID, captures images, and sends the processed data to the server. It runs on an ESP32 microcontroller with an RFID reader and ESP32-CAM module.

- **Components**:
  - ESP32 Development Board: Main controller.
  - RFID Reader: Connected via UART (TXD1 to GPIO 19, RXD1 to GPIO 21) to identify penguins.
  - ESP32-CAM: Captures images of penguins.


- **Functionality**:
  - RFID Tag Detection: Identifies penguins (e.g., Billy, Sally, Charlie) via RFID tags and records their weight immediately.
  - Weight Averaging: Averages weights from significant changes (1.5–5 kg) to insignificant (< 1.5 kg) if no tag is read.
  - Location Assignment: Sets location based on nickname (Billy: Boulders Beach, Sally: Stony Point, Charlie: Robben Island; default: Boulders Beach).
  - Image Capture: Takes photos via ESP32-CAM when a tag is read or weight changes are detected.
  - Data Transmission: Sends JSON data (penguin ID, weight, condition, location, image) to the server via HTTP POST.
  - Weight Filtering: Ignores unrealistic weights (> 5 kg) to handle scale errors.


- **Output**:
  - JSON payload sent to the server (e.g., {"penguin_id":"Billy","measurements":[{"weight":2.34,"location":"Boulders Beach"}]}).



### Subsystem 3: Data Storage and Analysis
This subsystem receives data from the ESP32, stores it in a database, and provides tools for analysis.

- **Components**:
  - Node.js Server: Hosted at http://196.24.134.8:3000/api/weight.
  - MongoDB: Database for storing penguin data.


- **Functionality**:
  - Receives HTTP POST requests with JSON data from the ESP32.
  - Stores data in MongoDB with fields: penguin_id, measurements (array with date, weight, condition, location), and image (base64-encoded).
  - Provides an API for querying historical data and analyzing trends (e.g., weight changes over time).


- **Output**:
  - Data stored in MongoDB for research and visualization.
  - API responses confirming successful data storage (e.g., {"success":true,"message":"Data saved"}).



## Hardware Requirements

- Load cell/scale (for weight measurement).
- RFID reader (for penguin identification, connected to ESP32).
- ESP32 Development Board (main controller).
- ESP32-CAM module (for image capture).
- Wires, power supply, and connectors for all components.

## Software Requirements

- Arduino IDE: For programming the ESP32 (with ESP32 board support).
- ESP32 Libraries:
  - WiFi.h, WiFiUdp.h, esp_wpa2.h, HTTPClient.h, ArduinoJson.h, base64.h.

- Node.js and MongoDB: For the server and database.
- Wi-Fi Network: Eduroam (or similar WPA2-Enterprise network) with valid credentials.

## Setup Instructions
1. Hardware Setup

- Connect the load cell/scale to send weight data via UDP to port 12345.
- Connect the RFID reader to the ESP32:
  - TXD1 to GPIO 19, RXD1 to GPIO 21.


- Connect the ESP32-CAM to the ESP32:
  - CAM_TXD to GPIO 17, CAM_RXD to GPIO 16.


- Power all components and ensure stable connections.

2. ESP32 Software Configuration

- Install Arduino IDE and add ESP32 board support.
- Install required libraries via the Library Manager.
- Open TESTING.ino for initializing camera(ESP32-CAM).
- Open linking.ino (for data processing: ESP32-WROOM-32).
- Update credentials:
  - ssid: "eduroam" (or your network).
  - username: Your Eduroam username (e.g. username@wf.uct.ac.za).
  - password: Your Eduroam password.
  - serverUrl: Your Node.js server URL (default: http://196.24.134.8:3000/api/weight).


- Upload the code to the ESP32 and monitor via Serial Monitor (115200 baud).

3. Server Setup

- Set up a Node.js server with an endpoint (/api/weight) to handle POST requests.
- Configure MongoDB to store data with the expected schema.
- Ensure the server is running at the specified URL and accessible from the ESP32.

4. Testing the System

- Place a tagged penguin (e.g., Billy) on the scale to test immediate data sending.
- Place an untagged penguin on the scale to test weight averaging.
- Verify data in MongoDB and check Serial Monitor for debug output.

## Usage

- Tagged Penguins: When a tagged penguin steps on the scale, the ESP32 identifies them via RFID (e.g., Billy), records their weight, captures an image, and sends data to the server with the assigned location.
- Untagged Penguins: If no tag is detected, the ESP32 averages weights during the visit and sends the average with the default location ("Boulders Beach") or the last known nickname’s location.
- Data Analysis: Use the Node.js API to query MongoDB for historical data, visualize weight trends, and assess penguin health.


## Directory Structure

- /subsystem1: Scale calibrattion.
- /subsystem2: ESP32 code (linking.ino) for data processing, RFID reading, and communication.
- /subsystem3: Node.js server code and MongoDB setup scripts.

