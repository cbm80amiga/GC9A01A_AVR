// GC9A01A library example
// Analog watch/clock, AVR + optional DS1307/DS3231 RTC module
// (c) 2024 Pawel A. Hernik
// YT videos:
// https://youtu.be/9RZII8Vx2ZY
// https://youtu.be/jFGDFuLhdMc 
// https://youtu.be/35Z0enhEYqM 
// https://youtu.be/Xr-dxPhePhY

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

#define BUTTON   3

#define TFT_CS  -1 // if not used define CS_ALWAYS_LOW in GC9A01A_AVR.h
#define TFT_DC   9
#define TFT_RST  10
GC9A01A_AVR lcd(TFT_CS, TFT_DC, TFT_RST);

//#include "clk.h"
//#include "sansmains.h"
//#include "smartq02.h"
#include "kit.h"
//#include "smartq07.h"
const unsigned char *clockface = dial2bit;

//#include "small4x6_font.h"

struct RTCData {
  int hour,minute,second;
  int year,month,day,dayOfWeek;
};

RTCData cur;

#define USEHW 1
#include <Wire.h>
#include "rtc.h"

uint8_t txt2num(const char* p) 
{
  return 10*(*p-'0') + *(p+1)-'0';
}

// configuration

const uint16_t cx = SCR_WD/2, cy = SCR_HT/2;  // clock center
const uint16_t selHandCol1 = RGBto565(250,250,0);
const uint16_t selHandCol2 = RGBto565(180,180,0);

// bright
//const uint16_t mHandCol1 = RGBto565(220,220,220);
//const uint16_t mHandCol2 = RGBto565(170,170,170);
// dark
const uint16_t mHandCol1 = RGBto565(150,150,150);
const uint16_t mHandCol2 = RGBto565(100,100,100);

const uint16_t hHandCol1 = RGBto565(130,130,130);
const uint16_t hHandCol2 = RGBto565(90,90,90);
//const uint16_t hHandCol1 = mHandCol1;
//const uint16_t hHandCol2 = mHandCol2;
const uint16_t sHandCol1 = RGBto565(250,80,80);
const uint16_t sHandCol2 = RGBto565(200,0,0);
const uint16_t circleCol=RGBto565(50,50,50);
//const uint16_t circleCol=sHandCol1;

int hHandL = 25*2, hHandW = 3*2;
int mHandL = 36*2, mHandW = 3*2;
int sHandL = 44*2, sHandW = 2*2;


int sDeg,mDeg,hDeg;
int sDegOld,mDegOld,hDegOld;
unsigned long styleTime, ms;
uint8_t hh = txt2num(__TIME__+0);
uint8_t mm = txt2num(__TIME__+3);
uint8_t ss = txt2num(__TIME__+6);
uint8_t start = 1;
int setMode = 0;

// ----------------------------------------------------------------

uint16_t palette[4];
uint16_t line[SCR_WD+4];

void imgLineH(int x, int y, int w)
{
  uint8_t v,*img = (uint8_t*)clockface+4*2+6+y*SCR_WD/4+x/4;
  int ww = (x&3)?w+4:w;
  for(int i=0;i<ww;i+=4) {
    v = pgm_read_byte(img++);
    line[i+0] = palette[(v>>6)&3];
    line[i+1] = palette[(v>>4)&3];
    line[i+2] = palette[(v>>2)&3];
    line[i+3] = palette[v&3];
  }
  lcd.drawImage(x,y,w,1,line+(x&3));
}

void imgRect(int x, int y, int w, int h)
{
  for(int i=y;i<y+h;i++) imgLineH(x,i,w);
}

//-----------------------------------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91] PROGMEM = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  while(i<0) i+=360;
  while(i>=360) i-=360;
  if(i<90)  return(pgm_read_byte(&sinTab[i])); else
  if(i<180) return(pgm_read_byte(&sinTab[180-i])); else
  if(i<270) return(-pgm_read_byte(&sinTab[i-180])); else
            return(-pgm_read_byte(&sinTab[360-i]));
}

int fastCos(int i)
{
  return fastSin(i+90);
}

//-----------------------------------------------------------------------------
// simple Amiga like blitter implementation
uint8_t raster[SCR_HT*2];
void rasterize(int x0, int y0, int x1, int y1) 
{
  if((y0<0 && y1<0) || (y0>=SCR_HT && y1>=SCR_HT)) return; // exit if line outside rasterized area
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int err2,err = dx-dy;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  
  while(1)  {
    if(y0>=0 && y0<SCR_HT) {
      if(x0<raster[2*y0+0]) raster[2*y0+0] = x0>0 ? x0 : 0;
      if(x0>raster[2*y0+1]) raster[2*y0+1] = x0<SCR_WD ? x0 : SCR_WD-1;
    }

    if(x0==x1 && y0==y1)  return;
    err2 = err+err;
    if(err2 > -dy) { err -= dy; x0 += sx; }
    if(err2 < dx)  { err += dx; y0 += sy; }
  }
}

void rastClear()
{
  for(int y=0;y<SCR_HT;y++) { raster[2*y+0] = SCR_WD+1; raster[2*y+1] = 0; }
}

void rastFill(uint16_t c)
{
  for(int y=0;y<SCR_HT;y++)
    if(raster[2*y+1]>raster[2*y+0]) if(c) lcd.drawFastHLine(raster[2*y+0],y,raster[2*y+1]-raster[2*y+0]+1,c); else imgLineH(raster[2*y+0],y,raster[2*y+1]-raster[2*y+0]+1);
}

void fillTri(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c=0) 
{
  rastClear();
  rasterize( x0, y0, x1, y1);
  rasterize( x1, y1, x2, y2);
  rasterize( x2, y2, x0, y0);
  rastFill(c);
}

void fillQuad( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c=0) 
{
  rastClear();
  rasterize( x0, y0, x1, y1);
  rasterize( x1, y1, x2, y2);
  rasterize( x2, y2, x3, y3);
  rasterize( x3, y3, x0, y0);
  rastFill(c);
}

//-----------------------------------------------------------------------------

int px[8],py[8];

void drawHand(int deg, int w, int l, int col1=0, int col2=0)
{
  int i,num = 4;
  px[0]= 0, py[0]= l;
  px[1]=-w-1, py[1]= 0;
  px[2]= w+1, py[2]= 0;
  px[3]= 0, py[3]=-15;
  int x[5],y[5];
  int cc = fastCos(deg+180);
  int ss = fastSin(deg+180);
  for(i=0;i<num;i++) {
    x[i] = px[i]*cc - py[i]*ss;
    y[i] = px[i]*ss + py[i]*cc;
    x[i] = cx + (x[i]+(x[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
    y[i] = cy + (y[i]+(y[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
  }
  fillTri(x[0],y[0], x[1],y[1], x[3],y[3], col2);
  fillTri(x[2],y[2], x[3],y[3], x[0],y[0], col1);
}

void drawHandS(int deg, int w, int l, int col1=0, int col2=0)
{
  int i,num = 8;
  px[0]=-w+3, py[0]= l;
  px[1]=-w+3, py[1]=-20;
  px[2]= w-3, py[2]= l;
  px[3]= w-3, py[3]=-20;
  px[4]=-w+1, py[4]=-40;
  px[5]=-w+1, py[5]=-15;
  px[6]= w-1, py[6]=-40;
  px[7]= w-1, py[7]=-15;
  int x[8],y[8];
  int cc = fastCos(deg+180);
  int ss = fastSin(deg+180);
  for(i=0;i<num;i++) {
    x[i] = px[i]*cc - py[i]*ss;
    y[i] = px[i]*ss + py[i]*cc;
    x[i] = cx + (x[i]+(x[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
    y[i] = cy + (y[i]+(y[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
  }
  fillQuad(x[0],y[0], x[1],y[1], x[3],y[3], x[2],y[2], col1);
  fillQuad(x[4],y[4], x[5],y[5], x[7],y[7], x[6],y[6], col1);
}

void clockUpdate() 
{
  if(millis()-ms<1000 && !start) return;
  ms = millis();
  if(setMode==0) {
    //getRTCDateTime(&cur);
    if(1) {
    if(++cur.second>59) {
      cur.second = 0;
      if(++cur.minute>59) {
        cur.minute = 0;
        if(++cur.hour>11) {
          cur.hour = 0;
        }
      }
    }
    }
    ss = cur.second;
    mm = cur.minute;
    hh = cur.hour;
  }

  sDeg = ss*6;

  if(ss==0 || start) {
    start = 0;
    mDeg = mm*6+sDeg/60;
    hDeg = hh*30+mDeg/12;
    drawHand(hDegOld,hHandW,hHandL);
    drawHand(mDegOld,mHandW,mHandL);
    mDegOld = mDeg;
    hDegOld = hDeg;
  }
  
  drawHandS(sDegOld,sHandW,sHandL);
  if(setMode==1)
    drawHand(hDeg,hHandW,hHandL,selHandCol1,selHandCol2);
  else
    drawHand(hDeg,hHandW,hHandL,hHandCol1,hHandCol2);
  if(setMode==2)
    drawHand(mDeg,mHandW,mHandL,selHandCol1,selHandCol2);
  else
    drawHand(mDeg,mHandW,mHandL,mHandCol1,mHandCol2);
  if(setMode==0)
    drawHandS(sDeg,sHandW,sHandL,sHandCol1,sHandCol2);
  sDegOld = sDeg;
  lcd.fillCircle(cx,cy, 4, circleCol);
/*
  lcd.setTextSize(3); lcd.setTextColor(WHITE,BLACK);
  lcd.setCursor(0,0);
  lcd.print(hh); lcd.print(":"); lcd.print(mm); lcd.print(":"); lcd.print(ss);
  lcd.setCursor(0,30);
  lcd.print(cur.day); lcd.print("."); lcd.print(cur.month); lcd.print("."); lcd.print(cur.year+2000);
*/
}

// --------------------------------------------------------------------------
int stateOld = HIGH;
long btDebounce    = 30;
long btDoubleClick = 600;
long btLongClick   = 700;
long btLongerClick = 2000;
long btTime = 0, btTime2 = 0;
int clickCnt = 1;

// 0=idle, 1,2,3=click, -1,-2=longclick
int checkButton()
{
  int state = digitalRead(BUTTON);
  if( state == LOW && stateOld == HIGH ) { btTime = millis(); stateOld = state; return 0; } // button just pressed
  if( state == HIGH && stateOld == LOW ) { // button just released
    stateOld = state;
    if( millis()-btTime >= btDebounce && millis()-btTime < btLongClick ) { 
      if( millis()-btTime2<btDoubleClick ) clickCnt++; else clickCnt=1;
      btTime2 = millis();
      return clickCnt; 
    } 
  }
  if( state == LOW && millis()-btTime >= btLongerClick ) { stateOld = state; return -2; }
  if( state == LOW && millis()-btTime >= btLongClick ) { stateOld = state; return -1; }
  return 0;
}
// --------------------------------------------------------------------------
const char *months[] = {"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char *delim = " :";
char bld[40];

uint8_t str2month(const char * d)
{
  uint8_t i = 13;
  while((--i) && strcmp(months[i], d));
  return i;
}

void setBuildTime()
{
  // Timestamp format: "Mar 3 2019 12:34:56"
  snprintf(bld,40,"%s %s\n", __DATE__, __TIME__);
  char *token = strtok(bld, delim);
  while(token) {
    int m = str2month((const char*)token);
    if(m>0) {
      cur.month = m;
      token = strtok(NULL, delim);  cur.day = atoi(token);
      token = strtok(NULL, delim);  cur.year = atoi(token) - 1970;
      token = strtok(NULL, delim);  cur.hour = atoi(token);
      token = strtok(NULL, delim);  cur.minute = atoi(token);
      token = strtok(NULL, delim);  cur.second = atoi(token);
    }
    token = strtok(NULL, delim);
  }
  //snprintf(bld,40,"Build: %02d-%02d-%02d %02d:%02d:%02d\n",mt.year+1970,mt.month,mt.day,mt.hour,mt.minute,mt.second); Serial.println(bld);
  setRTCTime(&cur);
}

//-----------------------------------------------------------------------------

void setup() 
{
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);
  Wire.begin();
  Wire.setClock(400000);  // faster
  lcd.init(SCR_WD, SCR_HT);
  //getRTCDateTime(&cur);
  if(cur.year+2000<2020) setBuildTime();  //  <2020 - invalid year
  uint16_t *pal = (uint16_t*)clockface+3;
  for(int i=0;i<4;i++) palette[i]=pgm_read_word(pal++);
  imgRect(0,0,SCR_WD,SCR_HT);
  ms = millis(); 
}

void loop()
{
  /*
  int st = checkButton();
  if(st<0 && setMode==0) setMode=1;
  if(setMode>0) {
    if(st>0) { start=1; if(++setMode>2) setMode=0; }
    if(setMode==1 && st<0) { if(++hh>23) hh=0; start=1; delay(600); }
    if(setMode==2 && st<0) { if(++mm>59) mm=0; start=1; delay(200); }
    cur.hour = hh;
    cur.minute = mm;
    setRTCTime(&cur);
  }*/
  clockUpdate();
}

