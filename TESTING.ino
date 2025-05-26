#include <WiFi.h>
#include "esp_wpa2.h"
#define CAMERA_MODEL_AI_THINKER
#include "esp_camera.h"
#include "camera_pins.h"
#include <base64.h> // For base64 encoding

#define CAM_RX 12  // I012 on ESP32-CAM (RX for UART2)
#define CAM_TX 13  // I013 on ESP32-CAM (TX for UART2)
HardwareSerial MasterSerial(2);  // UART2 for communication with ESP32-WROOM-32

// Eduroam credentials
const char* ssid = "eduroam";
const char* username = "username@wf.uct.ac.za"; //put your uct username
const char* password = ""; //put your uct password

camera_fb_t *fb = NULL;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("CAMERA INIT FAILED");
    while (true);
  }
  Serial.println("CAMERA READY");

  MasterSerial.begin(115200, SERIAL_8N1, CAM_RX, CAM_TX);
  Serial.println("UART2 INITIALIZED");

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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (MasterSerial.available()) {
    String command = MasterSerial.readStringUntil('\n');
    command.trim();

    if (command == "PING") {
      MasterSerial.println("PONG");
    }
    else if (command == "CAPTURE") {
      if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
      }
      fb = esp_camera_fb_get();
      if (!fb) {
        MasterSerial.println("CAPTURE FAILED");
      } else {
        Serial.println("IMAGE CAPTURED");
        MasterSerial.println("DONE");
      }
    }
    else if (command == "GET_IMAGE") {
      if (fb) {
        String base64Image = base64::encode(fb->buf, fb->len);
        MasterSerial.print("BASE64:");
        MasterSerial.println(base64Image);
        Serial.println("Base64 image sent over UART");
      } else {
        MasterSerial.println("NO_IMAGE");
        Serial.println("No image available to send");
      }
    }
  }
  delay(10);
}