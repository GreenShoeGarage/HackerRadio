#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Si4713.h>  //must edit Adafruit_Si4713.cpp if using NodeMCU. Must edit the begin() function to change the I2C SDA and SCL pins    _wire->pins(4,5); 
#include "secrets.h"   //use secrets.h to store the ssid and encryption key, or uncomment the next two lines
//const char *ssid = "hackerradio"; // The name of the Wi-Fi network that will be created
//const char *password = "my_encyrption_key";   // The password required to connect to it, leave blank for an open network

#define _BV(n) (1 << n)
#define RESETPIN 12
#define BAUDRATE 9600
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

ESP8266WebServer server(IP_PORT);

unsigned int FMSTATION = DEFAULT_FREQ;      // 10230 == 102.30 MHz





///////////////////////////////////////////////////////////////////////////////////
void sendClientWebpage() {
  DEBUG_PRINTLN(F("\nClient connected."));
  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<h1>HackerRadio v0.0.1</h1>";
  s += "<form action=";
  s += "\"/changefrequency\">\r\n";
  s += "<label for=\"frequency\">Frequency:</label><br>\r\n";
  s += "<input type=\"text\" id=\"frequency\" name=\"frequency\" value=\"10230\"><br>\r\n";
  s += "<input type=\"submit\" value=\"Submit\">\r\n";
  s += "</form>\r\n"; 
  s += "<br><br>";

  /*Note: Uncomment the line below to refresh automatically
          for every 1 second. This is not ideal for large pages
          but for a simple read out, it is useful for monitoring
          your sensors and I/O pins. To adjust the fresh rate,
          adjust the value for content. For 30 seconds, simply
          change the value to 30.*/
  //s += "<meta http-equiv='refresh' content='1'/>\r\n";//auto refresh page

  s += "<h2>";
  s += "Transmitting Frequency: ";
  s += String(FMSTATION / 100, DEC);
  s += ".";
  s += String(FMSTATION % 100, DEC);
  s += " MHz";
  s += "</h2></html>\n";

  // Send the response to the client
  server.send(200, "text/html", s);
  delay(1);
  DEBUG_PRINTLN(F("Client disonnected."));

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}






////////////////////////////////////////////////////////////////////////////////////
void printRadioInfo() {
  radio.readTuneStatus();
  DEBUG_PRINT("Curr freq: ");
  DEBUG_PRINT(radio.currFreq/100); DEBUG_PRINT("."); DEBUG_PRINT(radio.currFreq%100); DEBUG_PRINT(" MHz");
  DEBUG_PRINT("\tCurr ASQ: 0x");   DEBUG_PRINTHEX(radio.currASQ, HEX);
  DEBUG_PRINT("\tCurr InLevel:");  DEBUG_PRINTLN(radio.currInLevel);
}










//////////////////////////////////////////////////////////////////////////
void handleChangeRadioFrequencyRequest() {
  if (server.hasArg("frequency")) { // this is the variable sent from the client
    FMSTATION = server.arg("frequency").toInt();
    radio.tuneFM(FMSTATION);
    sendClientWebpage();
  }
  printRadioInfo();
}






////////////////////////////////////////////////////////////////////////////
void setup() {
  initSerial();
  initWifi();
  initFmRadio();
}







////////////////////////////////////////////////////////////////////////////
void loop() {
  server.handleClient();
}








//////////////////////////////////////////////////////////////////////////
void initSerial() {
  Serial.begin(BAUDRATE);
  delay(10);
  DEBUG_PRINTLN(F("\n\nWelcome to Hacker Radio v0.0.1"));
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

  server.on("/changefrequency", HTTP_GET, handleChangeRadioFrequencyRequest);
  server.on("/", HTTP_GET, sendClientWebpage);
  server.begin();
  DEBUG_PRINTLN(F("Wifi connection...SUCCESS"));
}




///////////////////////////////////////////////////////////////////////////////////////
void initFmRadio() {
  DEBUG_PRINT(F("\n\nConnecting to FM radio..."));
  while (! radio.begin()) {  // begin with address 0x63 (CS high default)
    DEBUG_PRINTLN(F("FAILED! Couldn't find radio!"));
    //while (1);
  }
  DEBUG_PRINTLN("connected.");

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
  DEBUG_PRINTLN(FMSTATION);
  radio.tuneFM(FMSTATION);
  DEBUG_PRINT(F("Setting TX power..."));
  radio.setTXpower(MAX_TX_POWER);  // dBuV, 88-115 max
  DEBUG_PRINTLN(F("done."));
  printRadioInfo();

  // begin the RDS/RDBS transmission
  radio.beginRDS();
  radio.setRDSstation("HackerRadio");
  radio.setRDSbuffer( "HackerRadio FTW!");
  DEBUG_PRINTLN("RDS on!\n\n");

  radio.setGPIOctrl(_BV(1) | _BV(2));  // set GP1 and GP2 to output
  radio.setGPIO((1 << 2) || (1 << 1));
}
