// GC9A01A library example
// Color clock demo
// (c) 2024 Pawel A. Hernik
// YouTube videos:
// https://youtu.be/9RZII8Vx2ZY
// https://youtube.com/shorts/9OcUPrnW_0E?si=zKvyCszEqFGFqfnI
// Requires RRE Fonts from:
// https://github.com/cbm80amiga/RREFont

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

#include "RREFont.h"
#include "rre_simple.h"

#include "fastquad.h"
#include "fastsincos.h"

unsigned long frTime;

int secLast=0;
int hr=20, mn=33, sec=20;
int cx,cy;
int sx,sy;
int xs0,ys0,xe0,ye0;
int xs1,ys1,xe1,ye1;

void drawClock()
{
  if(sec<0 || sec>61) return;
  int mina=270-3;
  int wheelStart=0, wheelLen=85*6;
  cx=SCR_WD/2;
  cy=SCR_HT/2;
  uint16_t bg1 = RGBto565(60,60,60);
  uint16_t bg2 = RGBto565(50,50,50);
  uint16_t bg1d = RGBto565(40,40,40);
  uint16_t bg2d = RGBto565(30,30,30);
  uint16_t col;
  int ss,cc;
  int r0=127-1;
  int r1=r0-20;
  int r2=r1-6;
  wheelStart = (mn+hr*60)>>2;
  int st=0,en=60-1;
  if(sec-secLast==1) st=en=sec;
  //if(sec-levelLast1==1) { st=sec-1; en=sec; }
  for(int i=st;i<=en;i++) {
    fastSinCos(i*6+mina,&ss,&cc);
    xs0 = cx+cc*r0/MAXSIN;
    ys0 = cy+ss*r0/MAXSIN;
    xe0 = cx+cc*r1/MAXSIN;
    ye0 = cy+ss*r1/MAXSIN;
    fastSinCos(i*6+6+mina,&ss,&cc);
    xs1 = cx+cc*r0/MAXSIN;
    ys1 = cy+ss*r0/MAXSIN;
    xe1 = cx+cc*r1/MAXSIN;
    ye1 = cy+ss*r1/MAXSIN;
    col = i<=sec ? lcd.rgbWheel(wheelStart+(long)wheelLen*(360-i*6)/360) : (i&1 ? bg1 : bg2);
    //col = i==sec ? lcd.rgbWheel(wheelStart+(long)wheelLen*(360-i*6)/360) : (i&1 ? bg1 : bg2);
    fillQuad(xs0,ys0,xe0,ye0,xe1,ye1,xs1,ys1, col);

    fastSinCos(i*6+mina,&ss,&cc);
    xs0 = cx+cc*r1/MAXSIN;
    ys0 = cy+ss*r1/MAXSIN;
    xe0 = cx+cc*r2/MAXSIN;
    ye0 = cy+ss*r2/MAXSIN;
    fastSinCos(i*6+6+mina,&ss,&cc);
    xs1 = cx+cc*r1/MAXSIN;
    ys1 = cy+ss*r1/MAXSIN;
    xe1 = cx+cc*r2/MAXSIN;
    ye1 = cy+ss*r2/MAXSIN;
    uint8_t r,g,b;
    lcd.rgbWheel(wheelStart+(long)wheelLen*(360-i*6)/360, &r,&g,&b);
    col = i<=sec ? RGBto565(r*5/8,g*5/8,b*5/8) : (i&1 ? bg1d : bg2d);
    //col = i==sec ? RGBto565(r*5/8,g*5/8,b*5/8) : (i&1 ? bg1d : bg2d);
    fillQuad(xs0,ys0,xe0,ye0,xe1,ye1,xs1,ys1, col);
  }
  secLast = sec;
  int x=31,y=90;
  setDigitWdAA(39);
  drawRREChar(x,y,(hr/10)+'0'); x+=39+2;
  drawRREChar(x,y,(hr%10)+'0'); x+=39+3;
  col = sec&1 ? colline[0] : colline[29];
  fillCircle(x+5,y+30-11,5,col);
  fillCircle(x+5,y+30+11,5,sec&1?colline[30]:colline[59]);
  x+=11+3;
  drawRREChar(x,y,(mn/10)+'0'); x+=39+2;
  drawRREChar(x,y,(mn%10)+'0');
}

// --------------------------------------------------------------------------

void setup(void) 
{
  Serial.begin(115200);
  lcd.init();
  lcd.fillScreen(BLACK);
  
  setFont(&rre_digits_arialb60v);
  //setFont(&rre_digits_arial60v);
  setDigitWdAA(13);

  // horizon
  //genColors(colline, 192,193,241, 80,80,120, 30); genColors(colline+30, 245,130,0, 90,60,10, 30);

  // sky blue
  genColors(colline, 210,210,255, 80,80,120, 30);  genColors(colline+30, 80,80,120, 210,210,255, 30);
}

// --------------------------------------------------------------------------

const char *months[] = {"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char *delim = " :";
char bldTime[40];
int month,day,year;

uint8_t str2month(const char * d)
{
  uint8_t i = 13;
  while((--i) && strcmp(months[i], d));
  return i;
}

void setBuildTime()
{
  // Timestamp format: "Mar 3 2019 12:34:56"
  snprintf(bldTime,40,"%s %s\n", __DATE__, __TIME__);
  char *token = strtok(bldTime, delim);
  while(token) {
    int m = str2month((const char*)token);
    if(m>0) {
      month = m;
      token = strtok(NULL, delim);  day = atoi(token);
      token = strtok(NULL, delim);  year = atoi(token) - 1970;
      token = strtok(NULL, delim);  hr = atoi(token);
      token = strtok(NULL, delim);  mn = atoi(token);
      token = strtok(NULL, delim);  sec = atoi(token);
    }
    token = strtok(NULL, delim);
  }
  //snprintf(bld,40,"Build: %02d-%02d-%02d %02d:%02d:%02d\n",mt.year+1970,mt.month,mt.day,mt.hour,mt.minute,mt.second); Serial.println(bld);
}

// --------------------------------------------------------------------------

void loop()
{
  // Clock
  lcd.fillScreen(BLACK);
  secLast=-1;
  setBuildTime();
  while(1) {
    frTime=millis();
    drawClock();
    if(++sec>59) { sec=0; if(++mn>59) { mn=0; if(++hr>23) hr=0; } }
    //for demo only
    if(sec>20&&sec<50) { genColors(colline, 192,193,241, 80,80,120, 30); genColors(colline+30, 245,130,0, 90,60,10, 30); }
    else { genColors(colline, 210,210,255, 80,80,120, 30);  genColors(colline+30, 80,80,120, 210,210,255, 30); }

    while(millis()-frTime<1000);
  }
}


