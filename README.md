# HackerRadio
 
NodeMCU + I2C-controlled FM Transmitter + AP Mode + TRS Electret Microphone = Remote Control Listening Device

Adafruit Stereo FM Transmitter with RDS/RBDS Breakout - Si4713
https://www.adafruit.com/product/1958

1. Solder length of wire for antenna
2. Wire up Si4713 breakout board to NodeMCU

- Node     Si4713
- 3V   ->  VIN
- GND  ->  GND
- D1   ->  SDA
- D2   ->  SCL

3.  Connect TRS electret mic to Si4714 TRS jack
4.  Upload firmware
5.  Connect to "hackerradio" AP broadcast by NodeMCU
6.  Open browser, navigate to 192.168.4.1
7.  Enter a frequency and click submit  (For example 102.35MHz should be entered as 10235)
8.  Tune FM radio to same frequency
9.  Talk and listen

Example of the HTTP request:
http://192.168.4.1/changefrequency?frequency=10230
