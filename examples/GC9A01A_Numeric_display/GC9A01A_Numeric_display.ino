// GC9A01A library example
// Numeric display - RREFont vs PropFont
// (c) 2020-24 Pawel A. Hernik
// YouTube videos:
// https://youtu.be/OOvzmHcou4E 
// https://youtu.be/-F7EWPt0yIo
// https://youtu.be/9RZII8Vx2ZY

/*
 GC9A01A 240x240/round connections (only 4+2 wires required):

 #01 VCC -> VCC (3.3V only?)
 #02 GND -> GND
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 DC  -> D9 or any digital
 #06 CS  -> D10 or any digital
 #07 RST -> opt
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "GC9A01A_AVR.h"

#define TFT_DC   9
#define TFT_CS  10
#define BUTTON   3

#define SCR_WD 240
#define SCR_HT 240
GC9A01A_AVR lcd(TFT_CS, TFT_DC);

// define what kind of fonts will be used
#define USE_RRE_FONTS 1

#if USE_RRE_FONTS==1

#include "RREFont.h"
#include "rre_term_10x16.h"
#include "rre_bold13x20.h"
#include "rre_bold13x20v.h"
#include "rre_bold13x20no.h"

RREFont font;
// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }  

#else

#include "PropFont.h"
#include "bold13x20digtop_font.h"
#include "term9x14_font.h"

PropFont font;
// needed for PropFont library initialization, define your drawPixel and fillRect
void customPixel(int x, int y, int c) { lcd.drawPixel(x, y, c); }
void customRect(int x, int y, int w, int h, int c) { lcd.fillRect(x, y, w, h, c); } 
#endif

//-----------------------------------------------------------------------------

unsigned long ms = 0;

void setup() 
{
  Serial.begin(115200);
  lcd.init();
#if USE_RRE_FONTS==1
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values 
#else
  font.init(customPixel, customRect, SCR_WD, SCR_HT); // custom drawPixel and fillRect function and screen width and height values
#endif
  /*
  char txt[20];
  float v;
  Serial.println("|1234567890123456789|");
  v=2.16; dtostrf(v,3,1,txt); Serial.println(String("|")+txt+"|");
  v=2.16; dtostrf(v,5,1,txt); Serial.println(String("|")+txt+"|");
  v=3232.1; dtostrf(v,3,2,txt);  Serial.println(String("|")+txt+"|");
  v=3232.1545; dtostrf(v,12,2,txt);  Serial.println(String("|")+txt+"|");
  */
}

const uint16_t lnCol  = RGBto565(255,80,255);
const uint16_t ln2Col = RGBto565(180,180,180);
const uint16_t labCol = RGBto565(250,250,250);
const uint16_t v1Col  = RGBto565(50,250,50);
const uint16_t v2Col  = RGBto565(255,250,50);
const uint16_t v3Col  = RGBto565(120,255,255);
const uint16_t v4Col  = RGBto565(255,70,70);
const uint16_t v5Col  = RGBto565(70,70,255);
int mode=0,lastMode=-1;

void setBigNumFont()
{
#if USE_RRE_FONTS==1
  font.setFont(&rre_Bold13x20v);
  //font.setFont(&rre_Bold13x20);  // regular RRE rendered with rectangles
  //font.setFont(&rre_Bold13x20no);  // like above but no overlapping
#else
  font.setFont(Bold13x20);
#endif
  font.setSpacing(1);
  font.setScale(1,2);
  font.setDigitMinWd(16);
}

void setInfoFont()
{
#if USE_RRE_FONTS==1
  font.setFont(&rre_term_10x16);
#else
  font.setFont(Term9x14);
#endif
}

void drawField(int x, int y, int w, int h, char *label, uint16_t col=lnCol)
{
  lcd.drawRect(x,y+7,w,h-7,col);
  setInfoFont();
  font.setScale(1);
  font.setColor(labCol,BLACK);
  int wl = font.strWidth(label);
  font.printStr(x+(w-wl)/2,y,label);
}

void showVal(float v, int x, int y, int w, int p, uint16_t col)
{
  setBigNumFont();
  font.setColor(col,BLACK);
  char txt[w+1];
  dtostrf(v,w,p,txt);
  font.printStr(x,y,txt);
  //Serial.println(txt);
}

void constData()
{
  drawField(    0,  0,120-5,80-2," Temp ");
  drawField(120+5,  0,120-5,80-2," Time ");
  drawField(    0, 81,120-5,80-2," Pressure ");
  drawField(120+5, 81,120-5,80-2," Altitude ");
  drawField(    0,162,240,80-2," Big Number ",ln2Col);
  setBigNumFont();
  int wv=font.strWidth("88.8");
  font.setColor(v1Col); font.printStr(18+wv,0+24,"'$");
  wv=font.strWidth("999999999.99");
  font.setColor(v5Col); font.printStr(22+wv,162+25,"'");
  wv=font.strWidth("888.8");
  int wv2=font.strWidth("888");
  setInfoFont();
  font.setScale(1,2);
  font.setColor(v4Col); font.printStr(21+120+wv+2,82+22+14,"m");
  font.setColor(v2Col); font.printStr(32+120+wv2+3,24+12,"ms");
}

float v1=0,v3=0,v4=0,v5=0;
int tm = 0;

void varData()
{
  //v1=88.8; v3=8888.8; v4=888.8; v5=8888888.88; // const values to test performance
// PropFont optimizing: noopt=350ms, ff=317ms, 0+ff=298ms, +f0=290ms +0f=277ms
  showVal(v1, 18,0+24, 4,1, v1Col);
  showVal(tm, 32+120,0+24, 3,0, v2Col);
  showVal(v3, 14,82+24, 6,1, v3Col);
  showVal(v4, 21+120,82+24, 5,1, v4Col);
  showVal(v5, 22,162+25, 12,2, v5Col);
  v1+=1.1; if(v1>99.0) v1=0;
  v3+=10.1; if(v3>9999.0) v3=0;
  v4+=3.3; if(v4>999.0) v4=0;
  v5+=941340.32; if(v5>=999999990.0) v5=0;
}

void loop()
{
  if(mode!=lastMode) {
    lastMode=mode;
    lcd.fillScreen(BLACK);
    constData();
  }
  ms = millis();
  varData();
  tm = millis()-ms;
  //Serial.println(tm);
}

