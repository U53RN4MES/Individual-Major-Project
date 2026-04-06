#include "esp_camera.h"
#include "waste_classifier.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

#define ARENA_SIZE 2000000

Eloquent::TF::Sequential<11, ARENA_SIZE> tf;

// ===== CLASS LABELS =====
const char* classNames[] = {"Cardboard", "Metal", "Plastic"};
float* input;

// ===== CAMERA PINS =====
#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

// ===== CAMERA INIT =====
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size   = FRAMESIZE_96X96;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }
  return true;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting...");

  input = (float*)ps_malloc(96 * 96 * 3 * sizeof(float));
  if (!input) {
      Serial.println("Input buffer PSRAM failed!");
      while(1);
  }
  Serial.println("Input buffer allocated in PSRAM");

  // Init model using EloquentTinyML wrapper
  tf.setNumInputs(96 * 96 * 3);
  tf.setNumOutputs(3);

  while (!tf.begin(waste_classifier_tflite).isOk()) {
    Serial.println(tf.exception.toString());
    delay(1000);
  }
  Serial.println("Model loaded successfully!");

  if (!initCamera()) {
    Serial.println("Camera init failed!");
    while (1);
  }
  Serial.println("Camera ready!");
  // Communication with Arduino UNO
  // RX = GPIO44 (D7), TX = GPIO43 (D6)
  Serial1.begin(9600, SERIAL_8N1, 44, 43);
  Serial.println("Waiting for CAPTURE command from UNO...");
}

// ===== CLASSIFY =====
void classifyImage() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Convert RGB565 to float array normalized 0-1
  uint16_t* pixels = (uint16_t*)fb->buf;
  for (int i = 0; i < 96 * 96; i++) {
    uint16_t pixel = pixels[i];
    input[i * 3 + 0] = ((pixel >> 11) & 0x1F) / 31.0f;
    input[i * 3 + 1] = ((pixel >> 5)  & 0x3F) / 63.0f;
    input[i * 3 + 2] = (pixel         & 0x1F) / 31.0f;
  }
  esp_camera_fb_return(fb);

  // Run inference
  if (!tf.predict(input).isOk()) {
    Serial.println(tf.exception.toString());
    return;
  }

  // Print results
  for (int i = 0; i < 3; i++) {
    Serial.printf("  %s: %.1f%%\n", classNames[i], tf.output(i) * 100);
  }

  int classIndex = tf.classification;
  Serial1.println(classNames[classIndex]);
  Serial.printf("Sent to UNO: %s\n", classNames[classIndex]);
}

// ===== LOOP =====
void loop() {
  // Check if UNO sent something
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    
    // Print what we received to Serial Monitor
    Serial.print("Received from UNO: ");
    Serial.println(command);

    if (command == "CAPTURE") {
      classifyImage();
    }
  }
}