#include <Arduino.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <EEPROM.h>

#define CS_PIN D2
XPT2046_Touchscreen ts(CS_PIN);
#define TS_MINX 250
#define TS_MINY 200 // calibration points for touchscreen
#define TS_MAXX 3800
#define TS_MAXY 3750
#define TFT_DC D4
#define TFT_CS D8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define DoseBACKGROUND 0x0455
int ColorPaletteHigh = 30; // Height of palette boxes
int color = WHITE;     //Starting paint brush color
unsigned int colors[10] = {RED, GREEN, BLUE, BLACK, CYAN, YELLOW, WHITE, MAGENTA, BLACK, BLACK};
unsigned int ColourBack;
unsigned int ColourDose;
int WasTouched;
unsigned long currentMicros;
unsigned long previousMicros;
int x, y;
unsigned int previousIntMicros;
int page = 0;
int DoseUnit;
int Counts;
int CountUnit;
int CumuRate;
float CumuDose;
int IntOption;
int IntTime = 5;
int JustSwitchedPage = 0;
int JustSwitchedLED = 0;
int JustSwitchedBuzzer = 0;
int LED;
int BUZZER;
int Updated;
int AlertVal;
int CalibrationVal;
int TimedCountVal;
int BatteryIn;
float BatteryPer;
int BatteryUpdate;
int BatteryUpdateCounter = 29;
float BatteryVal;
float Battery_Per;

char BackgroundText[30];
//EEPROM ADDRESSES
unsigned int conversionFactor = 175;
const int DoseUnitAdd =0;
const int AlertThreshAdd = 1;
const int CalValAdd =2;
int JustSwitchedITime;
float CountingPer;
float ProgCountVal;
int StartCountVal;
unsigned long AvgCount;
unsigned long CurCount;  // incremented by interrupt
unsigned long PrevCount; // to activate buzzer and LED
unsigned long CumuCount;
const int interruptPin = 5;
int integrationMode;
long Count[61];
long FastCount[6]; // arrays to store running Counts
long SlowCount[181];
int i = 0;         // array elements
int j = 0;
int k = 0;
void ICACHE_RAM_ATTR isr();
float DoseRate;
float TotalDose;
char Dose[5];
int DoseLevel;               // determines home screen warning signs
int PreviousDoseLevel;
const unsigned char gammaBitmap [] PROGMEM = {
  0x30, 0x00, 0x78, 0x70, 0xe8, 0xe0, 0xc4, 0xe0, 0x84, 0xc0, 0x05, 0xc0, 0x05, 0x80, 0x07, 0x80,
  0x03, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x3e, 0x00,
  0x1c, 0x00, 0x00, 0x00
};

const unsigned char betaBitmap [] PROGMEM = {
  0x00, 0xc0, 0x00, 0x03, 0xf0, 0x00, 0x07, 0x18, 0x00, 0x06, 0x18, 0x00, 0x0e, 0x18, 0x00, 0x0e,
  0x18, 0x00, 0x0e, 0xf8, 0x00, 0x0e, 0x1c, 0x00, 0x0e, 0x0c, 0x00, 0x0e, 0x0c, 0x00, 0x0e, 0x0c,
  0x00, 0x0e, 0x0c, 0x00, 0x0f, 0x1c, 0x00, 0x0f, 0xf8, 0x00, 0x0e, 0x00, 0x00, 0x0e, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char wifiBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x0f, 0xfe, 0x00, 0x3f, 0xff, 0x80, 0x78,
  0x03, 0xc0, 0xe0, 0x00, 0xe0, 0x47, 0xfc, 0x40, 0x0f, 0xfe, 0x00, 0x1c, 0x07, 0x00, 0x08, 0x02,
  0x00, 0x01, 0xf0, 0x00, 0x03, 0xf8, 0x00, 0x01, 0x10, 0x00, 0x00, 0x40, 0x00, 0x00, 0xe0, 0x00,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char settingsBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x01, 0xc0, 0x7f, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x7f, 0xe0, 0xfc, 0x00, 0x00,
  0x00, 0x07, 0xf9, 0xff, 0xf9, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x7f, 0xfc, 0x00, 0x00,
  0x00, 0x03, 0xff, 0xc0, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x1f, 0xff, 0x80, 0x00,
  0x00, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x01, 0xff, 0xff, 0x00, 0x07, 0xff, 0xf8, 0x00,
  0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xf8, 0x00,
  0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xf8, 0x00,
  0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xf0, 0x00,
  0x00, 0x1f, 0xff, 0x80, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x3f, 0xfc, 0x00, 0x00,
  0x00, 0x03, 0xff, 0xe0, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x07, 0xf9, 0xff, 0xf9, 0xfe, 0x00, 0x00,
  0x00, 0x03, 0xf0, 0x7f, 0xe0, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x7f, 0xe0, 0x38, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char buzzerOnBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x0c, 0x00,
  0x00, 0x00, 0x07, 0x80, 0x0e, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x80,
  0x07, 0x00, 0x00, 0x00, 0x3f, 0x80, 0xc7, 0x80, 0x00, 0x00, 0xff, 0x80, 0xe3, 0x80, 0x00, 0x01,
  0xff, 0x80, 0xf3, 0xc0, 0x00, 0x03, 0xff, 0x80, 0x71, 0xc0, 0x00, 0x07, 0xff, 0x8c, 0x79, 0xc0,
  0x3f, 0xff, 0xff, 0x9e, 0x38, 0xe0, 0x3f, 0xff, 0xff, 0x8e, 0x38, 0xe0, 0x3f, 0xff, 0xff, 0x8e,
  0x3c, 0xe0, 0x3f, 0xff, 0xff, 0x87, 0x1c, 0xe0, 0x3f, 0xff, 0xff, 0x87, 0x1c, 0x60, 0x3f, 0xff,
  0xff, 0x87, 0x1c, 0x70, 0x3f, 0xff, 0xff, 0x87, 0x1c, 0x70, 0x3f, 0xff, 0xff, 0x87, 0x1c, 0x70,
  0x3f, 0xff, 0xff, 0x87, 0x1c, 0x70, 0x3f, 0xff, 0xff, 0x87, 0x1c, 0x70, 0x3f, 0xff, 0xff, 0x87,
  0x1c, 0xe0, 0x3f, 0xff, 0xff, 0x8e, 0x3c, 0xe0, 0x3f, 0xff, 0xff, 0x8e, 0x38, 0xe0, 0x3f, 0xff,
  0xff, 0x9e, 0x38, 0xe0, 0x00, 0x07, 0xff, 0x8c, 0x79, 0xc0, 0x00, 0x03, 0xff, 0x80, 0x71, 0xc0,
  0x00, 0x00, 0xff, 0x80, 0xf1, 0xc0, 0x00, 0x00, 0x7f, 0x80, 0xe3, 0x80, 0x00, 0x00, 0x3f, 0x80,
  0xc7, 0x80, 0x00, 0x00, 0x1f, 0x80, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x0f, 0x00, 0x00, 0x00,
  0x07, 0x80, 0x0e, 0x00, 0x00, 0x00, 0x03, 0x80, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char buzzerOffBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80,
  0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xff,
  0xff, 0x8f, 0x00, 0x78, 0x7f, 0xff, 0xff, 0x8f, 0x80, 0xf8, 0x7f, 0xff, 0xff, 0x8f, 0xc1, 0xf8,
  0x7f, 0xff, 0xff, 0x87, 0xe3, 0xf0, 0x7f, 0xff, 0xff, 0x83, 0xf7, 0xe0, 0x7f, 0xff, 0xff, 0x81,
  0xff, 0xc0, 0x7f, 0xff, 0xff, 0x80, 0xff, 0x80, 0x7f, 0xff, 0xff, 0x80, 0x7f, 0x00, 0x7f, 0xff,
  0xff, 0x80, 0x7f, 0x00, 0x7f, 0xff, 0xff, 0x80, 0xff, 0x80, 0x7f, 0xff, 0xff, 0x81, 0xff, 0xc0,
  0x7f, 0xff, 0xff, 0x83, 0xf7, 0xe0, 0x7f, 0xff, 0xff, 0x87, 0xe3, 0xf0, 0x7f, 0xff, 0xff, 0x8f,
  0xc1, 0xf0, 0x7f, 0xff, 0xff, 0x8f, 0x80, 0xf8, 0x3f, 0xff, 0xff, 0x8f, 0x00, 0x70, 0x3f, 0xff,
  0xff, 0x84, 0x00, 0x20, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00,
  0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char ledOnBitmap[] PROGMEM = {
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x18, 0x07, 0x00, 0xc0, 0x00, 0x00, 0x1c, 0x07, 0x01, 0xc0, 0x00, 0x00, 0x1e,
  0x07, 0x03, 0xc0, 0x00, 0x00, 0x0e, 0x07, 0x03, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x1e, 0x00, 0x1f, 0xc0, 0x03, 0xc0, 0x0f, 0x80, 0x7f, 0xf0, 0x0f, 0x80, 0x07, 0xc1,
  0xff, 0xfc, 0x1f, 0x00, 0x03, 0xc3, 0xe0, 0x3e, 0x1e, 0x00, 0x00, 0x07, 0xc0, 0x0f, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x03,
  0x80, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x1c, 0x00, 0x01, 0xc0, 0x00, 0x7f, 0x1c,
  0x00, 0x01, 0xc3, 0xf0, 0x7f, 0x1c, 0x00, 0x01, 0xc7, 0xf0, 0x3c, 0x0e, 0x00, 0x03, 0x81, 0xe0,
  0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x07,
  0x80, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 0x80, 0x0f, 0x00, 0x00, 0x01, 0xc3,
  0xc0, 0x1e, 0x1c, 0x00, 0x07, 0xc1, 0xc0, 0x1c, 0x1f, 0x00, 0x0f, 0x81, 0xe0, 0x3c, 0x0f, 0x80,
  0x1e, 0x00, 0xe0, 0x38, 0x03, 0xc0, 0x0c, 0x00, 0xe0, 0x38, 0x01, 0x80, 0x00, 0x00, 0xf0, 0x78,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00
};

const unsigned char ledOffBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x01,
  0xff, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x3e, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x0f, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x03,
  0x80, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x1c, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x1c,
  0x00, 0x01, 0xc0, 0x00, 0x00, 0x1c, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00,
  0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x07,
  0x80, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x03,
  0xc0, 0x1e, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x1c, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x3c, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x78,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00
};

const unsigned char BackBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
  0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
  0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

long EEPROMReadlong(long address);
void EEPROMWritelong(int address, long value); // logging functions
unsigned long StartMillis;
unsigned long IntervalMillis;
unsigned long ElapsedTime;
unsigned long ConvertedVal;
unsigned long CurrentTime;
unsigned long PreviousMillis;
float CPM;
float PerCountVal;
int CompletedCount;


void setup() {
  Serial.begin(115200);

  ts.begin();
  ts.setRotation(2);
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
 EEPROM.begin(4096);
  //    tft.setFont(&FreeSans9pt7b);
    DrawHomePage();
     pinMode(D3, OUTPUT);
       digitalWrite(D3, LOW);
  //DrawSettingsPage();
  //DrawUnitsSettings();
//  DrawAlertThreshSet();
//  DrawCalibrationSettings();
 // DrawTimedCountPage();
//  DrawCountingPage();
  attachInterrupt(interruptPin, isr, FALLING);
  DoseUnit = EEPROM.read(DoseUnitAdd);
  AlertVal = EEPROM.read(AlertThreshAdd);
  CalibrationVal = EEPROM.read(CalValAdd);
}




void loop() {
    CurrentTime=millis();
    BatteryUpdateCounter ++;
  if (BatteryUpdateCounter == 30){
    BatteryIn=analogRead(A0);
//    Serial.print("Battery: ");
//    Serial.println(BatteryIn);
    BatteryIn=constrain(BatteryIn, 590,800);
 //   Serial.print("Battery2: ");
 //   Serial.println(BatteryIn);
    BatteryPer = map(BatteryIn,590,800,100,0);
    BatteryVal = BatteryPer/100;
 //       Serial.print("BatteryVal: ");
 //   Serial.println(BatteryVal);
  Battery_Per = 24 * BatteryVal;
  tft.fillRect(211, 5, Battery_Per, 12, BLACK);
  BatteryUpdateCounter=0;
 //     Serial.print("BatteryPer: ");
 //   Serial.println(Battery_Per);
  }
  int X_Coord;
  int Y_Coord;
  float Battery_Val;
  float Battery_Per;
  if (JustSwitchedPage == 1) {
    JustSwitchedPage = 0;
  }
  if (JustSwitchedLED == 1) {
    JustSwitchedLED = 0;
  }
  if (JustSwitchedBuzzer == 1) {
    JustSwitchedBuzzer = 0;
  }
  //battery


  if (page == 0) {
     if (CurrentTime - PreviousMillis >= 1000){
            PreviousMillis = CurrentTime;
         Count[i] = CurCount;
      i++;
      FastCount[j] = CurCount; // keep concurrent arrays of Counts. Use only one depending on user choice
      j++;
      SlowCount[k] = CurCount;
      k++;

      if (i == 61)
      {
        i = 0;
      }

      if (j == 6)
      {
        j = 0;
      }

      if (k == 181)
      {
        k = 0;
      }

      if (integrationMode == 2)
      {
        AvgCount = (CurCount - SlowCount[k]) / 3;
      }

      if (integrationMode == 1)
      {
        AvgCount = (CurCount - FastCount[j]) * 12;
      }

      else if (integrationMode == 0)
      {
        AvgCount = CurCount - Count[i]; // Count[i] stores the value from 60 seconds ago
      }

      AvgCount = ((AvgCount) / (1 - 0.00000333 * float(AvgCount))); // acCounts for dead time of the geiger tube. relevant at high Count rates

      if (DoseUnit == 0)
      {
        DoseRate = AvgCount / float(conversionFactor);
        TotalDose = CumuCount / (60 * float(conversionFactor));
        
      }
      else if (DoseUnit == 1)
      {
        DoseRate = AvgCount / float(conversionFactor * 10.0);
        TotalDose = CumuCount / (60 * float(conversionFactor * 10.0)); // 1 mRem == 10 uSv
        
      }

      if (AvgCount < conversionFactor/2) {
        DoseLevel = 0; // determines alert level displayed on homescreen
          AlertLevelCheck();
     }
      else if (AvgCount < AlertVal * conversionFactor){
        DoseLevel = 1;
          AlertLevelCheck();
      }
      else
      {
        DoseLevel = 2;}

      if (DoseRate < 10.0)
      {
        dtostrf(DoseRate, 4, 2, Dose); // display two digits after the decimal point if value is less than 10
      }
      else if ((DoseRate >= 10) && (DoseRate < 100))
      {
        dtostrf(DoseRate, 4, 1, Dose); // display one digit after decimal point when Dose is greater than 10
      }
      else if ((DoseRate >= 100))
      {
        dtostrf(DoseRate, 4, 0, Dose); // whole numbers only when Dose is higher than 100
      }
      else {
        dtostrf(DoseRate, 4, 0, Dose);  // covers the rare edge case where the Dose rate is sometimes errorenously calculated to be negative
      }
         DrawDoseEffect();


      Serial.println(CurCount);
     }  // end of millis()-controlled block that runs once every second. The rest of the code on page 0 runs every loop
    if (CurCount > PrevCount)
    {
      PrevCount = CurCount;
      previousMicros = micros();
    }
    currentMicros = micros();
    if (currentMicros - previousMicros >= 200)
    {
      digitalWrite(D3, LOW);
      digitalWrite(D0, LOW);
      previousMicros = currentMicros;
    }

  

    if (!ts.touched()) {
      WasTouched = 0;
    }
    if (ts.touched() && !WasTouched)
    {
      WasTouched = 1;
      TS_Point p = ts.getPoint();
      x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
      y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
      Serial.print(", x = ");
      Serial.print(x);
      Serial.print(", y = ");
      Serial.println(y);
      if ((x > 170 && x < 230) && (y > 2 && y < 50))
      {
        if (page == 0 && JustSwitchedPage == 0)
        {
          DrawSettingsPage();
          Serial.println(page);
          Serial.println(JustSwitchedPage);
          Serial.println("S2");
        }
      }
      //LEDCHECK
      if ((x > 5 && x < 90) && (y > 100 && y < 150))
      {
        if (LED == 0 && JustSwitchedLED == 0) {
          LED = 1;
          JustSwitchedLED = 1;
          DrawLEDHome();
        }
        if (LED == 1 && JustSwitchedLED == 0) {
          LED = 0;
          JustSwitchedLED = 1;
          DrawLEDHome();
        }
      }
      if (LED==1){
          digitalWrite(D3, HIGH);
      }
      if(LED==0){
          digitalWrite(D3, LOW);
      }
    if ((x>85 && x<165) && (y>5&&y<40)){
DrawTimedCountPage();    }
    if((x > 5 && x < 89) && (y > 5&& y < 40)){
       integrationMode ++;
        if (integrationMode == 3)
        {
          integrationMode = 0;
        }
        CurCount = 0;
        PrevCount = 0;
        for (int a = 0; a < 61; a++) // reset Counts when integretation speed is changed
        {
          Count[a] = 0;
        }
        for (int b = 0; b < 6; b++)
        {
          FastCount[b] = 0;
        }
        for (int c = 0; c < 181; c++)
        {
          SlowCount[c] = 0;
        }
        if (integrationMode == 3)
        {
          integrationMode = 0;
        }
        DrawIntTime();
      }

      //BUZZCHECK
      if ((x > 5 && x < 85) && (y > 56 && y < 97)){
        if (BUZZER == 0 && JustSwitchedBuzzer == 0) {
          BUZZER = 1;
          JustSwitchedBuzzer = 1;
          DrawBuzzerHome();
        }
        if (BUZZER == 1 && JustSwitchedBuzzer == 0) {
          BUZZER = 0;
          JustSwitchedBuzzer = 1;
          DrawBuzzerHome();
        }
        Serial.println(BUZZER);
        Serial.println(LED);
      }
    }
    //END OF PAGE0
    
  }
 


  if (page == 1) {
    if (!ts.touched()) {
      WasTouched = 0;
    }
    if (ts.touched() && !WasTouched)
    {
      WasTouched = 1;
      TS_Point p = ts.getPoint();
      x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
      y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
      Serial.print(", x = ");
      Serial.print(x);
      Serial.print(", y = ");
      Serial.println(y);
      if ((x > 170 && x < 230) && (y > 15 && y < 50))
      {
        if (page == 1 && JustSwitchedPage == 0);
        {
          DrawHomePage();
          Serial.println(page);
          Serial.println(JustSwitchedPage);
          Serial.println("S1");
        }
      }
    if ((x > 10 && x < 230) && ( y > 115 && y < 160)) {
      if (page == 1 && JustSwitchedPage == 0) {
        DrawAlertThreshSet();
      }
    }
      if ((x > 10 && x < 230) && ( y > 190 && y < 210))
      {
        if (page == 1 && JustSwitchedPage == 0) {
          DrawUnitsSettings();
        }
      }
            if((x>10 && x<230) && ( y>60 && y<105)){
              DrawCalibrationSettings();
            }

    }
  }

if (page == 2) {
  if (!ts.touched()) {
    WasTouched = 0;
  }
  if (ts.touched() && !WasTouched)
  {
    WasTouched = 1;
    TS_Point p = ts.getPoint();
    x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print(", x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
          if ((x > 10 && x < 230) && ( y > 190 && y < 210))
      {
        if (page == 2 && JustSwitchedPage == 0) {
          if(DoseUnit==1){
            DoseUnit=0;
            DrawUnitsOptions();
          }
        }
      }
    if ((x > 170 && x < 230) && (y > 15 && y < 50))
    {
      if (page == 2 && JustSwitchedPage == 0);
      {
        if (EEPROM.read(DoseUnitAdd) != DoseUnit){
          EEPROM.write(DoseUnitAdd, DoseUnit);
          EEPROM.commit();
        }
        DrawSettingsPage();
        
      }
    }
    
    if ((x > 10 && x < 230) && ( y > 115 && y < 160)) {
      if (page == 2 && JustSwitchedPage == 0) {
      if(DoseUnit==0){
        DoseUnit=1;
        DrawUnitsOptions();
        
      }

      }
    }
  }

}
if (page == 3) {
  tft.setCursor(132,175);
  tft.println(AlertVal);
  if (!ts.touched()) {
    WasTouched = 0;
  }
  if (ts.touched() && !WasTouched)
  {
    WasTouched = 1;
    TS_Point p = ts.getPoint();
    x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print(", x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
    if ((x > 170 && x < 230) && (y > 15 && y < 50))
    {
      if (page == 3 && JustSwitchedPage == 0);
      {
         if (EEPROM.read(AlertThreshAdd) != AlertVal){
          EEPROM.write(AlertThreshAdd, AlertVal);
          EEPROM.commit();
        }
        DrawSettingsPage();
      }
    }
    
    if ((x > 85 && x < 122) && ( y > 178 && y < 210)) {
      if (page == 3 && JustSwitchedPage == 0) {
      AlertVal=AlertVal+1;
        tft.fillRect(120,136,60,60,BLACK);
      }
      }
    if ((x > 80 && x < 122) && ( y > 70 && y < 84)) {
      AlertVal=AlertVal-1;
        tft.fillRect(120,136,60,60,BLACK);
    }
  }
}

if (page == 4) {
  tft.setFont(&FreeSans12pt7b);
  tft.setCursor(170,175);
  tft.println(CalibrationVal);
  if (!ts.touched()) {
    WasTouched = 0;
  }
  if (ts.touched() && !WasTouched)
  {
    WasTouched = 1;
    TS_Point p = ts.getPoint();
    x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print(", x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
    if ((x > 170 && x < 230) && (y > 15 && y < 50))
    {
      if (page == 4 && JustSwitchedPage == 0);
      {
      if (EEPROM.read(CalValAdd) != CalibrationVal){
          EEPROM.write(CalValAdd, CalibrationVal);
          EEPROM.commit();
        }
        DrawSettingsPage();
      }
    }
    
    if ((x > 35 && x < 70) && ( y > 178 && y < 215)) {
      CalibrationVal=CalibrationVal+1;
        tft.fillRect(160,136,60,60,BLACK);
      
      }
      
    if ((x > 35 && x < 70) && ( y > 60 && y < 88)) {
      CalibrationVal=CalibrationVal-1;
        tft.fillRect(160,136,60,60,BLACK);
    }
  }
}
if(page==5){
  tft.setFont(&FreeSans12pt7b);
  tft.setCursor(170,175);
  tft.println(TimedCountVal);
    if (!ts.touched()) {
    WasTouched = 0;
  }
  if (ts.touched() && !WasTouched)
  {
    WasTouched = 1;
    TS_Point p = ts.getPoint();
    x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print(", x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
    if ((x > 4 && x < 66) && (y > 0 && y < 40))
    {
      if (page == 5 && JustSwitchedPage == 0);
      {
       StartCountVal=1;
        DrawCountingPage();
      }
    }
    
    if ((x > 170 && x < 230) && (y > 15 && y < 50))
    {
      if (page == 5 && JustSwitchedPage == 0);
      {
        DrawSettingsPage();
      }
    }
    
    if ((x > 35 && x < 70) && ( y > 178 && y < 215)) {
      TimedCountVal=TimedCountVal+1;
        tft.fillRect(160,136,40,40,BLACK);

        Serial.println( " plus");
      
      }
    if ((x > 35 && x < 70) && ( y > 60 && y < 88)) {
      TimedCountVal=TimedCountVal-1;
        tft.fillRect(160,136,40,40,BLACK);

        Serial.println("minus");
    }
  }
}
if(page==6){

  Serial.print("StartMillis: ");
  Serial.println(StartMillis);
  tft.setFont(&FreeSans12pt7b);
  tft.setCursor(170,180);
  tft.println(TimedCountVal);
  ConvertedVal=TimedCountVal;
  ConvertedVal=ConvertedVal*60000;
  Serial.print("ConvertedVal: ");
  Serial.println(ConvertedVal);
  PerCountVal =  PerCountVal=StartMillis;
PerCountVal = PerCountVal/ConvertedVal;
  Serial.print("PerCountVal: ");
  Serial.println(PerCountVal);
DrawCountingRefresh();
  if (PerCountVal==1){
tft.fillRect(72, 266, 95, 50, GREEN);
tft.println("Ok");
StartCountVal=0;
  }
  
    if (!ts.touched()) {
    WasTouched = 0;
  }
  if (ts.touched() && !WasTouched)
  {
    WasTouched = 1;
        TS_Point p = ts.getPoint();
    x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print(", x = ");
    Serial.print(x);
    Serial.print(", y = ");
    Serial.println(y);
      if ((x > 70 && x < 170) && ( y > 0 && y < 44)) {
       if (page == 6 && JustSwitchedPage == 0);
      {
        DrawTimedCountPage();
      }
      }
      }

    }






}








void DrawHomePage() {
  page = 0;
  JustSwitchedPage = 1;
  AlertLevelCheck();
  tft.setFont();
  //DoseRATE
  tft.fillScreen(ILI9341_BLACK);
  DrawBattery();
  DrawDoseHome();
  DrawBackgroundHome();
  DrawCumulativeDose();
  DrawBuzzerHome();
  DrawLEDHome();
  DrawTimedCount();
  DrawIntTime();
  //SETTINGS
  tft.fillRect(2, 263, 59, 59, GREEN);
  tft.drawBitmap(1, 264, settingsBitmap, 58, 58, ILI9341_WHITE);
}

void AlertLevelCheck(){
  if (DoseLevel == 0){
    
    ColourDose = BLUE;
    ColourBack = GREEN;
  }

  if(DoseLevel == 1){
    ColourDose = YELLOW;
    ColourBack = YELLOW;
  }

  if(DoseLevel == 2){
    ColourDose = RED;
    ColourBack = RED;
  }

}
void DrawBattery() {
  tft.setFont();
  tft.fillRect(0,0,240,23, WHITE);
  tft.fillRect(1, 1, 238, 21, BLACK);
  tft.drawRect(210, 4, 26, 14, ILI9341_WHITE);
  tft.drawLine(209, 8, 209, 13, ILI9341_WHITE); // Battery symbol
  tft.drawLine(208, 8, 208, 13, ILI9341_WHITE);
  tft.fillRect(211, 5, 24, 12, GREEN);
  tft.setCursor(2, 8);
  tft.setTextColor(YELLOW);
  tft.setTextSize(1);
  tft.println("rm2020");
}
void DrawBuzzerHome() {
  //BUZZER
  tft.fillRect(153, 210, 84, 50, RED);
  if (BUZZER == 1) {
    tft.drawBitmap(173, 214, buzzerOnBitmap, 45, 45, ILI9341_WHITE);
  }
  if (BUZZER == 0) {
    tft.drawBitmap(173, 214, buzzerOffBitmap, 45, 45, ILI9341_WHITE);
  }
}
void DrawDoseHome() {

  tft.fillRect(0, 27, 240, 80, ColourDose);
  tft.setCursor(6, 47);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Effective Dose Rate");

  tft.setCursor(110, 70);
//  tft.print(Dose);
  if (DoseUnit == 0) {
    tft.println(" uS/h");
  }
  else if (DoseUnit == 1) {
    tft.println(" mRem/h");
    
  }
}
void DrawDoseEffect(){
  tft.fillRect(60,60,60,30,ColourDose);
    tft.setCursor(65, 85);

    tft.print(Dose);

}

void DrawBackgroundHome() {
  tft.fillRect(0, 110, 240, 40, ColourBack);
  tft.setTextColor(BLACK);
  tft.setCursor(19, 120);
  if (DoseLevel == 0){
      tft.println("Normal Background");
  }

  if(DoseLevel == 1){
      tft.println("Elevated Background");
  }

  if(DoseLevel == 2){
      tft.println("IN ALERT");
  }  //CountS
  tft.fillRect(0, 153, 150, 35, CYAN);
  tft.setCursor(3, 162);
  tft.print(CurCount);
  if (CountUnit == 0) {
    tft.println(" CPM");
  }
  else if (CountUnit == 1) {
    tft.println(" CPS");
  }
}
void DrawLEDHome() {
  //LED
  tft.fillRect(153, 153, 84, 54, BLUE);
  if (LED == 1) {
    tft.drawBitmap(173, 157, ledOnBitmap, 45, 45, ILI9341_WHITE);
    Serial.println("LEDON");
  }
  if (LED == 0) {
    tft.drawBitmap(173, 157, ledOffBitmap, 45, 45, ILI9341_WHITE);
  }
}
void DrawTimedCount() {
  //TO GO TO TIMED Count
  tft.fillRect(64, 263, 86, 59, GREEN);
  tft.setCursor(82, 285);
  tft.setTextColor(WHITE);
  tft.println("Timed");
  tft.setCursor(83, 305);
  tft.println("Count");
}
void DrawIntTime() {
  //INT TIME
  if (integrationMode==0)
  {
    IntTime=60;
    }
    if (integrationMode==1)
    {
      IntTime=5;
    }
    if (integrationMode==2)
    {
      IntTime=180;
    }
  tft.fillRect(153, 263, 84, 59, GREEN);
  tft.setCursor(179, 290);
  tft.setFont(&FreeSans12pt7b);
  tft.println("INT");
  tft.setCursor(174, 310);
  tft.print(IntTime);
  tft.println(" s");
}

void DrawCumulativeDose() {
  //CUMULATIVEDose
  tft.fillRect(0, 190, 150, 70, MAGENTA);
  tft.setCursor(5, 205);
  tft.setTextSize(0);
  tft.setFont(&FreeSans9pt7b);
  tft.println("Cumulative Dose");
  tft.print(Dose);
  tft.println(" CPM");
  tft.print(Counts);
if(DoseUnit==0){
  tft.println(" uSv/H");
}
if(DoseUnit==1){
  tft.println(" mRem/h");
}
}

void DrawBackButton() {
  tft.fillRect(3, 276, 65, 40, GREEN);
  tft.drawBitmap(3, 273, BackBitmap, 62, 45, WHITE);
}
void DrawSettingNeeds() {
  tft.fillScreen(BLACK);
  DrawBattery();
  DrawBackButton();
  tft.fillRect(0, 23, 240, 50, BLUE);
}

void DrawSettingsPage() {
  page = 1;
  JustSwitchedPage = 1;
  DrawSettingNeeds();
  tft.fillRect(03, 86, 234, 50, GREEN);
  tft.fillRect(03, 142, 234, 50, GREEN);
  tft.fillRect(03, 198, 234, 50, GREEN);

  tft.setCursor(55, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("SETTINGS");

  tft.setTextColor(BLACK);
  tft.setCursor(55, 120);
  tft.println("Dose UNIT");

  tft.setCursor(5, 175);
  tft.println("ALERT THRESHOLD");

  tft.setCursor(38, 230);
  tft.println("CALIBRATION");
}

void DrawUnitsSettings() {
  page = 2;
  JustSwitchedPage = 1;
  DrawSettingNeeds();
  tft.setCursor(80, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("UNITS");
  DrawUnitsOptions();

}

void DrawUnitsOptions() {
  if (DoseUnit == 0) {
    tft.fillRect(03, 86, 234, 50, GREEN);
    tft.fillRect(03, 142, 234, 50, RED);
  }
  if (DoseUnit == 1) {
    tft.fillRect(03, 86, 234, 50, RED);
    tft.fillRect(03, 142, 234, 50, GREEN);
  }
  tft.setTextColor(BLACK);
  tft.setCursor(45, 120);
  tft.println("Sieverts (uSv/h)");
  tft.setCursor(55, 175);
  tft.println("Rems (mR/h)");
}

void DrawAlertThreshSet(){
  page=3;
  DrawSettingNeeds();
  tft.setCursor(5, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("ALERT THRESHOLD");
  tft.setCursor(5,175);
  if(DoseUnit==0){
    tft.println("uSv/hr:");
  }
  if(DoseUnit==1){
    tft.println("mRem/hr:");
  }
  tft.fillRect(120,86,40,40,GREEN);
  tft.fillRect(120,206,40,40,GREEN);

tft.fillRect(138,96,4,20,WHITE);
tft.fillRect(131,104,20,4,WHITE);
tft.fillRect(131,224,20,4,WHITE);
}

void DrawCalibrationSettings(){
    page=4;
  DrawSettingNeeds();
  tft.setCursor(5, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("CALIBRATION");
  tft.setCursor(5,165);
tft.setFont(&FreeSans9pt7b);
    tft.println("Conversion Factor");
    tft.setCursor(5,180);
    tft.println("(CPM per uSv/h)");
tft.setFont();
  tft.fillRect(165,86,40,40,GREEN);
  tft.fillRect(165,206,40,40,GREEN);

tft.fillRect(182,96,4,20,WHITE);
tft.fillRect(176,104,20,4,WHITE);
tft.fillRect(176,224,20,4,WHITE);
}
void DrawTimedCountPage(){
  page=5;
  DrawSettingNeeds();
  tft.setCursor(40, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("TIMED Count");
    tft.setCursor(5,175);
  tft.setFont(&FreeSans9pt7b);
    tft.println("Duration (Minutes)");

  tft.fillRect(165,86,40,40,GREEN);
  tft.fillRect(165,206,40,40,GREEN);
  tft.fillRect(182,96,4,20,WHITE);
  tft.fillRect(176,104,20,4,WHITE);
  tft.fillRect(176,224,20,4,WHITE);
  tft.fillRect(172, 276, 65, 40, GREEN);
  tft.setCursor(177,302);
  tft.setTextColor(WHITE);
  tft.println("BEGIN");
    tft.setFont();
    tft.setTextColor(YELLOW);
}
void DrawCountingRefresh(){
    tft.fillRect(15,115,210*PerCountVal,35,GREEN);
}
void DrawCountingPage(){
    page=6;
    DrawCountingRefresh();
      tft.fillScreen(BLACK);
  DrawBattery();
  tft.fillRect(0, 23, 240, 50, BLUE);
  tft.setCursor(40, 50);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(YELLOW);
  tft.println("TIMED Count");

tft.setCursor(70,100);
tft.println("Progress");
ProgCountVal=TimedCountVal;


//  tft.fillRect(165,206,40,40,GREEN);
  tft.setCursor(177,302);
  tft.drawRect(15,115, 210, 35, ILI9341_WHITE);
tft.setCursor(65,180);
tft.println("Duration");
tft.setCursor(83,296);
  tft.setTextColor(BLACK);
  tft.fillRect(72, 266, 95, 50, GREEN);
tft.println("Cancel");
  tft.setTextColor(YELLOW);
}

long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
 
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
 
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
void isr() // interrupt service routine
{
  if ((micros() - 200) > previousIntMicros){
    CurCount++;
    CumuCount++;
  }
  previousIntMicros = micros();
}
