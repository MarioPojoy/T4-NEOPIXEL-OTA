#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include "credentials.h"

#define LED_PIN   D2
#define LED_COUNT 16

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long pixelPrevious = 0;     
unsigned long patternPrevious = 0;     
int           patternCurrent = 0;      
int           patternInterval = 5000;  
int           pixelInterval = 10;      
int           pixelQueue = 0;           
int           pixelCycle = 0;           
uint16_t      pixelCurrent = 0;         
uint16_t      pixelNumber = LED_COUNT;  
const char* hostname = "esp8266-neopixel";

void colorWipe(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;             
  strip.setPixelColor(pixelCurrent, color); 
  strip.show();                        
  pixelCurrent++;                         
  if(pixelCurrent >= pixelNumber)           
    pixelCurrent = 0;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  ArduinoOTA.setHostname(hostname);

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {
        type = "filesystem";
      }
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  strip.begin();      
  strip.show(); 
  strip.setBrightness(10);
}

void loop() {
ArduinoOTA.handle();
  unsigned long currentMillis = millis();                  
  if((currentMillis - patternPrevious) >= patternInterval) { 
    patternPrevious = currentMillis;
    patternCurrent++;                                    
    if(patternCurrent >= 3)
      patternCurrent = 0;
  }
  
  if(currentMillis - pixelPrevious >= pixelInterval) {      
    pixelPrevious = currentMillis;                        
    switch (patternCurrent) {
      case 2:
        colorWipe(strip.Color(0, 0, 255), pixelInterval);
        break;
      case 1:
        colorWipe(strip.Color(0, 255, 0), pixelInterval);
        break;        
      default:
        colorWipe(strip.Color(255, 0, 0), pixelInterval);
        break;
    }
  }
}
