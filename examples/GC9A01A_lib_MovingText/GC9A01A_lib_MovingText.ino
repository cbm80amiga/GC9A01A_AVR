// GC9A01A library example
// (C)2019-24 Pawel A. Hernik
// requires RRE Font library:
// https://github.com/cbm80amiga/RREFont
// YT video:
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

#include "RREFont.h"
#include "rre_chicago_20x24.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(BLACK);

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_chicago_20x24);
  int i;
  for(i=0;i<10;i++) {
    font.setColor(RGBto565(i*25,i*25,i*25));
    font.printStr(30+i,20+i,"Hello");
  }
  font.setColor(WHITE);
  font.printStr(30+i,20+i,"Hello");

  for(i=0;i<10;i++) {
    font.setColor(lcd.rgbWheel(0+i*8));
    font.printStr(25+i,60+i,"World");
  }
  font.setColor(WHITE);
  font.printStr(25+i,60+i,"World");
  delay(4000);
  lcd.fillScreen();
}

#define MAX_TXT 32
byte tx[MAX_TXT];
byte ty[MAX_TXT];
byte cur=0;
int numTxt=16;
int x=0,y=0;
int dx=6,dy=5;
int i,ii,cc=0;

void loop()
{
  x+=dx;
  y+=dy;
  if(x>SCR_WD-20) { dx=-dx; x=SCR_WD-20; }
  if(x<1)   { dx=-dx; x=0; }
  if(y>SCR_HT-20) { dy=-dy; y=SCR_HT-20; }
  if(y<1)   { dy=-dy; y=0; }
  int i=cur;
  //font.setColor(BLACK);
  //font.printStr(tx[i],ty[i],"Hello!");
  tx[cur]=x;
  ty[cur]=y;
  if(++cur>=numTxt) cur=0;
  for(i=0;i<numTxt;i++) {
    ii=i+cur;
    if(ii>=numTxt) ii-=numTxt;
    font.setColor(RGBto565(i*15,i*5,0));
    if(i==numTxt-1) font.setColor(WHITE);
    font.printStr(tx[ii],ty[ii],"Hello!");
    cc++;
  }
  delay(20);
}

