//External Libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>              // Include the SPIFFS library
#include <Adafruit_Si4713.h> //must edit Adafruit_Si4713.cpp if using NodeMCU. Must edit the begin() function to change the I2C SDA and SCL pins    _wire->pins(4,5);
#include "secrets.h"

#define RESETPIN 12
#define BAUDRATE 115200
#define DEFAULT_TXPOWER 95 //MIN 88-115 MAX
#define DEFAULT_FREQ 10230 //MIN 8800-10800 MAX
#define TCP_PORT 80

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTHEX(x, HEX) Serial.print(x, HEX)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTHEX(x, HEX)
#endif

Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);

AsyncWebServer server(TCP_PORT);

unsigned int FMSTATION = DEFAULT_FREQ; // 10230 == 102.30 MHz
unsigned int TXPOWER = DEFAULT_TXPOWER;

String hr_version = "v0.0.3";

bool isRadioConnected = false;

////////////////////////////////////////////////////////////////////////////////////
void printRadioInfo()
{
  if (isRadioConnected)
  {
    radio.readTuneStatus();
    DEBUG_PRINT("Current TX Frequency: ");
    printFrequency(radio.currFreq);
    DEBUG_PRINT("\tCurrent ASQ: 0x");
    DEBUG_PRINTHEX(radio.currASQ, HEX);
    DEBUG_PRINT("\tCurrent InLevel:");
    DEBUG_PRINT(radio.currInLevel);
    DEBUG_PRINT("\tTX Power:");
    DEBUG_PRINTLN(TXPOWER);
  }
  else
  {
    printRadioErrorMessage();
  }
}

////////////////////////////////////////////////////////////////////////////
void setup()
{
  initSerial();
  initWifiAP();
  initServer();
  initSPIFFS();
  initFmRadio();
  DEBUG_PRINTLN(F("\n\nWaiting for clients to connect..."));
}

////////////////////////////////////////////////////////////////////////////
void loop()
{
}

//////////////////////////////////////////////////////////////////////////
void initSerial()
{
  Serial.begin(BAUDRATE);
  delay(10);
  DEBUG_PRINT(F("\n\nWelcome to Hacker Radio "));
  DEBUG_PRINTLN(hr_version);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

///////////////////////////////////////////////////////////////////////////
void initWifiAP()
{
  DEBUG_PRINTLN(F("Establishing WiFi AP..."));
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password); // Start the access point
  DEBUG_PRINT("Access Point \"");
  DEBUG_PRINT(ssid);
  DEBUG_PRINTLN("\" started");
  DEBUG_PRINT("IP address:\t");
  DEBUG_PRINTLN(WiFi.softAPIP()); // Send the IP address of the ESP8266 to the computer
  DEBUG_PRINTLN(F("WiFi AP established."));
}

///////////////////////////////////////////////////////////////////////////////////
void initServer()
{
  DEBUG_PRINTLN(F("Starting server..."));
  server.serveStatic("/images/gsg.jpg", SPIFFS, "/images/gsg.jpg");

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
    DEBUG_PRINTLN(F("\n### Client connected."));
  });

  server.on("/index", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
    DEBUG_PRINTLN(F("\n### Client connected."));
  });

  server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css");
  });

  server.on("/getcurrentfrequency", HTTP_ANY, [](AsyncWebServerRequest *request) {
    String s = String(FMSTATION / 100);
    s += ".";
    s += String(FMSTATION % 100);
    request->send(200, "plain/text", s);
  });

  server.on("/getcurrenttxpower", HTTP_ANY, [](AsyncWebServerRequest *request) {
    String s = String(TXPOWER);
    request->send(200, "plain/text", s);
  });

  server.on("/changefrequency", HTTP_ANY, [](AsyncWebServerRequest *request) {
    if (request->hasParam("newfrequency"), true)
    {
      AsyncWebParameter *param = request->getParam("newfrequency", false);
      String param_string = param->value();
      int NEW_FMSTATION = param_string.toInt();

      DEBUG_PRINT(F("\n### Received request to change frequency to: "));
      printFrequency(NEW_FMSTATION);
      DEBUG_PRINTLN("");

      if (NEW_FMSTATION >= 8800 && NEW_FMSTATION <= 10800)
      {
        FMSTATION = NEW_FMSTATION;
        radio.tuneFM(FMSTATION);
      }
      else
      {
        DEBUG_PRINTLN(F("!!! ERROR: Invalid frequency selected. Must be between 88 and 108 MHz."));
      }

      request->send(SPIFFS, "/index.html");
      printRadioInfo();
    }
  });

  server.on("/changetxpower", HTTP_ANY, [](AsyncWebServerRequest *request) {
    if (request->hasParam("newtxpower"), true)
    {
      AsyncWebParameter *param = request->getParam("newtxpower", false);
      String param_string = param->value();
      unsigned int NEW_TXPOWER = param_string.toInt();

      DEBUG_PRINT(F("\n### Received request to change TX power to: "));
      DEBUG_PRINT(NEW_TXPOWER);
      DEBUG_PRINTLN(F("dBuV"));
      if (NEW_TXPOWER >= 88 && NEW_TXPOWER <= 115)
      {
        TXPOWER = NEW_TXPOWER;
        radio.setTXpower(TXPOWER);
      }
      else
      {
        DEBUG_PRINTLN(F("!!! ERROR: TX power is limited to between 88 and 115 dBuV"));
      }

      request->send(SPIFFS, "/index.html");

      printRadioInfo();
    }
  });

  server.begin();
  DEBUG_PRINTLN(F("Server is running."));
}

///////////////////////////////////////////////////////////////////////////////////////
void initFmRadio()
{
  DEBUG_PRINT(F("\n\nConnecting to FM radio..."));
  if (!radio.begin())
  { // begin with address 0x63 (CS high default)
    DEBUG_PRINTLN(F("FAILED! Couldn't find radio!"));
    //while (1);
  }
  else
  {
    DEBUG_PRINTLN("connected.");
    isRadioConnected = true;
  }

  if (isRadioConnected)
  {
    // Uncomment to scan power of entire range from 87.5 to 108.0 MHz
    /*
    for (uint16_t f  = 8750; f<10800; f+=10) {
    radio.readTuneMeasure(f);
    DEBUG_PRINT("Measuring "); DEBUG_PRINT(f); DEBUG_PRINT("...");
    radio.readTuneStatus();
    DEBUG_PRINTLN(radio.currNoiseLevel);
    }
  */
    radio.setGPIOctrl((1 << 2) || (1 << 1));
    radio.setGPIO((1 << 2) || (1 << 1));
    DEBUG_PRINT(F("Tuning to: "));
    printFrequency(FMSTATION);
    radio.tuneFM(FMSTATION);
    DEBUG_PRINT(F("\nSetting TX power to: "));
    DEBUG_PRINT(TXPOWER);
    DEBUG_PRINTLN(F(" dBuV"));
    radio.setTXpower(TXPOWER); // dBuV, 88-115 max
    printRadioInfo();

    // begin the RDS/RDBS transmission
    radio.beginRDS();
    radio.setRDSstation("HackerRadio");
    radio.setRDSbuffer("HackerRadio FTW!");
    DEBUG_PRINTLN(F("RDS on!"));
    DEBUG_PRINTLN(F("Radio setup complete."));
  }
  else
  {
    printRadioErrorMessage();
  }
}

/////////////////////////////////////////////////////
void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

/////////////////////////////////////////////////////
void printFrequency(int frequency)
{
  DEBUG_PRINT(frequency / 100);
  DEBUG_PRINT(".");
  DEBUG_PRINT(frequency % 100);
  DEBUG_PRINT(" MHz");
}

/////////////////////////////////////////////////////
void printRadioErrorMessage()
{
  DEBUG_PRINTLN(F("Radio disconnected. Check wiring and then reboot system."));
}
