/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */
////// DS18B20 1-Wire sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 15
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
// variable to hold device addresses
DeviceAddress Thermometer;

int deviceCount = 0;
float tempC;

uint8_t sensor1[8] = {  0x28, 0xFF, 0x64, 0x1E, 0x01, 0xDA, 0x1F, 0xA3};
uint8_t sensor2[8] = {  0x28, 0xFF, 0x64, 0x1E, 0x01, 0xD6, 0xD4, 0x04};
uint8_t sensor3[8] = {  0x28, 0xFF, 0x64, 0x1E, 0x01, 0xE5, 0x60, 0x2F};
uint8_t sensor4[8] = {  0x28, 0xFF, 0x64, 0x1E, 0x01, 0xE5, 0x77, 0x31};


 
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
// 60=1 min
#define TIME_TO_SLEEP 60 /* Time ESP32 will go to sleep (in seconds) */ 
#include <SPI.h>  //OLED
#include <WiFi.h>  
#include <PubSubClient.h> //MQTT client

int status = WL_IDLE_STATUS;
// the Wifi radio's status
//SSID of your network
char prog_status[] = "Starting";
char ssid[] = "Valkyrie";
//password of your WPA Network
char pass[] = "Laminar68";
char wifi_local_IP[] = "SSID: Valkyrie";
//"0.0.0.0";
char wifi_status[]   = "X";
char wifi_gateway_IP[] ="PASS: 12345678Aa";
//char mqtt_server_IP[] =  "MQTT: 0.0.0.0:1883";
char mqtt_server_IP[] =  "MQTT: 192.168.0.11:1883";

char sensor_1_temp[20] = "? °C";
char sensor_2_temp[20] = "? °C";
char sensor_3_temp[20] = "? °C";
char sensor_4_temp[20] = "? °C";

char sensor_1_temp_short[20] = "...";
char sensor_2_temp_short[20] = "...";
char sensor_3_temp_short[20] = "...";
char sensor_4_temp_short[20] = "...";

byte mac[6];
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.0.50";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long wifilastMsg = millis();
long sleeplastMsg = millis();

//timer ocitanja vage, svakih 5 sekundi
char msg[50];
int value = 0;
IPAddress gateway;
////////////////////////////////////////////////////////////////
// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// Include the UI lib
#include "OLEDDisplayUi.h"
// Include custom images
#include "images.h"
// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 22, 21);
// SH1106Wire display(0x3c, D3, D5);
OLEDDisplayUi ui     ( &display );
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 0, prog_status);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128, 0, String(millis()));
  display->drawString(90, 0, wifi_status);
}
void drawFrame0(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
  strcpy(prog_status, "(´･_･`) DS18B20");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(5 + x, 16 + y, sensor_1_temp_short);
  display->drawString(5 + x, 34 + y, sensor_3_temp_short);

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->setFont(ArialMT_Plain_16);
  display->drawString(110 + x, 16 + y, sensor_2_temp_short);
  display->drawString(110 + x, 34 + y, sensor_4_temp_short);



 
}
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
  int n;
  char sensorID[20];
  n=sprintf (sensorID, "ID=%02X%02X%02X", sensor1[5], sensor1[6], sensor1[7]);
  strcpy(prog_status, sensorID);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(64 + x, 20 + y, sensor_1_temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "T1");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 20 + y, "°C");    
}
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int n;
  char sensorID[20];
  n=sprintf (sensorID, "ID=%02X%02X%02X", sensor2[5], sensor2[6], sensor2[7]);
  strcpy(prog_status, sensorID);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(64 + x, 20 + y, sensor_2_temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "T2");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 20 + y, "°C");    
}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int n;
  char sensorID[20];
  n=sprintf (sensorID, "ID=%02X%02X%02X", sensor3[5], sensor3[6], sensor3[7]);
  strcpy(prog_status, sensorID);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(64 + x, 20 + y, sensor_3_temp);
   display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "T3");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 20 + y, "°C");      
}
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int n;
  char sensorID[20];
  n=sprintf (sensorID, "ID=%02X%02X%02X", sensor4[5], sensor4[6], sensor4[7]);
  strcpy(prog_status, sensorID);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(64 + x, 20 + y, sensor_4_temp);
   display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "T4");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 20 + y, "°C");      
}
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  strcpy(prog_status, "WIFI");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 14 + y, wifi_local_IP);
  display->drawString(0 + x, 26 + y, wifi_gateway_IP);
  display->drawString(0 + x, 38 + y, mqtt_server_IP);
}
// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {
  drawFrame5,drawFrame0,drawFrame1, drawFrame2, drawFrame3, drawFrame4
}
;
// how many frames are there?
int frameCount = 6;
// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {
  msOverlay
}
;
int overlaysCount = 1;
///////////////////////////////////////////////
//
// -------------  S E T U P   --------------
//
///////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println("Serial port open - speed 115200");

  sensors.begin();  // Start up the library
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" DS18B20 devices.");
  Serial.println(".");


  Serial.println("Printing addresses...");
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer);
  }


  delay(500);
  ////////////////////////////////////////////////
   // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);
  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);
  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  // Add frames
  ui.setFrames(frames, frameCount);
  // Add overlays
  ui.setOverlays(overlays, overlaysCount);
  // Initialising the UI will init the display too.
  ui.init();
  display.flipScreenVertically();
  ///////////////////////////////////////////
  /// SPOJI SE NA WIFI NAKON ŠTO PRIKAŽEŠ EKRAN
  Serial.print("Pokrecem loop() ");
  // setup_wifi();
  // client.setServer(mqtt_server, 1883);
  //  client.setCallback(callback); 
  ////////////////////////////////////////////
}

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}



void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println("______________");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int dotcounter=0;
  while (WiFi.status() != WL_CONNECTED && dotcounter<3) {
    delay(500);
    Serial.print(".");
    dotcounter++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    strcpy(wifi_status,"@");
    Serial.println("-->  ");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("GATEWAY: ");
    Serial.println(WiFi.gatewayIP());
    Serial.println("Postavljam MQTT server nakon što sam se spojio na WIFI");
    client.setServer(mqtt_server, 1883);
    //client.setServer(WiFi.gatewayIP(), 1883);  // OVO OTKOMENTIRAJ !!!!!
    
    client.setCallback(callback);
  } else {
    Serial.println("WiFi not avilable, try again in 20sec");
  }
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
 
  // Feel free to add more if statements to control more GPIOs with MQTT
  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on") {
      Serial.println("on");
    } else if(messageTemp == "off") {
      Serial.println("off");
    }
  }
}
//////////////////////////////////////////////////
//
//---------------   M Q T T   --------------------
//
//////////////////////////////////////////////////
void reconnect() {
  int try_mqtt_counter=0;
  // Loop until we're reconnected
  while (!client.connected()) {
    try_mqtt_counter++;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Suscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 0.5 seconds");
      // Wait 2 seconds before retrying
      if(try_mqtt_counter>3) {
        Serial.println("MQTT broker not avaulable, please start broker at 0.0.0.0:1883");
        break;
      }
      delay(500);
    }
  }
}
//////////////////////////////////////////////////
//
//---------------   L O O P   --------------------
//
//////////////////////////////////////////////////
void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    long wifinow = millis();
    if((WiFi.status() != WL_CONNECTED) && (wifinow - wifilastMsg > 30000)) {
      // ako WIFI nije upaljen a proslo je vise od 20sec
      wifilastMsg = wifinow;
      // Postzavi status na ekran
      strcpy(prog_status, "Connecting...");
      setup_wifi();
      //pali WIFI
      if (!client.connected()) {
        reconnect();
        //Ovo je recconect samo na MQTT
      }
      client.loop();
    }
    /*
    ////////////
    /// CHECK SLEEPY TIME !!! :)
    long sleepnow = millis();
    if (sleepnow - sleeplastMsg > 300000) { // 300.000= 5 minuta sleep
      //10 sec =10000  1 minuta=60000
      sleeplastMsg = sleepnow;
      Serial.println("Sleepy time .....   zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz ");
      // puts the chip into power down mode
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      esp_deep_sleep_start();
    }
    /////////////
    */
    long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;
      // ČITANJE VAGE

       sensors.requestTemperatures(); 


  // Display address from each sensor
  sensors.getAddress(sensor1, 0);
  sensors.getAddress(sensor2, 1);
  sensors.getAddress(sensor3, 2);
  sensors.getAddress(sensor4, 3);

/*
      strcpy(sensor_1_temp,         sensors.getTempCByIndex(0));
      strcpy(sensor_2_temp,         sensors.getTempCByIndex(1));
      strcpy(sensor_3_temp,         sensors.getTempCByIndex(2));
      strcpy(sensor_4_temp,         sensors.getTempCByIndex(3));
*/
/*
dtostrf(sensors.getTempC(sensor1), 7, 2, sensor_1_temp);
dtostrf(sensors.getTempC(sensor2), 7, 2, sensor_2_temp);
dtostrf(sensors.getTempC(sensor3), 7, 2, sensor_3_temp);
dtostrf(sensors.getTempC(sensor4), 7, 2, sensor_4_temp);
*/

dtostrf(sensors.getTempCByIndex(0), 7, 2, sensor_1_temp);
dtostrf(sensors.getTempCByIndex(1), 7, 2, sensor_2_temp);
dtostrf(sensors.getTempCByIndex(2), 7, 2, sensor_3_temp);
dtostrf(sensors.getTempCByIndex(3), 7, 2, sensor_4_temp);
      
dtostrf(sensors.getTempCByIndex(0), 7, 0, sensor_1_temp_short);
dtostrf(sensors.getTempCByIndex(1), 7, 0, sensor_2_temp_short);
dtostrf(sensors.getTempCByIndex(2), 7, 0, sensor_3_temp_short);
dtostrf(sensors.getTempCByIndex(3), 7, 0, sensor_4_temp_short);


      Serial.println(sensor_1_temp);
      Serial.println(sensor_2_temp);
      Serial.println(sensor_3_temp);
      Serial.println(sensor_4_temp);          

      
      if (client.connected()) {
        // postavi na display
        strcpy(wifi_local_IP,             WiFi.localIP().toString().c_str() );
        strcpy(wifi_gateway_IP,           WiFi.gatewayIP().toString().c_str() );
       
        // salji vrijednosti
        client.publish("esp32/t1",          sensor_1_temp);
        client.publish("esp32/t2",          sensor_2_temp);
        client.publish("esp32/t3",          sensor_3_temp);
        client.publish("esp32/t4",          sensor_4_temp);
      } else {
        strcpy(wifi_status,"X");
      }
    }
    delay(remainingTimeBudget);
    //////////////////////
    // Ovdje je kraj ne pisi nista ispod
    ///////////////////
  }
}
