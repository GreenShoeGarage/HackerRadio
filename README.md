# HackerRadio
 
NodeMCU + I2C-controlled FM Transmitter + AP Mode + TRS Electret Microphone = Remote Control Listening Device

Adafruit Stereo FM Transmitter with RDS/RBDS Breakout - Si4713
https://www.adafruit.com/product/1958

You must also download this tool to the Arduino IDE: https://github.com/esp8266/arduino-esp8266fs-plugin  
This will allow you to upload the .HTML, and .CSS files  

You will also need these two libraries:  
https://github.com/me-no-dev/ESPAsyncTCP  
https://github.com/me-no-dev/ESPAsyncWebServer  


1. Solder length of wire for antenna
2. Wire up Si4713 breakout board to NodeMCU. Audio device must be transmitting at line level, mic level audio sources will not work.

| Node |  Si4713 |
| --- | --- |
| 3V | VIN |
| GND  | GND |
| D1 | SDA |
| D2 | SCL |
| D6 | RST |

3.  Connect TRS electret mic to Si4714 TRS jack
4.  Upload firmware
5.  Connect to "hackerradio" AP broadcast by NodeMCU
6.  Open browser, navigate to 192.168.4.1
7.  Enter a frequency and click submit  (For example 102.35MHz should be entered as 10235)
8.  Tune FM radio to same frequency
9.  Talk and listen

Example of the HTTP POST request:
http://192.168.4.1/changefrequency?newfrequency=10230


## Using with a NodeMCU

- You must edit the Adafruit_Si4713.cpp file to work with NodeMCU
- Add a new line to: bool Adafruit_Si4713::begin(uint8_t addr, TwoWire *theWire)  

...  
_wire = theWire;  
_wire->pins(4,5);  //ADD THIS LINE ONLY WHEN USING A NodeMCU <===================  
_wire->begin();  
...  

BOM  
https://partsbox.com/mbparks/project/22hpb7c3a4jxg8dhe0753b87zm/bom  
