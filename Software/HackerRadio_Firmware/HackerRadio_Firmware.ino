#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <Adafruit_Si4713.h>  //must edit Adafruit_Si4713.cpp if using NodeMCU. Must edit the begin() function to change the I2C SDA and SCL pins    _wire->pins(4,5); 
#include "secrets.h"   


#define _BV(n) (1 << n)
#define RESETPIN 12
#define BAUDRATE 115200
#define MAX_TX_POWER 115  //88-115 MAX
#define DEFAULT_FREQ 10230
#define IP_PORT 80

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTLN(x)         Serial.println (x)
#define DEBUG_PRINT(x)           Serial.print(x)
#define DEBUG_PRINTHEX(x, HEX)   Serial.print(x, HEX)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTHEX(x, HEX)
#endif

Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);

AsyncWebServer server(IP_PORT);

unsigned int FMSTATION = DEFAULT_FREQ;      // 10230 == 102.30 MHz

String hr_version = "v0.0.2";






////////////////////////////////////////////////////////////////////////////////////
void printRadioInfo() {
  radio.readTuneStatus();
  DEBUG_PRINT("Current TX Frequency: ");
  printFrequency(radio.currFreq);
  DEBUG_PRINT("\tCurrent ASQ: 0x");   DEBUG_PRINTHEX(radio.currASQ, HEX);
  DEBUG_PRINT("\tCurrent InLevel:");  DEBUG_PRINTLN(radio.currInLevel);
}





////////////////////////////////////////////////////////////////////////////
void setup() {
  initSerial();
  initWifi();
  initSPIFFS();
  initFmRadio();
  DEBUG_PRINTLN(F("\n\nWaiting for clients to connect..."));
}




////////////////////////////////////////////////////////////////////////////
void loop() {
}




//////////////////////////////////////////////////////////////////////////
void initSerial() {
  Serial.begin(BAUDRATE);
  delay(10);
  DEBUG_PRINT(F("\n\nWelcome to Hacker Radio "));
  DEBUG_PRINTLN(hr_version);
}




///////////////////////////////////////////////////////////////////////////
void initWifi() {
  DEBUG_PRINTLN(F("Initialzing wifi..."));
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);             // Start the access point
  DEBUG_PRINT("Access Point \"");
  DEBUG_PRINT(ssid);
  DEBUG_PRINTLN("\" started");

  DEBUG_PRINT("IP address:\t");
  DEBUG_PRINTLN(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer


  server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  server.on("/index", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  server.on("/getcurrentfrequency", HTTP_ANY, [](AsyncWebServerRequest * request) {
    String s = String(FMSTATION / 100);
    s += ".";
    s += String(FMSTATION % 100);
    request->send(200, "plain/text", s);
  });

  server.on("/changefrequency", HTTP_ANY, [](AsyncWebServerRequest * request) {
    if (request->hasParam("frequency"), true) {
      AsyncWebParameter* param = request->getParam("frequency", false);
      String param_string = param->value();
      FMSTATION = param_string.toInt();

      DEBUG_PRINT(F("\n### Received request to change frequency to: "));
      printFrequency(FMSTATION);
      DEBUG_PRINTLN("");

      radio.tuneFM(FMSTATION);

      request->send(SPIFFS, "/index.html");

      printRadioInfo();
    }
  });


  server.begin();
  DEBUG_PRINTLN(F("Wifi connection...SUCCESS"));
}




///////////////////////////////////////////////////////////////////////////////////////
void initFmRadio() {
  DEBUG_PRINT(F("\n\nConnecting to FM radio..."));
  if (! radio.begin()) {  // begin with address 0x63 (CS high default)
    DEBUG_PRINTLN(F("FAILED! Couldn't find radio!"));
    //while (1);
  }
  else {
    DEBUG_PRINTLN("connected.");
  }
  // Uncomment to scan power of entire range from 87.5 to 108.0 MHz
  /*
    for (uint16_t f  = 8750; f<10800; f+=10) {
    radio.readTuneMeasure(f);
    DEBUG_PRINT("Measuring "); DEBUG_PRINT(f); DEBUG_PRINT("...");
    radio.readTuneStatus();
    DEBUG_PRINTLN(radio.currNoiseLevel);
    }
  */
  DEBUG_PRINT(F("Tuning to: "));
  printFrequency(FMSTATION);
  radio.tuneFM(FMSTATION);
  DEBUG_PRINT(F("\nSetting TX power to: "));
  DEBUG_PRINT(MAX_TX_POWER);
  DEBUG_PRINTLN(F(" dBuV"));
  radio.setTXpower(MAX_TX_POWER);  // dBuV, 88-115 max
  printRadioInfo();

  // begin the RDS/RDBS transmission
  radio.beginRDS();
  radio.setRDSstation("HackerRadio");
  radio.setRDSbuffer( "HackerRadio FTW!");
  DEBUG_PRINTLN(F("RDS on!"));
  DEBUG_PRINTLN(F("Radio setup complete."));

  radio.setGPIOctrl(_BV(1) | _BV(2));  // set GP1 and GP2 to output
  radio.setGPIO((1 << 2) || (1 << 1));
}





/////////////////////////////////////////////////////
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}





void printFrequency(int frequency) {
  DEBUG_PRINT(frequency / 100); DEBUG_PRINT("."); DEBUG_PRINT(frequency % 100); DEBUG_PRINT(" MHz");
}
