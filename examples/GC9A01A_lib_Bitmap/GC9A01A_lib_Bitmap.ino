// GC9A01A library example
// (c) 2024 Pawel A. Hernik
// YouTube video:
// https://youtu.be/9RZII8Vx2ZY

/*
 GC9A01A 240x240 round 1.28" IPS, round PCB variant, version with RST and DC only:

 #01 VCC -> VCC (3.3V only?)
 #02 GND -> GND
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 DC  -> D9 or any digital
 #06 CS  -> optional, when not used then CS_ALWAYS_LOW should be defined
 #07 RST -> D10 or any digital

 GC9A01A 240x240 round 1.28" IPS, square PCB variant, version with RST and DC only:

 #01 GND -> GND
 #02 VCC -> VCC (3.3V only?)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RST -> D10 or any digital
 #06 DC  -> D9 or any digital
 #07 CS  -> optional, when not used then CS_ALWAYS_LOW should be defined
 #08 BLK -> NC or VCC
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "GC9A01A_AVR.h"

#define SCR_WD 240
#define SCR_HT 240

#define TFT_CS  -1 // if not used define CS_ALWAYS_LOW in GC9A01A_AVR.h
#define TFT_DC   9
#define TFT_RST  10
GC9A01A_AVR lcd(TFT_CS, TFT_DC, TFT_RST);

#include "bitmap.h"

uint16_t colorBar[50];

void setup(void) 
{
  Serial.begin(9600);
  lcd.init(SCR_WD, SCR_HT);
  lcd.fillScreen(BLACK);

  int i,j;
  for(j=0;j<7;j++) 
    for(i=0;i<7;i++)
      lcd.drawImageF(i*34,j*34,32,32,mario);
  delay(4000);

  for(i=0;i<25;i++) {
    colorBar[i]    = RGBto565(i*256/25,0,i*256/25);
    colorBar[i+25] = RGBto565((24-i)*255/25,0,(24-i)*255/25);
  }
  for(i=0;i<240;i++) {
    lcd.drawImage(i,0,1,50,colorBar);
    lcd.drawImage(i,240-50,1,50,colorBar);
  }
  for(i=50;i<240-50;i++) {
    lcd.drawImage(0,i,50,1,colorBar);
    lcd.drawImage(240-50,i,50,1,colorBar);
  }
  delay(4000);
}

void loop()
{
  lcd.drawImageF(random(0,240-32),random(0,240-32),32,32,mario);
}

