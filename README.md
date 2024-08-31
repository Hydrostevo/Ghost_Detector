# Ghost_Detector
An ESP32 E.M.F. meter and temperature/pressure sensor that can also be used to detect ghosts (👻 possibly)

Inspired by an entertaining Ghost walk in Edinburgh and the Ghostbusters P.K.E. Meter the ESP32 captures input from a floating antenna into the ESP32 ADC via the A0 pin.  A BMP180 tempaerture/pressure sensor captures ambient changes (ghosts are supposed to make things feel a bit chilly if they are floating about).  E.M.F. level is visulised on a Neopixel stick and displayed on a 1.3" OLED screen.  In addition the captured data is stored on a micro sd card.  There is a basic menu structure to set calibration parametes and some buttons to interact with the menus (because ghosts love buttons).  


[add demonic possession disclaimer here]



# Tasks
**Current tasks**
- Rebuild graphics from AdaFruit GFX to U8g2 to support different OLED sceens [30% complete]
- Refine Graphing
    - Bar chart
    - Line chart
    - Dial chart
    - Combination grpahs
- Add Preference functionality to retain parameters on power off
- Initiate data logging to sd card
- Menu structure rebuild
  - Calibration
  - RGBLED config
  - Button set up
- Signal smoothing
- <s>Add SMA antenna socket</S>

**Future tasks**
- Webserver to display data (not sure how the ghosts will react to wifi or how it will corrupt the A0 input)
- Add touch input sensors
- <s>Add second SMA antenna socket</s>
- Add onboard battery and charging circuit
- Source/print a case
- Upgrade to 1.5" TFT colour screen (SPI)
- Leave somewhere really haunted and see if anything happens

# Credits
EMF meter:<br>

Grpahics:<br>
https://github.com/olikraus/u8g2 <br>
OTA:<br>
https://lastminuteengineers.com/esp32-ota-updates-arduino-ide/ <br>

Graphing:<br>
https://github.com/KrisKasprzak/96_Graphing
