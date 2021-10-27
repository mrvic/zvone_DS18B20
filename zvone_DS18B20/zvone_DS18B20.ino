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
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 
// 300=5 min
#define TIME_TO_SLEEP 300 /* Time ESP32 will go to sleep (in seconds) */ 
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
char wifi_gateway_IP[] ="PASS: Laminar68";
char mqtt_server_IP[] =  "MQTT: 0.0.0.0:1883";
char scale_weight[] = "? kg";
char scale_internal_temperature[] = "? °C";
char scale_humidity[] = "? %";
char scale_temp_ext1[] = "? °C";
char scale_temp_ext2[] = "? °C";
byte mac[6];
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
//const char* mqtt_server = "192.168.43.2";
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
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
  strcpy(prog_status, "VAGA");
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(64 + x, 20 + y, scale_weight);
  Serial.println(scale_weight);
  // display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  strcpy(prog_status, "WIFI");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 14 + y, wifi_local_IP);
  display->drawString(0 + x, 26 + y, wifi_gateway_IP);
  display->drawString(0 + x, 38 + y, mqtt_server_IP);
}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  strcpy(prog_status, "TEMP, VLAGA");
  display->setFont(ArialMT_Plain_16);
  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(5 + x, 14 + y, scale_internal_temperature);
  display->drawString(5 + x, 32 + y, scale_humidity);
  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(123 + x, 14 + y, scale_temp_ext1);
  display->drawString(123 + x, 32 + y, scale_temp_ext2);
}
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Predrag Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
}
// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {
  drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5
}
;
// how many frames are there?
int frameCount = 5;
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
  Serial.println();
  Serial.println();
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
void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
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
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("GATEWAY: ");
    Serial.println(WiFi.gatewayIP());
    Serial.println("Postavljam MQTT server nakon što sam se spojio na WIFI");
    //client.setServer(mqtt_server, 1883);
    client.setServer(WiFi.gatewayIP(), 1883);
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
  Serial.println();
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
    if((WiFi.status() != WL_CONNECTED) && (wifinow - wifilastMsg > 20000)) {
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
    long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;
      // ČITANJE VAGE
      Serial.print("UNITS: ");

      strcpy(scale_internal_temperature,"12");
      strcpy(scale_humidity,            "50");
      strcpy(scale_temp_ext1,           "32");
      strcpy(scale_temp_ext2,           "31");
      Serial.print("Temperature: ");
      Serial.print("Humidity: ");
      if (client.connected()) {
        // postavi na display
        strcpy(wifi_local_IP,             WiFi.localIP().toString().c_str() );
        strcpy(wifi_gateway_IP,           WiFi.gatewayIP().toString().c_str() );
        // salji vrijednosti
        client.publish("esp32/vaga",        scale_weight);
        client.publish("esp32/t1",          scale_temp_ext1);
        client.publish("esp32/t2",          scale_temp_ext2);
        client.publish("esp32/temperature", scale_internal_temperature);
        client.publish("esp32/humidity",    scale_humidity);
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
