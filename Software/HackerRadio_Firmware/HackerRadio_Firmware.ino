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

Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);

ESP8266WebServer server(80);

unsigned int FMSTATION = 10230;      // 10230 == 102.30 MHz





///////////////////////////////////////////////////////////////////////////////////
void sendClientWebpage() {
  Serial.println(F("\nClient connected."));
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
  Serial.println(F("Client disonnected."));

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}






////////////////////////////////////////////////////////////////////////////////////
void printRadioInfo() {
  radio.readTuneStatus();
  Serial.print("Curr freq: ");
  Serial.print(radio.currFreq/100); Serial.print("."); Serial.print(radio.currFreq%100); Serial.print(" MHz");
  Serial.print("\tCurr ASQ: 0x");   Serial.print(radio.currASQ, HEX);
  Serial.print("\tCurr InLevel:");  Serial.println(radio.currInLevel);
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
  Serial.println(F("\n\nWelcome to Hacker Radio v0.0.1"));
}




///////////////////////////////////////////////////////////////////////////
void initWifi() {
  Serial.println(F("Initialzing wifi..."));
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer

  server.on("/changefrequency", HTTP_GET, handleChangeRadioFrequencyRequest);
  server.on("/", HTTP_GET, sendClientWebpage);
  server.begin();
  Serial.println(F("Wifi connection...SUCCESS"));
}




///////////////////////////////////////////////////////////////////////////////////////
void initFmRadio() {
  Serial.print(F("\n\nConnecting to FM radio..."));
  while (! radio.begin()) {  // begin with address 0x63 (CS high default)
    Serial.println(F("FAILED! Couldn't find radio!"));
    //while (1);
  }
  Serial.println("connected.");

  // Uncomment to scan power of entire range from 87.5 to 108.0 MHz
  /*
    for (uint16_t f  = 8750; f<10800; f+=10) {
    radio.readTuneMeasure(f);
    Serial.print("Measuring "); Serial.print(f); Serial.print("...");
    radio.readTuneStatus();
    Serial.println(radio.currNoiseLevel);
    }
  */
  Serial.print(F("Tuning to: "));
  Serial.println(FMSTATION);
  radio.tuneFM(FMSTATION);
  Serial.print(F("Setting TX power..."));
  radio.setTXpower(MAX_TX_POWER);  // dBuV, 88-115 max
  Serial.println(F("done."));
  printRadioInfo();

  // begin the RDS/RDBS transmission
  radio.beginRDS();
  radio.setRDSstation("HackerRadio");
  radio.setRDSbuffer( "HackerRadio FTW!");
  Serial.println("RDS on!\n\n");

  radio.setGPIOctrl(_BV(1) | _BV(2));  // set GP1 and GP2 to output
  radio.setGPIO((1 << 2) || (1 << 1));
}
