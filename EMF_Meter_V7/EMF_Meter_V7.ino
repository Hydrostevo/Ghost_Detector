//  An EMF detector and Ghost finder
// Global
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>  //http://www.mathertel.de/Arduino/OneButtonLibrary.aspx
#include <FastLED.h>
// Local
#include "Bitmaps.h"

//-------------------------------------------------------------------------------

// OLED Screen: 128 X 64
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (D2=SDA,D1=SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// RGBLED strip (8 RGBLEDS)
#define NUM_LEDS 8 // add number of LEDs of your RGB LED strip
#define PIN_LED 2 // digital output PIN that is connected to DIN of the RGB LED strip
#define LED_COLOR CRGB::DarkOrchid // see https://github.com/FastLED/FastLED/wiki/Pixel-reference for a full list, e.g. CRGB::AliceBlue, CRGB::Amethyst, CRGB::AntiqueWhite...
#define BRIGHTNESS 5
CRGB rgb_led[NUM_LEDS]; // color array of the LED RGB strip


// Set A0 pin
#define PIN_ANTENNA A0
/* Uncomment to set 3v or 5v range. Analog-to-digital converted value depends on the voltage 
   level of the VCC pin (maximum voltage to ESP8266 dev board A0 pin is 3.3v).*/
//#define MAX_PIN_ANTENNA 1023  // ~ 5V
#define MAX_PIN_ANTENNA   700   // ~ 3.3V


// Set button pins & script values 
#define PIN_INPUT1 14 //D5
#define PIN_INPUT2 12 //D6
#define PIN_INPUT3  0 //D3 
#define PIN_INPUT4 13 //D7

// Setup a new OneButton on pins
OneButton UpButton     (PIN_INPUT1, true);
OneButton DownButton   (PIN_INPUT2, true);
OneButton SelectButton (PIN_INPUT3, true);
OneButton MenuButton   (PIN_INPUT4, true);

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
int ghostValue = 120;

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
  
//----- Set Antenna pin -----//  
  pinMode(PIN_ANTENNA, INPUT);

//----- Set FastLED -----//  
  FastLED.setBrightness(BRIGHTNESS );
  FastLED.addLeds<WS2812B, PIN_LED>(rgb_led, NUM_LEDS);
      
//----- Set Display -----//
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

//----- Confirm set up -----//
  delay(1000);
  Serial.println("Buttons set");
  Serial.println("Display live");
  Serial.println("SD card ready");
  Serial.println("RGBLED on");

//----- Start display with spalsh screen and message -----//
  display.clearDisplay();
  display.drawBitmap(0, 0, GhostBusters, 128, 64, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,20);
  display.println("EMF METER");
  display.display();
  delay(2000);


}   // End of Set Up

//----- Show EMF values -----//
void showReadings(int emfValue) {
    display.clearDisplay();   
    display.setTextSize(2);
    display.setTextColor(WHITE);
    // Display EMF on screen
    display.setCursor(5,0);
    display.println("EMF:");
    display.setCursor(70,0);
    display.println(emfValue);
    
    display.display();

 //   Serial.print("EMF: ");
 //   Serial.println(emfValue);   // Print/Plot value in serial terminal
 //   Serial.print("Calibration offset: ");
 //   Serial.println(calValue);
    Serial.print("PIC; ");      Serial.println(pic);
    Serial.print("Menu #");     Serial.println(menuSelected);
//    Serial.print("Temp: ");
//    Serial.println("Add temp here");
//    Serial.println();
}

//====================//
void loop() {

//----- Monitor buttons -----//
  UpButton.tick();
  DownButton.tick();
  SelectButton.tick();
  MenuButton.tick();

//----- Menus -----//
    if (millis() >= (lastmillis + maxtime))
    {
        pic = 0;
    }

    if (pic == 0) // Default if no clicks detected
    {
//----- Capture EMF value -----//
    static int avgValue = 0, emfValue = 0;
    static long nextCheck = 0, emfSum = 0, iterations = 0;

    emfValue = constrain(analogRead(PIN_ANTENNA), 0, 1023) + calValue;
    emfSum += emfValue;
    iterations++;

    if (lmillis() - nextCheck >= 0) {
        avgValue = emfSum / iterations;
        emfSum = 0;
        iterations = 0;
        showReadings(avgValue);
        nextCheck = lmillis() + CHECK_DELAY;
    }


//----- Map emfValue to led range -----//
  int num_leds_switchedon = map(emfValue, 0, MAX_PIN_ANTENNA, 0, NUM_LEDS);  
  for (int i = 0; i < num_leds_switchedon; ++i) {
    rgb_led[i] = LED_COLOR;
  }
  for (int i = num_leds_switchedon; i < NUM_LEDS; ++i) {
    rgb_led[i] = CRGB::Black;
  }
  FastLED.show(); 
    
//----- Draw bar gauge -----//
    display.drawRoundRect(0, 20, 126, 20, 2, WHITE);
    //display.fillRect(3, 25, 120, 13, BLACK);
    display.fillRect(0, 22, map(emfValue, 0, 1023, 0, 118), 16, WHITE);
    display.display();

    }

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

//----------------------------------------------------------------
//---- Detect a ghost -----//
    if (emfValue >ghostValue) {
        Serial.println("Ghost detected");
        display.clearDisplay();
        display.drawBitmap(0, 0, GhostBusters_small, 128, 46, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
        display.display();
        display.setCursor(30,50);
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.println("GHOST!!");
        display.display();
        delay(1000);        
        }
    else {
/*    // Temp output
    display.setTextSize(1);
    display.setCursor(5,50);
    display.println("TEMP:");
    display.setCursor(70,50);
    display.println("21");

  //----- Draw bitmap -----//
  display.clearDisplay();
  display.drawBitmap(0, 0, GhostBusters_small, 128, 64, WHITE); // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.display();
  delay(2000);
  
  */
      ;
  }

}   // end of loop

//----- Button wiring & actions -----//
/*
_____
     |                                  - Button modes -
  E  +- D6 ---<   Up   >---¬            > click
  S  |                     |            > doubleclick
  P  +- D5 ---<  Down  >---|            > longPressStart
  8  |                     |            > longPress
  2  +- D3 ---< Select >---|            > longPressStop      
  6  |                     |
  6  +- D7 ---<  Menu  >---|
_____|                     |
                          Gnd
                           |     
*/

//--- Up button actions ---//
void clickUp() {
lastmillis = millis();  
  if (pic == 0) {
    Serial.print  ("pic: ");
    Serial.println(pic);
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(BLACK,WHITE);
    display.setTextSize(2);
    display.println("Use [Menu]");
    display.println("button to ");    
    display.println("  change  ");
    display.println(" settings ");
    display.display();
    delay(2000);
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
  lastmillis = millis();  
  // Use [Menu] warning
  if (pic == 0) {
    Serial.println("oh my eyes");
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(BLACK,WHITE);
    display.setTextSize(2);
    display.println("Use [Menu]");
    display.println("button to ");    
    display.println("  change  ");
    display.println(" settings ");
    display.display();
    delay(2000);
  }
      
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

//----- End of Buttons -----//

void header1()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("Menu #1: Graphs");
  display.drawLine (0,9,128,9, WHITE);
}

void header2()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("Menu #2: Settings");
  display.drawLine (0,9,128,9, WHITE);
}
  
void refresh()
{
  display.display();
  delay(00);
  display.clearDisplay();
}
