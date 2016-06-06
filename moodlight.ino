#include "FastLED.h"
#define LED_COUNT 20
#define LED_PIN D4
CRGB leds[LED_COUNT];
static uint8_t bri = 128;
static uint8_t hue = 0;
static uint8_t sat = 255;
static bool rainb = true;
static unsigned long rainb_t = 0;
static const long rainb_i = 100;

#include <ESP8266WiFi.h>
#include "wifi.h"

#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
ESP8266WebServer HTTP(80);

void rainbow() {
  unsigned long now = millis();
  if (now - rainb_t >= rainb_i) {
    FastLED.showColor(CHSV(hue++, 255, bri));
    rainb_t = now;
  }
}

void handleHSV() {
  HTTP.send(200);
  hue = atoi(HTTP.arg("h").c_str());
  sat = atoi(HTTP.arg("s").c_str());
  bri = atoi(HTTP.arg("v").c_str());
  Serial.printf("h=%d s=%d v=%d",hue,sat,bri);
  Serial.println();
  FastLED.showColor(CHSV(hue, sat, bri));
  rainb = false;
}

void handleRGB() {
  HTTP.send(200);
  static uint8_t r = atoi(HTTP.arg("r").c_str());
  static uint8_t g = atoi(HTTP.arg("g").c_str());
  static uint8_t b = atoi(HTTP.arg("b").c_str());
  bri = atoi(HTTP.arg("h").c_str());
  Serial.printf("r=%d g=%d b=%d h=%d",r,g,b,bri);
  Serial.println();
  FastLED.showColor(CRGB(r, g, b));
  FastLED.setBrightness(bri);
  rainb = false;
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Starting HTTP...\n");
  HTTP.on("/rgb", handleRGB);
  HTTP.on("/hsv", handleHSV);
  HTTP.on("/rainbow", [](){ HTTP.send(200); rainb = true; rainb_t = millis(); });
  HTTP.on("/description.xml", HTTP_GET, [](){
    SSDP.schema(HTTP.client());
  });
  HTTP.begin();
  Serial.printf("Starting SSDP...\n");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("moodlight");
  SSDP.setSerialNumber("001788102207");
  SSDP.setURL("index.html");
  SSDP.setModelName("moodlight");
  SSDP.setModelNumber("1337");
  SSDP.setModelURL("http://toerb.de");
  SSDP.setManufacturer("toerb electronics");
  SSDP.setManufacturerURL("http://toerb.de");
  SSDP.begin();
  
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
  FastLED.setBrightness(bri);
  
}

void loop() {
  HTTP.handleClient();
  if (rainb) {
    rainbow();
  }
  delay(1);
}
