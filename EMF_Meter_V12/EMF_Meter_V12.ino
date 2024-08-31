//  An EMF detector and Ghost finder
//ESP32

// Global
#include <SPI.h>
#include <Wire.h>
#include <OneButton.h>  //http://www.mathertel.de/Arduino/OneButtonLibrary.aspx
#include <FastLED.h>
#include <Adafruit_BMP085.h>
#include <Arduino.h>

// OLED config 
#include <U8g2lib.h>      //Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  #ifdef U8X8_HAVE_HW_SPI
  #include <SPI.h>
  #endif
  #ifdef U8X8_HAVE_HW_I2C
  #include <Wire.h>
  #endif

// Local
#include "Bitmaps.h"

//--- OLED Constructor list ---//
// Please UNCOMMENT one of the contructor lines below.  The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// *** Update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected ***
//U8G2_NULL u8g2(U8G2_R0);	                            // null device, a 8x8 pixel display which does nothing
//U8G2_SSD1306_128X64_NONAME_1_3W_SW_SPI u8g2(U8G2_R0,  /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_1_3W_HW_SPI u8g2(U8G2_R0,  /* cs=*/ 10, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,     /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_1_HW_I2C u8g2(U8G2_R0,       /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0,     /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0,     /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0,     /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,     /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping

//U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//U8G2_SH1106_128X64_VCOMH0_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// same as the NONAME variant, but maximizes setContrast() range
//U8G2_SH1106_128X64_WINSTAR_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// same as the NONAME variant, but uses updated SH1106 init sequence

//--- End of constructor list ---//

//----- RGBLED strip (8 RGBLEDS) -----//
#define NUM_LEDS 8 // add number of LEDs of your RGB LED strip
#define PIN_LED 2 // digital output PIN that is connected to DIN of the RGB LED strip
#define LED_COLOR CRGB::DarkOrchid // see https://github.com/FastLED/FastLED/wiki/Pixel-reference for a full list, e.g. CRGB::AliceBlue, CRGB::Amethyst, CRGB::AntiqueWhite...
#define BRIGHTNESS 5
CRGB rgb_led[NUM_LEDS]; // color array of the LED RGB strip

//----- SD Card -----//
/*
MOSI = 23
MISO = 19
SCK = 18
SS = 5  
vcc = 3v3
*/

//----- BMP180 pressure/temp sensor -----//
Adafruit_BMP085 bmp;
      // VCC - 3.3V (NOT 5.0V!)
      // SCL - 
      // SDA - 

  
// Set A0 pin
#define PIN_ANTENNA1 36
#define PIN_ANTENNA2 39
/*
 Uncomment to set 3v or 5v range. Analog-to-digital converted value depends on the voltage 
 level of the VCC pin (maximum voltage to ESP8266 dev board A0 pin is 3.3v).
 Maximum voltage to ESP32 input pins 3.3v
*/
//ESP8266
//#define MAX_PIN_ANTENNA 1023  // ~ 5V
//#define MAX_PIN_ANTENNA 700   // ~ 3.3V
// ESP32
#define MAX_PIN_ANTENNA   4095  // ~ 3.3V

// Set button pins & script values 
#define PIN_INPUT5 34 // ButtonB     [GPI only requires external pull up resistor]
#define PIN_INPUT6 35 // ButtonA     [GPI only requires external pull up resistor]
#define PIN_INPUT1 33 // Select
#define PIN_INPUT2 32 // Menu
#define PIN_INPUT3 25 // Up button (side)
#define PIN_INPUT4 26 // Down button (side)

// Setup a new OneButton on pins
OneButton UpButton     (PIN_INPUT3, true);
OneButton DownButton   (PIN_INPUT4, true);
OneButton SelectButton (PIN_INPUT1, true);
OneButton MenuButton   (PIN_INPUT2, true);
OneButton ButtonA      (PIN_INPUT5, true);
OneButton ButtonB      (PIN_INPUT6, true);

//----- Menu variables -----//
int pic = 0;
int maxPics_L1 = 8;   // Set for the number of screens at level 1
int maxPics_L2 = 18;  // Set for the number of screens at level 2
//int maxPics_L3 = 400; // Set for the number of screens at level 3

int menuSelected = 0;

long lastmillis = 0;
long maxtime = 30000;

//----- Other varaibles -----//
int calValue = 0;
int emfValue1;
int emfValue2;
int emfValue;
int calIncrease;
int calDecrease;


//----- Graph variables -----//
int graphSelected = 1;


double volts;
double bvolts;
double x, y;

// Variables for the graphing functions
bool Redraw1 = true;
bool Redraw2 = true;
bool Redraw3 = true;
bool Redraw4 = true;
double ox , oy ;

//----- No idea -----//
unsigned long OldTime;
unsigned long counter;

//----- Timer -----//
#define CHECK_DELAY 1000
#define lmillis() ((long)millis())

//----- Ghost threshold -----
int ghostValue = 700;

//----- Fonts -----//
const char DEGREE_SYMBOL[] = { 0xB0, '\0' };

//=================================================================//
void setup() {
// Serial port
  Serial.begin(115200); // Setup the Serial port. see http://arduino.cc/en/Serial/IfSerial
  while (!Serial) {
    ;
  }   // wait for serial port to connect

//----- Set Buttons -----//
  // link the Up button functions.
  UpButton.attachClick(clickUp);
//  UpButton.attachDoubleClick(doubleclickUp);
//  UpButton.attachLongPressStart(longPressStartUp);
//  UpButton.attachLongPressStop(longPressStopUp);
//  UpButton.attachDuringLongPress(longPressUp);

  // link the Down button functions.
  DownButton.attachClick(clickDown);
//  DownButton.attachDoubleClick(doubleclickDown);
//  DownButton.attachLongPressStart(longPressStartDown);
//  DownButton.attachLongPressStop(longPressStopDown);
//  DownButton.attachDuringLongPress(longPressDown);

  // link the Select button functions.
  SelectButton.attachClick(clickSelect);
  SelectButton.attachDoubleClick(doubleclickSelect);
  SelectButton.attachLongPressStart(longPressStartSelect);
  SelectButton.attachLongPressStop(longPressStopSelect);
  SelectButton.attachDuringLongPress(longPressSelect);

  // link the Menu button functions.
  MenuButton.attachClick(clickMenu);
  MenuButton.attachDoubleClick(doubleclickMenu);
  MenuButton.attachLongPressStart(longPressStartMenu);
//  MenuButton.attachLongPressStop(longPressStopMenu);
  MenuButton.attachDuringLongPress(longPressMenu);

  // link the ButtonA button functions.
  ButtonA.attachClick(clickButtonA);
  
  // link the ButtonB button functions.
  ButtonB.attachClick(clickButtonB);

//----- Set Antenna pin -----//  
  pinMode(PIN_ANTENNA1, INPUT);
  pinMode(PIN_ANTENNA2, INPUT);
  
//----- Set FastLED -----//
  // FastLED define SPI pins
  FastLED.setBrightness(BRIGHTNESS );
  FastLED.addLeds<WS2812B, PIN_LED>(rgb_led, NUM_LEDS); 

      
//----- Set Display -----//
//  u8g2.begin();

  if(!u8g2.begin()) { 
    Serial.println("OLED setup failed");
  }

//----- Set BMP180 -----//
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }

//----- Confirm set up -----//
  delay(1000);
  Serial.println("Buttons set");
  Serial.println("Display live");
  Serial.println("SD card ready");
  Serial.println("RGBLED on");
  Serial.println("Pressure on");
  Serial.println("Temperature on");

//----- Start display with splash screen and message -----//
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.clearBuffer();					// clear the internal memory
  
  u8g2.drawXBM( 0, 0, 128, 64, PhantomShadow); // Draw Bitmap(x position, y position, bitmap width, bitmap height, bitmap name)
  u8g2.sendBuffer();
  delay(2000);
  u8g2.clearBuffer();	
  u8g2.drawStr(15,20,"EMF METER");
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(2000);

}   // End of Set Up

//----- Show EMF values -----//
void showReadings(int emfValue) {

//----- Display on OLED -----//
u8g2.firstPage();  // the loop routine runs over and over again forever:
  do {
    u8g2.setFont  (u8g2_font_helvR10_tf);
  // EMF
      u8g2.drawStr  (5,15,"EMF:");
      u8g2.setCursor(50, 15);
      u8g2.print    (emfValue);
  // Temperature
      u8g2.drawStr  (5,35,"Temp:");
      u8g2.setCursor(50,35);
      u8g2.print    (bmp.readTemperature());
      u8g2.drawStr  (105, 35, DEGREE_SYMBOL);
      u8g2.drawStr  (110,35, "C");
  // Pressure
      u8g2.drawStr  (5,55,"Press:");
      u8g2.setCursor(50,55);
      u8g2.print    (bmp.readPressure());
      u8g2.drawStr  (105,55, " Pa");

    } while (u8g2.nextPage());
delay(1000);
//----- Send to Serial monitor -----//
  // EMF
   Serial.print("EMF: ");
   Serial.println(emfValue);   // Print/Plot value in serial terminal
   Serial.println(emfValue1);
   Serial.println(emfValue2);
  // Calibration offset
    Serial.print("Calibration offset: ");
    Serial.println(calValue);
  // Temperature
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
  // Pressure
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
  // Altitude
  /*  
    // Calculate altitude: 'Standard' barometric pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());     Serial.println(" meters");

  // True Altitude: 1015 millibars = 101500 Pascals.
    Serial.print("True altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
    Serial.println();
*/

//----- Menu information -----//
    Serial.print("PIC; ");      Serial.println(pic);
    Serial.print("Menu #");     Serial.println(menuSelected);

}

//=================================================================//
void loop() {

//----- Monitor buttons -----//
  UpButton.tick();
  DownButton.tick();
  SelectButton.tick();
  MenuButton.tick();
  ButtonA.tick();
  ButtonB.tick();

//----- Map emfValue to led range -----//
 int num_leds_switchedon = map(emfValue, 0, MAX_PIN_ANTENNA, 0, NUM_LEDS);  
  for (int i = 0; i < num_leds_switchedon; ++i) {
    rgb_led[i] = LED_COLOR;
  }
  for (int i = num_leds_switchedon; i < NUM_LEDS; ++i) {
    rgb_led[i] = CRGB::Black;
  }
  FastLED.show(); 

//----- Menus -----//
    if (millis() >= (lastmillis + maxtime))
    {
        pic = 0;
    }

    if (pic == 0) // Default if no clicks detected
    {

//----- Capture EMF value -----//
    static int avgValue = 0, avgValue1 = 0, avgValue2 = 0, emfValue = 0; emfValue1 = 0; emfValue2 = 0;
    static long nextCheck = 0, emfSum = 0, emfSum1 = 0, emfSum2 = 0, iterations = 0;

    emfValue1 = constrain(analogRead(PIN_ANTENNA1), calValue, 4095);
    emfValue2 = constrain(analogRead(PIN_ANTENNA2), calValue, 4095);
    
    emfValue = (emfValue1 + emfValue2) / 2;

    emfSum1 += emfValue1;
    emfSum2 += emfValue2;
    emfSum  += emfValue;
    iterations++;

    if (lmillis() - nextCheck >= 0) {
        avgValue1 = emfSum1 / iterations;
        avgValue2 = emfSum2 / iterations;
        avgValue  = emfSum / iterations;

        emfSum1 = 0;
        emfSum2 = 0;
        emfSum  = 0;

        iterations = 0;
        showReadings(avgValue);
        showReadings(avgValue1);
        showReadings(avgValue2);
        nextCheck = lmillis() + CHECK_DELAY;
    }
/*    
//----- Draw bar gauge -----//
/*
// Build barchart
    display.drawRoundRect(0, 20, 126, 20, 2, WHITE);
    display.fillRect(3, 25, 120, 13, BLACK);
    display.fillRect(0, 22, map(emfValue, 0, 4095, 0, 118), 16, WHITE);
    display.display();
 */
    }

/*
//----- 1st layer screens [Main Menu] -----//                      
if (pic == 1) // [Bar graph]
{
  header1();
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,12); display.print ("Bar graph         o");
  display.setTextColor(WHITE);
  display.setCursor(5,22); display.print   ("Line graph        |");
  display.setCursor(5,32); display.print   ("Dial gauge        |");
  display.setCursor(5,42); display.print   ("Calibrate         |");
  menuSelected = 1;
  refresh();
}

if (pic == 2) // [Line graph]
{
  header1();
  display.setCursor(5,12); display.print   ("Bar graph         |");
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,22); display.print ("Line graph        o");
  display.setTextColor(WHITE);
  display.setCursor(5,32); display.print   ("Dial gauge        |");
  display.setCursor(5,42); display.print   ("Calibrate         |");
  menuSelected = 2;
  refresh();
}

if (pic == 3) // [Dial gauge]
{
  header1();
  display.setTextColor(WHITE);
  display.setCursor(5,12); display.print   ("Bar graph         |"); 
  display.setCursor(5,22); display.print   ("Line graph        |");
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,32); display.print ("Dial gauge        o");
  display.setTextColor(WHITE);
  display.setCursor(5,42);display.print    ("Calibrate         |");
  menuSelected = 3;
  refresh();
}

if (pic == 4) // [Graphs off]
{
  header1();
  display.setTextColor(WHITE);
  display.setCursor(5,12); display.print  ("Bar graph         |"); 
  display.setCursor(5,22); display.print  ("Line graph        |");
  display.setCursor(5,32); display.print  ("Dial gauge        |");
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,42); display.print("Calibrate         o");
  menuSelected = 4;
  refresh();
}

if (pic == 5)  // [Ghost threshold]
{  
  header2();
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,12); display.print("Ghost threshold   o");
  display.setTextColor(WHITE);
  display.setCursor(5,22); display.print  ("REBLED settings   |");
  display.setCursor(5,32); display.print  ("Information       |");
  display.setCursor(5,42); display.print  ("Exit menu         |");

  menuSelected = 5;
  refresh();
}

if (pic == 6)  // [RGBLED]
{
  header2();
  display.setCursor(5,12); display.print  ("Ghost Threshold   |");
    display.setTextColor(BLACK,WHITE);
  display.setCursor(5,22); display.print  ("REBLED settings   o");
  display.setTextColor(WHITE);
  display.setCursor(5,32); display.print  ("Information       |");
  display.setCursor(5,42); display.print  ("Exit menu         |");
  menuSelected = 6;
  refresh();
}

if (pic == 7)  // [Information]
{  
  header2();
  display.setTextColor(WHITE);
  display.setCursor(5,12); display.print  ("Ghost Threshold   |");
  display.setCursor(5,22); display.print  ("REBLED settings   |");
    display.setTextColor(BLACK,WHITE);
    display.setCursor(5,32); display.print("Information       o");
  display.setTextColor(WHITE);  
  display.setCursor(5,42); display.print  ("Exit menu         |");
  menuSelected = 7;
  refresh();
}

if (pic == 8)  // [Exit]
{  
  header2();
  display.setTextColor(WHITE);
  display.setCursor(5,12); display.print  ("Ghost Threshold   |");
  display.setCursor(5,22); display.print  ("REBLED settings   |");
  display.setCursor(5,32); display.print  ("Information       |");
    display.setTextColor(BLACK,WHITE);  
    display.setCursor(5,42); display.print("Exit menu         o");
  menuSelected = 8;
  refresh();
}

//----- 2nd layer screens -----//

if (pic == 11)  // [Screen 1 => Bar graph]
{  
  
  display.setCursor(5,22); display.print   ("Bar graph");

  refresh();
}

if (pic == 12)  // [Screen 1 => Line graph]
{
  
  display.setCursor(5,12); display.print   ("Line graph");

  refresh();
}

if (pic == 13)  // [Screen 1 => Dial gauge]
{
  
  display.setCursor(5,12); display.print   ("Dial gauge");

  refresh();
}

if (pic == 14)  // [Screen 1 => Calibrate]
{
  display.println  ("Use Up/Down buttons");
  display.println  ("to zero EMF reading");
  display.print    ("Cal offset: ");
  display.print    (calValue);
}

if (pic == 15)  // [Screen 2 => Ghost threshold]
{
  
  display.setCursor(5,12); display.print   ("Ghost threshold");

  refresh();
}

if (pic == 16)  // [Screen 2 => RGBLEd]
{
  
  display.setCursor(5,12); display.print   ("Ergableds");

  refresh();
}

if (pic == 17)  // [Screen 2 => Information]
{
  
  display.setCursor(5,12); display.print   ("Info");

  refresh();
}

if (pic == 18)  // [Screen 2 => Exit]
{
 pic=0;
}

*/

//----------------------------------------------------------------
//---- Detect a ghost -----//
    if (emfValue >ghostValue) {
        Serial.println("Ghost detected");
          u8g2.drawStr(5,12,"EMF:");
          u8g2.setCursor(70, 12);
          u8g2.print(emfValue);
          u8g2.sendBuffer();
  
  /*      display.clearDisplay();
        display.drawBitmap(0, 0, GhostBusters_small, 128, 46, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
        display.display();
        display.setCursor(30,50);
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.println("GHOST!!");
        display.display();
        delay(1000);        
    */    }
//    else {
//    }

}   // end of loop

//----- Button wiring & actions -----//
/*
_____                  3v3
     |                  ¦                             - Button modes -
     +- 34 --[270 ohm]--¦--<    B     >---¬            > click
     |                  ¦                 ¦            > doubleclick
     +- 35 --[270 ohm]--¦--<    A     >---¦            > longPressStart
  E  |                                    ¦            > longPress
  S  +- 33 ----------------<  Select  >---¦            > longPressStop      
  P  |                                    ¦
  3  +- 32 ----------------<   Menu   >---¦
  2  |                                    ¦     
     +- 25 ----------------<    Up    >---¦
     |                                    ¦
     +- 26 ----------------<   Down   >---¦    
_____|                                    ¦
                                         Gnd
                                          ¦     
*/

//--- Up button actions ---//
void clickUp() {
  lastmillis = millis();  
  // [Menu] warning
  if (pic == 0) {
//    menuWarning();
// DrawCGraph();
// DrawBarChartV();


Serial.print("Up button");
  }
      
      // Use Up button to navigate menu screens
  if(pic>=1 && pic<10)
  {
    if (pic >= maxPics_L1)
    {
      pic=1;
    }
    else if(pic < maxPics_L1)
    {
      pic--;
    }
  }
    
  // Use Up button to set calibration offset
  if(pic == 14){
    Serial.println("Calibration offset +1");
     calIncrease = calValue+1;
  if(calIncrease!=calValue){
     Serial.println(calIncrease);
     calValue = calIncrease;
        }
  }
  // Default for Up button
  else{
    ;
  }
  
  Serial.print("pic: "); Serial.println(pic);
}

/*
// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclickUp() {
  Serial.println("Button 1 doubleclick.");
}  // doubleclick1


// This function will be called once, when the button1 is pressed for a long time.
void longPressStartUp() {
  Serial.println("Button 1 longPress start");
}  // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPressUp() {
  Serial.println("Button 1 longPress...");
}  // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStopUp() {
  Serial.println("Button 1 longPress stop");
}  // longPressStop1
*/

//--- Down button actions ---//
void clickDown() {
      // Use Down button to navigate menu screens
  if(pic>=1 && pic<10)
  {
    if (pic >= maxPics_L1)
    {
      pic=1;
    }
    else if(pic < maxPics_L1)
    {
      pic++;
    }
  }
 
  // Use Up button to set calibration offset
  if(pic == 14){
    Serial.println("Calibration offset -1");
     calDecrease = calValue-1;
  if(calDecrease!=calValue){
     Serial.println(calDecrease);
     calValue = calDecrease;
        }
  }
  
  // Default for Down button
  else{
    ;
  }
  Serial.print("pic: "); Serial.println(pic);
}  
/*
void doubleclickDown() {
  Serial.println("Down doubleclick.");  // doubleclick2
}  

void longPressStartDown() {
  Serial.println("Down longPress start");  // longPressStart2
}

void longPressDown() {
  Serial.println("Down longPress...");  // longPress2
}

void longPressStopDown() {
  Serial.println("Down longPress stop");  // longPressStop2
}  // longPressStop2
*/

//----- [Select] button actions -----//
void clickSelect() {
  lastmillis = millis();
//--- 1st level menu ---//
  if (pic>0 && pic<10)
  {
    if (pic >= maxPics_L1)
    {
      pic=1;
    }
    pic = pic+10;
  }

//----- 2nd level menu -----//
//   if (pic>=10 && pic<20)
//    {
//    if (pic >= maxPics_L2)
//    {
//      pic=11;
//    }
//      pic = pic+10;
//    }

  Serial.print("pic: "); Serial.println(pic);
}  

void doubleclickSelect() {
  Serial.println("Select doubleclick.");  // doubleclick2
}  


void longPressStartSelect() {
  Serial.println("Select longPress start");  // longPressStart2
}

void longPressSelect() {
  Serial.println("Select longPress...");  // longPress2
}

void longPressStopSelect() {
    Serial.println("Select longPressStop");
}


//----- Menu button actions -----//
void clickMenu() {
  Serial.println("Menu one click");

  lastmillis = millis();
//----- Loop through menu items -----//
  if(pic>=0 && pic<10)
  {
    if (pic >= maxPics_L1)
    {
      pic=1;
    }
    else if(pic < maxPics_L1)
    {
      pic++;
    }
  }
//----- 1st level menu -----//
   if(pic>=10 && pic<100)
    {
    if (pic >= maxPics_L2)
    {
      pic=11;
    }
    else if (pic < maxPics_L2)
    {
      pic = pic+10;
    }
  }

  Serial.print("pic: "); Serial.println(pic);
}

void doubleclickMenu() {
  lastmillis = millis();
  if (pic == 5 || pic == 6 || pic == 7 || pic == 8) pic=1;
//  if (pic == 12 || pic == 22 || pic == 32 || pic == 42) pic=2;
//  if (pic == 13) pic=3;
//  if (pic == 14) pic=4;
//  if (pic == 15) pic=5;

  Serial.print("pic: "); Serial.println(pic);
}  

void longPressStartMenu() {
  lastmillis = millis();
  if (pic>0 & pic<10)
  {
    pic=pic+10;
  }
  
  Serial.print("pic: "); Serial.println(pic);
}

void longPressStopMenu() {
    lastmillis = millis();
  if (pic>0 & pic<10)
  {
    pic=pic+10;
  }
}

void longPressMenu() {
  ;
}

//----- ButtonA button actions -----//
void clickButtonA() {
  lastmillis = millis();  
  // Use [Menu] warning
  if (pic == 0) {
        u8g2.drawStr(5,0,"Use [Menu]");
  /*      u8g2.drawStr(70,0,(emfValue);
    
    
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(BLACK,WHITE);
    display.setTextSize(2);
    display.println();
    display.println("button to ");    
    display.println("  change  ");
    display.println(" settings ");
    display.display();
    delay(2000);
*/
  Serial.println("ButtonA");
  }
}

//----- ButtonB button actions -----//
void clickButtonB() {
  lastmillis = millis();  
  // Use [Menu] warning
  if (pic == 0) {
  /*  display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(BLACK,WHITE);
    display.setTextSize(2);
    display.println("Use [Menu]");
    display.println("button to ");    
    display.println("  change  ");
    display.println(" settings ");
    display.display();
    delay(2000);
*/
  Serial.println("ButtonB");
  }
}

//----- End of Buttons -----//

void header1()
{
  /*
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("Menu #1: Graphs");
  display.drawLine (0,9,128,9, WHITE);
  */

}

void header2()
{
  /*
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("Menu #2: Settings");
  display.drawLine (0,9,128,9, WHITE);
  */
}
  
void refresh()
{
  /*
  display.display();
  delay(00);
  display.clearDisplay();
*/
}

void menuWarning() {  
  // Use [Menu] warning
  /*  display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(BLACK,WHITE);
    display.setTextSize(2);
    display.println("Use [Menu]");
    display.println("button to ");    
    display.println("  change  ");
    display.println(" settings ");
    display.display();
    delay(2000);
*/
}