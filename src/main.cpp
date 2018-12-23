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
#include <XPT2046.h>


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


Adafruit_GFX_Button buttons[32];

int activeScreenNumber = 0;

XPT2046 touch(/*cs=*/ /* 4 */ D2, /*irq=*/ D1);

/* Wifi Setup */
WiFiManager wifiManager;
WiFiClient wifiClient;


String SensorTemp[4];
String SensorState[4];
String DesiredTemperature;

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

    tft.setCursor(130, 55); tft.setTextSize(4);
    if (SensorState[1] == "ON" || SensorState[0] == "ON") {
      tft.setTextColor(ILI9341_GREEN);
    } else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(DesiredTemperature);
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
  mqttClient.subscribe("/Home/Climate/DesiredTemperature");
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

  if (topic == "/Home/Climate/DesiredTemperature") {
    DesiredTemperature = payload;
  }

  if (activeScreenNumber == 0) {
      displayStates();
  }

}

void mqttConnect() {

  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(1);
  tft.println("Connecting to MQTT.");
  int i = 0;

  mqttClient.begin("192.168.1.106", 1883, wifiClient);
  // mqttClient.begin("192.168.1.250", 1883, wifiClient);
  Serial.print("Connecting to MQTT... ");
  displayMqttConnect();
  while (!mqttClient.connect("Screen01", "try", "try")) {

    if (i++ > 20) {
      tft.println("Failed. Rebooting.");
      ESP.restart();
    }

    tft.print(i);
    tft.println("...");
    Serial.print(".");
    delay(1000);
  }
  tft.println("Connected to MQTT");
}


void mqttSetup(){
  mqttConnect();
  mqttSubscriptions();
  mqttClient.onMessage(mqttMesageReceived);
}


void screenMain() {
    activeScreenNumber = 0;
    tft.fillScreen(ILI9341_BLACK);
    // Replace these for your screen module
    //                        X   Y   W   H    Border           Background    TextC                Text            Font size
    buttons[0].initButton(&tft, 60, 40, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREENYELLOW, "Temp++", 1);
    buttons[0].drawButton();

    buttons[1].initButton(&tft, 60, 120, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREEN, "Temp--", 1);
    buttons[1].drawButton();

    buttons[2].initButton(&tft, 60, 200, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "On", 1);
    buttons[2].drawButton();

    buttons[3].initButton(&tft, 180, 200, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "Off", 1);
    buttons[3].drawButton();

    buttons[4].initButton(&tft, 120, 280, 240, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "Scenes", 1);
    buttons[4].drawButton();

    // tft.drawChar(130, 160, 'Te', ILI9341_CYAN, ILI9341_BLACK, 1);
    displayStates();
}

void screenScenes() {
    activeScreenNumber = 1;
    tft.fillScreen(ILI9341_BLACK);
    // Replace these for your screen module
    //                        X   Y   W   H    Border           Background    TextC                Text            Font size
    buttons[5].initButton(&tft, 60, 40, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREENYELLOW, "Morning", 1);
    buttons[5].drawButton();

    buttons[6].initButton(&tft, 180, 40, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREEN, "Movies", 1);
    buttons[6].drawButton();

    buttons[7].initButton(&tft, 60, 120, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREENYELLOW, "Chillin", 1);
    buttons[7].drawButton();

    buttons[8].initButton(&tft, 180, 120, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREEN, "Minimal", 1);
    buttons[8].drawButton();

    buttons[9].initButton(&tft, 60, 200, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREENYELLOW, "Bureau", 1);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 180, 200, 120, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_GREEN, "Full", 1);
    buttons[10].drawButton();

    buttons[11].initButton(&tft, 120, 280, 240, 80, ILI9341_DARKCYAN, ILI9341_BLACK, ILI9341_WHITE, "< Back", 1);
    buttons[11].drawButton();

    // tft.drawChar(130, 160, 'Te', ILI9341_CYAN, ILI9341_BLACK, 1);
    // displayStates();
}




void displayWaitingForWifi(){
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(1);
  tft.println("Connecting to WiFi. Worst case, try to set it up again");
}

void setup() {
  Serial.begin(115200);
  SPI.setFrequency(ESP_SPI_FREQ);
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation

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



static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  tft.drawFastHLine(x - 8, y, 16,0xff);
  tft.drawFastVLine(x, y - 8, 16,0xff);
  while (!touch.isTouching()) {
    delay(10);
  }
  touch.getRaw(vi, vj);
  // Erase by overwriting with black
  tft.drawFastHLine(x - 8, y, 16, 0);
  tft.drawFastVLine(x, y - 8, 16, 0);
}

void calibrate() {
  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;
  touch.getCalibrationPoints(x1, y1, x2, y2);
  calibratePoint(x1, y1, vi1, vj1);
  delay(1000);
  calibratePoint(x2, y2, vi2, vj2);
  touch.setCalibration(vi1, vj1, vi2, vj2);


  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.println("Calibration Params");
  tft.println("");
  tft.setTextSize(3);
  tft.println(vi1);
  tft.println(vj1);
  tft.println(vi2);
  tft.println(vj2);
}


void loop() {

  if (false) {
    tft.fillScreen(ILI9341_BLACK);
    calibrate();  // No rotation!!
    while(true){
      delay(1000);
    }
    return ;
  }

  // touch.setCalibration(159, 1696, 1776, 221);
  touch.setCalibration(282, 1696, 1672, 305);
  // touch.setCalibration(186, 1712, 1760, 321);

  mqttClient.loop();

  if (!mqttClient.connected()) {
    mqttConnect();
  }

  uint16_t x, y;
  if (touch.isTouching()) {

    touch.getPosition(x, y);
    /* Serial.print("x: ");
    Serial.print(x);
    Serial.print(" - y: ");
    Serial.println(y); */

    if(activeScreenNumber == 0){
      if(activeScreenNumber == 0 && buttons[0].contains(x, y)){
          buttons[0].press(true);
          buttons[0].drawButton(true);
          Serial.println("Temp++");
          mqttClient.publish("/Screen01/Climate/DesiredTemperatureButton", "++");
          delay(25);
          buttons[0].drawButton(false);
          buttons[0].press(false);
      }

      if(activeScreenNumber == 0 && buttons[1].contains(x, y)){
          buttons[1].drawButton(true);
          Serial.println("Temp--");
          mqttClient.publish("/Screen01/Climate/DesiredTemperatureButton", "--");
          buttons[1].press(true);
          delay(25);
          buttons[1].drawButton(false);
          buttons[1].press(false);
      }

      if(activeScreenNumber == 0 && buttons[2].contains(x, y)){
          buttons[2].drawButton(true);
          Serial.println("On/Off");
          mqttClient.publish("/Screen01/Lights/Single/OnOff", "OnOff");
          buttons[2].press(true);
          delay(25);
          buttons[2].drawButton(false);
          buttons[2].press(false);
      }

      if(activeScreenNumber == 0 && buttons[3].contains(x, y)){
          buttons[3].drawButton(true);
          Serial.println("Global On/Off");
          mqttClient.publish("/Screen01/Lights/Global/OnOff", "OnOff");
          buttons[3].press(true);
          delay(25);
          buttons[3].drawButton(false);
          buttons[3].press(false);
      }


      if(activeScreenNumber == 0 && buttons[4].contains(x, y)){
          screenScenes();
          return ;
      }
    } else if(activeScreenNumber == 1){
      if (buttons[11].contains(x,y)){
          screenMain();
      }

    }

    for (int i = 0; i < 32; i++) {
        buttons[i].press(false);
    }
  }


}
