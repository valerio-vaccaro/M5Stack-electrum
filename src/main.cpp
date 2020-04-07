#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "config.h"

#define ORIENTATION         1
#define BRIGHTNESS          100

#define ELECTRUM_URL        "kirsche.emzy.de"
#define ELECTRUM_PORT       50001

WiFiMulti WiFiMulti;
WiFiClient client;
int id;

void setup() {
  M5.begin();
  M5.Power.begin();

  // Lcd display
  M5.Lcd.setBrightness(BRIGHTNESS);
  Wire.begin();
  SD.begin();
  M5.Speaker.mute();
  M5.Lcd.println("M5Stack-electrum");
  WiFiMulti.addAP(ssid, password);
  M5.Lcd.print("Connecting to WiFi");
  while(WiFiMulti.run() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(500);
  }
  M5.Lcd.print(" Connected! Ip address: ");
  M5.Lcd.println(WiFi.localIP());
  M5.Lcd.println("A - ScriptHash history");
  M5.Lcd.println("B - Block 123 header");
  M5.Lcd.println("C - Hal Finney first transaction");

  id=0;
}

void call_electrum(char *method, char* params) {
  if (!client.connect(ELECTRUM_URL, ELECTRUM_PORT)) {
    M5.Lcd.println("!!! Connection failed.");
  } else {
    id+=1;
    char request[256];
    snprintf(request, 256, "{\"id\": %d, \"method\": \"%s\", \"params\": %s}", id, method, params);;
    client.println(request);
    delay(100);
    int maxloops = 0;
    while (!client.available() && maxloops < 1000) {
      maxloops++;
      delay(1);
    }
    if (client.available() > 0) {
      String line = client.readStringUntil('\r');
      M5.Lcd.println(line);
    } else {
      M5.Lcd.println("!!! Timed out !!!");
    }
    client.stop();
  }
}

void loop() {
  M5.update();
  if (M5.BtnA.wasReleased()) {
    M5.Lcd.println("--- ScriptHash history ---");
    call_electrum("blockchain.scripthash.get_history", "[\"9c0b6d7a6f8e74f16ed5e1adae8ee70e88a7e77c8a9bcc063b44c6f43e5f7fb2\"]");
  } else if (M5.BtnB.wasReleased()) {
    M5.Lcd.println("--- Block 123 header ---");
    call_electrum("blockchain.block.header", "[\"123\"]");
  } else if (M5.BtnC.wasReleased()) {
    M5.Lcd.println("--- Hal Finney first transaction ---");
    call_electrum("blockchain.transaction.get", "[\"f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\"]");
  }
}
