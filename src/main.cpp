#include <Arduino.h>

// TFT Libs and UI
#include <SPI.h>
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>


// WIFI
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


/** MQTT */
#include <MQTT.h>
MQTTClient mqttClient;



// TOUCH
#define TOUCH_ORIENTATION  1

// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// SCREEN
#define TFT_DC D4   // BEFORE 2
#define TFT_CS D8   // BEFORE 15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Adafruit_GFX_Button buttonA;
Adafruit_GFX_Button buttonB;
Adafruit_GFX_Button buttonC;
Adafruit_GFX_Button buttonD;


/* Wifi Setup */
WiFiManager wifiManager;
WiFiClient wifiClient;


String SensorTemp[4];
String SensorState[4];

static uint16_t prev_x = 0xffff, prev_y = 0xffff;

void displayStates(){

    tft.writeFillRect(120, 0, 120, 80, ILI9341_BLACK);
    String Sensor01String = String(SensorTemp[0]) + String("C");
    String Sensor02String = String(SensorTemp[1]) + String("C");

    tft.setCursor(130, 5); tft.setTextSize(2);
    if (SensorState[0] == "ON") {
      tft.setTextColor(ILI9341_GREEN);
    } else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(Sensor01String);

    tft.setCursor(130, 25); tft.setTextSize(2);
    if (SensorState[1] == "ON") {
      tft.setTextColor(ILI9341_GREEN);
    } else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(Sensor02String);

}

void displayMqttConnect(){
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Connecting to MQTT");
}

void displayMqttSubscriptions(){
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Subscribing to MQTT topics");
}

void mqttSubscriptions(){
  displayMqttSubscriptions();
  mqttClient.subscribe("/ThermoDev/Sensor01/CurrentState");
  mqttClient.subscribe("/ThermoDev/Sensor01/CurrentTemperature");
  mqttClient.subscribe("/ThermoDev/Sensor02/CurrentState");
  mqttClient.subscribe("/ThermoDev/Sensor02/CurrentTemperature");
}

void mqttMesageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  if (topic == "/ThermoDev/Sensor01/CurrentTemperature") {
    SensorTemp[0] = payload;
  }

  if (topic == "/ThermoDev/Sensor01/CurrentState") {
    SensorState[0] = payload;
  }

  if (topic == "/ThermoDev/Sensor02/CurrentTemperature") {
    SensorTemp[1] = payload;
  }

  if (topic == "/ThermoDev/Sensor02/CurrentState") {
    SensorState[1] = payload;
  }

  displayStates();
}

void mqttConnect() {
  mqttClient.begin("192.168.1.106", 1883, wifiClient);
  Serial.print("Connecting to MQTT...");
  displayMqttConnect();
  while (!mqttClient.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
}


void mqttSetup(){
  mqttConnect();
  mqttSubscriptions();
  mqttClient.onMessage(mqttMesageReceived);
}


void screenMain() {
    tft.fillScreen(ILI9341_BLACK);

    // Replace these for your screen module
    //                        X   Y   W   H    Border           Background    TextC                Text            Font size
    buttonA.initButton(&tft, 60, 40, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREENYELLOW, "Temp++", 1);
    buttonA.drawButton();

    buttonB.initButton(&tft, 60, 120, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREEN, "Temp--", 1);
    buttonB.drawButton();

    buttonC.initButton(&tft, 60, 200, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "On / Off", 1);
    buttonC.drawButton();

    buttonD.initButton(&tft, 120, 280, 240, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "Global On / Off", 1);
    buttonD.drawButton();

    // tft.drawChar(130, 160, 'Te', ILI9341_CYAN, ILI9341_BLACK, 1);
    displayStates();
}


void displayWaitingForWifi(){
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Connecting to WiFi. Worst case, try to set it up again");
}

void setup() {
  Serial.begin(115200);
  SPI.setFrequency(ESP_SPI_FREQ);

  tft.begin();

  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());


  displayWaitingForWifi();

  wifiManager.setTimeout(30);

  if(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
  }
  Serial.println("Connected");
  mqttSetup();
  screenMain();


}


void loop() {
  mqttClient.loop();

  if (!mqttClient.connected()) {
    mqttConnect();
  }

}
