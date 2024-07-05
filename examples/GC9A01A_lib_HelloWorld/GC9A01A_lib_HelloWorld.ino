// GC9A01A library example
// (c) 2019-24 Pawel A. Hernik

/*
 GC9A01A 240x240 round 1.28" IPS - only 4+2 wires required:

 #01 VCC -> VCC (3.3V only?)
 #02 GND -> GND
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 DC  -> D9 or any digital
 #06 CS  -> D10 or any digital
 #07 RST -> NC
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "GC9A01A_AVR.h"

#define TFT_DC   9
#define TFT_CS  10

#define SCR_WD 240
#define SCR_HT 240
GC9A01A_AVR lcd(TFT_CS, TFT_DC);

void setup(void) 
{
  Serial.begin(9600);
  lcd.init(SCR_WD, SCR_HT);
  lcd.fillScreen(BLACK);
  lcd.setCursor(0, 100);
  lcd.setTextColor(WHITE,BLUE);
  lcd.setTextSize(3);
  lcd.println("HELLO WORLD");
 }

void loop()
{
}

