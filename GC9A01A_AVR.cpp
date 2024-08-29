// Fast GC9A01A IPS 240x240 SPI display library
// (c) 2024 by Pawel A. Hernik

#include "GC9A01A_AVR.h"
//#include <limits.h>
//#include "pins_arduino.h"
//#include "wiring_private.h"
#include <SPI.h>

// -----------------------------------------
// GC9A01A commands

#define GC9A01A_TFTWIDTH 240  ///< Display width in pixels
#define GC9A01A_TFTHEIGHT 240 ///< Display height in pixels

#define GC9A01A_SWRESET 0x01   ///< Software Reset (maybe, not documented)
#define GC9A01A_RDDID 0x04     ///< Read display identification information
#define GC9A01A_RDDST 0x09     ///< Read Display Status
#define GC9A01A_SLPIN 0x10     ///< Enter Sleep Mode
#define GC9A01A_SLPOUT 0x11    ///< Sleep Out
#define GC9A01A_PTLON 0x12     ///< Partial Mode ON
#define GC9A01A_NORON 0x13     ///< Normal Display Mode ON
#define GC9A01A_INVOFF 0x20    ///< Display Inversion OFF
#define GC9A01A_INVON 0x21     ///< Display Inversion ON
#define GC9A01A_DISPOFF 0x28   ///< Display OFF
#define GC9A01A_DISPON 0x29    ///< Display ON
#define GC9A01A_CASET 0x2A     ///< Column Address Set
#define GC9A01A_RASET 0x2B     ///< Row Address Set
#define GC9A01A_RAMWR 0x2C     ///< Memory Write
#define GC9A01A_PTLAR 0x30     ///< Partial Area
#define GC9A01A_VSCRDEF 0x33   ///< Vertical Scrolling Definition
#define GC9A01A_TEOFF 0x34     ///< Tearing Effect Line OFF
#define GC9A01A_TEON 0x35      ///< Tearing Effect Line ON
#define GC9A01A_MADCTL 0x36    ///< Memory Access Control
#define GC9A01A_VSCRSADD 0x37  ///< Vertical Scrolling Start Address
#define GC9A01A_IDLEOFF 0x38   ///< Idle mode OFF
#define GC9A01A_IDLEON 0x39    ///< Idle mode ON
#define GC9A01A_COLMOD 0x3A    ///< Pixel Format Set
#define GC9A01A_CONTINUE 0x3C  ///< Write Memory Continue
#define GC9A01A_TEARSET 0x44   ///< Set Tear Scanline
#define GC9A01A_GETLINE 0x45   ///< Get Scanline
#define GC9A01A_SETBRIGHT 0x51 ///< Write Display Brightness
#define GC9A01A_SETCTRL 0x53   ///< Write CTRL Display
#define GC9A01A1_POWER7 0xA7   ///< Power Control 7
#define GC9A01A_TEWC 0xBA      ///< Tearing effect width control
#define GC9A01A1_POWER1 0xC1   ///< Power Control 1
#define GC9A01A1_POWER2 0xC3   ///< Power Control 2
#define GC9A01A1_POWER3 0xC4   ///< Power Control 3
#define GC9A01A1_POWER4 0xC9   ///< Power Control 4
#define GC9A01A_RDID1 0xDA     ///< Read ID 1
#define GC9A01A_RDID2 0xDB     ///< Read ID 2
#define GC9A01A_RDID3 0xDC     ///< Read ID 3
#define GC9A01A_FRAMERATE 0xE8 ///< Frame rate control
#define GC9A01A_SPI2DATA 0xE9  ///< SPI 2DATA control
#define GC9A01A_INREGEN2 0xEF  ///< Inter register enable 2
#define GC9A01A_GAMMA1 0xF0    ///< Set gamma 1
#define GC9A01A_GAMMA2 0xF1    ///< Set gamma 2
#define GC9A01A_GAMMA3 0xF2    ///< Set gamma 3
#define GC9A01A_GAMMA4 0xF3    ///< Set gamma 4
#define GC9A01A_IFACE 0xF6     ///< Interface control
#define GC9A01A_INREGEN1 0xFE  ///< Inter register enable 1

#define ST_CMD_DELAY   0x80

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

// Initialization commands for GC9A01A 240x240 1.28" IPS
// taken from Adafruit
static const uint8_t PROGMEM init_240x240[] = {
  GC9A01A_INREGEN2, 0,
  0xEB, 1, 0x14, // ?
  GC9A01A_INREGEN1, 0,
  GC9A01A_INREGEN2, 0,
  0xEB, 1, 0x14, // ?
  0x84, 1, 0x40, // ?
  0x85, 1, 0xFF, // ?
  0x86, 1, 0xFF, // ?
  0x87, 1, 0xFF, // ?
  0x88, 1, 0x0A, // ?
  0x89, 1, 0x21, // ?
  0x8A, 1, 0x00, // ?
  0x8B, 1, 0x80, // ?
  0x8C, 1, 0x01, // ?
  0x8D, 1, 0x01, // ?
  0x8E, 1, 0xFF, // ?
  0x8F, 1, 0xFF, // ?
  0xB6, 2, 0x00, 0x00, // ?
  GC9A01A_MADCTL, 1, MADCTL_MX | MADCTL_BGR,
  GC9A01A_COLMOD, 1, 0x05,
  0x90, 4, 0x08, 0x08, 0x08, 0x08, // ?
  0xBD, 1, 0x06, // ?
  0xBC, 1, 0x00, // ?
  0xFF, 3, 0x60, 0x01, 0x04, // ?
  GC9A01A1_POWER2, 1, 0x13,
  GC9A01A1_POWER3, 1, 0x13,
  GC9A01A1_POWER4, 1, 0x22,
  0xBE, 1, 0x11, // ?
  0xE1, 2, 0x10, 0x0E, // ?
  0xDF, 3, 0x21, 0x0c, 0x02, // ?
  GC9A01A_GAMMA1, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
  GC9A01A_GAMMA2, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
  GC9A01A_GAMMA3, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
  GC9A01A_GAMMA4, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
  0xED, 2, 0x1B, 0x0B, // ?
  0xAE, 1, 0x77, // ?
  0xCD, 1, 0x63, // ?
  // Unsure what this line (from manufacturer's boilerplate code) is
  // meant to do, but users reported issues, seems to work OK without:
  //0x70, 9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03, // ?
  GC9A01A_FRAMERATE, 1, 0x34,
  0x62, 12, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, // ?
            0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
  0x63, 12, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, // ?
            0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
  0x64, 7, 0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07, // ?
  0x66, 10, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00, // ?
  0x67, 10, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98, // ?
  0x74, 7, 0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00, // ?
  0x98, 2, 0x3e, 0x07, // ?
  GC9A01A_VSCRSADD, 2, 0,0,
  GC9A01A_TEON, 0,
  GC9A01A_INVON, 0,
  GC9A01A_SLPOUT, 0x80, // Exit sleep
  GC9A01A_DISPON, 0x80, // Display on
  0x00                  // End of list
};
// -----------------------------------------
#if FASTLINE_CLIP==2
// clip to circle precalculated edge values
const uint8_t cornerPGM[120] PROGMEM = {
104,98,93,89,85,82,79,76,74,72,
69,67,65,63,61,60,58,56,55,53,
52,50,49,48,46,45,44,42,41,40,
39,38,37,36,35,34,33,32,31,30,
29,28,27,27,26,25,24,24,23,22,
21,21,20,19,19,18,17,17,16,16,
15,14,14,13,13,12,12,11,11,10,
10,10,9,9,8,8,7,7,7,6,
6,6,5,5,5,4,4,4,4,3,
3,3,3,2,2,2,2,2,1,1,
1,1,1,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0 };
#endif
// -----------------------------------------

#ifdef COMPATIBILITY_MODE
static SPISettings spiSettings;
#define SPI_START  SPI.beginTransaction(spiSettings)
#define SPI_END    SPI.endTransaction()
#else
#define SPI_START
#define SPI_END
#endif

// macros for fast DC and CS state changes
#ifdef COMPATIBILITY_MODE
#define DC_DATA     digitalWrite(dcPin, HIGH)
#define DC_COMMAND  digitalWrite(dcPin, LOW)
#define CS_IDLE     digitalWrite(csPin, HIGH)
#define CS_ACTIVE   digitalWrite(csPin, LOW)
#else
#define DC_DATA    *dcPort |= dcMask
#define DC_COMMAND *dcPort &= ~dcMask
#define CS_IDLE    *csPort |= csMask
#define CS_ACTIVE  *csPort &= ~csMask
#endif

// if CS always connected to the ground then don't do anything for better performance
#ifdef CS_ALWAYS_LOW
#define CS_IDLE
#define CS_ACTIVE
#endif

// ----------------------------------------------------------
// speed test results:
// in AVR best performance mode -> about 6.9 Mbps
// in compatibility mode (SPI.transfer(c)) -> about 4 Mbps
inline void GC9A01A_AVR::writeSPI(uint8_t c) 
{
#ifdef COMPATIBILITY_MODE
    SPI.transfer(c);
#else
    SPDR = c;
    /*
    asm volatile("nop"); // 8 NOPs seem to be enough for 16MHz AVR @ DIV2 to avoid using while loop
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    */
    asm volatile("rjmp .+0\n");
    asm volatile("rjmp .+0\n");
    asm volatile("rjmp .+0\n");
    asm volatile("rjmp .+0\n");
    //while(!(SPSR & _BV(SPIF))) ;
#endif
}

// ----------------------------------------------------------
// fast method to send multiple 16-bit values via SPI
inline void GC9A01A_AVR::writeMulti(uint16_t color, uint16_t num)
{
#ifdef COMPATIBILITY_MODE
  while(num--) { SPI.transfer(color>>8);  SPI.transfer(color); }
#else
  asm volatile
  (
  "next:\n"
    "out %[spdr],%[hi]\n"
    "rjmp .+0\n"  // wait 8*2+1 = 17 cycles
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "nop\n"
    "out %[spdr],%[lo]\n"
    "rjmp .+0\n"  // wait 6*2+1 = 13 cycles + sbiw + brne
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "nop\n"
    "sbiw %[num],1\n"
    "brne next\n"
    : [num] "+w" (num)
    : [spdr] "I" (_SFR_IO_ADDR(SPDR)), [lo] "r" ((uint8_t)color), [hi] "r" ((uint8_t)(color>>8))
  );
#endif
} 
// ----------------------------------------------------------
// fast method to send multiple 16-bit values from RAM via SPI
inline void GC9A01A_AVR::copyMulti(uint8_t *img, uint16_t num)
{
#ifdef COMPATIBILITY_MODE
  while(num--) { SPI.transfer(*(img+1)); SPI.transfer(*(img+0)); img+=2; }
#else
  uint8_t lo,hi;
  asm volatile
  (
  "nextCopy:\n"
    "ld  %[hi],%a[img]+\n"
    "ld  %[lo],%a[img]+\n"
    "out %[spdr],%[lo]\n"
    "rjmp .+0\n"  // wait 8*2+1 = 17 cycles
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "nop\n"
    "out %[spdr],%[hi]\n"
    "rjmp .+0\n"  // wait 4*2+1 = 9 cycles + sbiw + brne + ld*2
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "nop\n"
    "sbiw %[num],1\n"
    "brne nextCopy\n"
    : [num] "+w" (num)
    : [spdr] "I" (_SFR_IO_ADDR(SPDR)), [img] "e" (img), [lo] "r" (lo), [hi] "r" (hi)
  );
#endif
} 
// ----------------------------------------------------------
void GC9A01A_AVR::writeCmd(uint8_t c) 
{
  DC_COMMAND;
  SPI_START;
  CS_ACTIVE;

  writeSPI(c);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void GC9A01A_AVR::writeData(uint8_t d8) 
{
  DC_DATA;
  SPI_START;
  CS_ACTIVE;
    
  writeSPI(d8);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void GC9A01A_AVR::writeData16(uint16_t d16) 
{
  DC_DATA;
  SPI_START;
  CS_ACTIVE;
    
  writeMulti(d16,1);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
GC9A01A_AVR::GC9A01A_AVR(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(GC9A01A_TFTWIDTH, GC9A01A_TFTHEIGHT) 
{
  csPin = cs;
  dcPin = dc;
  rstPin = rst;
}

// ----------------------------------------------------------
void GC9A01A_AVR::init(uint16_t width, uint16_t height) 
{
  Serial.println(F("GC9A01A_AVR lib init"));
  pinMode(dcPin, OUTPUT);
#ifndef CS_ALWAYS_LOW
	pinMode(csPin, OUTPUT);
#endif

#ifndef COMPATIBILITY_MODE
  dcPort = portOutputRegister(digitalPinToPort(dcPin));
  dcMask = digitalPinToBitMask(dcPin);
#ifndef CS_ALWAYS_LOW
  Serial.println(F("CS PIN MODE"));
	csPort = portOutputRegister(digitalPinToPort(csPin));
	csMask = digitalPinToBitMask(csPin);
#endif
#endif

  SPI.begin();
#ifdef COMPATIBILITY_MODE
  Serial.println(F("COMPATIBILITY_MODE"));
  spiSettings = SPISettings(8000000, MSBFIRST, SPI_MODE0);  // 8000000 gives max speed on AVR 16MHz
#else
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
#endif

  _width  = width;
  _height = height;

  if(csPin>=0) digitalWrite(csPin, LOW);

  if(rstPin>=0) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(100);
    digitalWrite(rstPin, LOW);
    delay(100);
    digitalWrite(rstPin, HIGH);
    delay(200);
  } else {
    writeCmd(GC9A01A_SWRESET);
    delay(150);
  }

  displayInit(init_240x240);
  setRotation(2);
}

// ----------------------------------------------------------
void GC9A01A_AVR::displayInit(const uint8_t *addr) 
{
  uint8_t cmd, x, numArgs;
  while ((cmd = pgm_read_byte(addr++)) > 0) {
    writeCmd(cmd);
    x = pgm_read_byte(addr++);
    numArgs = x & 0x7F;
    while(numArgs--) writeData(pgm_read_byte(addr++));
    if (x & 0x80) delay(150);
  }
}

// ----------------------------------------------------------
void GC9A01A_AVR::setRotation(uint8_t m) 
{
  rotation = m & 3;
  switch (rotation) {
    case 0:
    m = (MADCTL_MX | MADCTL_BGR);
    _width = GC9A01A_TFTWIDTH;
    _height = GC9A01A_TFTHEIGHT;
    break;
  case 1:
    m = (MADCTL_MV | MADCTL_BGR);
    _width = GC9A01A_TFTHEIGHT;
    _height = GC9A01A_TFTWIDTH;
    break;
  case 2:
    m = (MADCTL_MY | MADCTL_BGR);
    _width = GC9A01A_TFTWIDTH;
    _height = GC9A01A_TFTHEIGHT;
    break;
  case 3:
    m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    _width = GC9A01A_TFTHEIGHT;
    _height = GC9A01A_TFTWIDTH;
    break;
  }
  writeCmd(GC9A01A_MADCTL);
  writeData(m);
}

// ----------------------------------------------------------
void GC9A01A_AVR::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
  SPI_START;
  CS_ACTIVE;
  
  DC_COMMAND; writeSPI(GC9A01A_CASET);
  DC_DATA;
  writeSPI(xs >> 8); writeSPI(xs);
  writeSPI(xe >> 8); writeSPI(xe);

  DC_COMMAND; writeSPI(GC9A01A_RASET);
  DC_DATA;
  writeSPI(ys >> 8); writeSPI(ys);
  writeSPI(ye >> 8); writeSPI(ye);

  DC_COMMAND; writeSPI(GC9A01A_RAMWR);

  DC_DATA;
  // no CS_IDLE + SPI_END, DC_DATA to save memory
}

// ----------------------------------------------------------
void GC9A01A_AVR::pushColor(uint16_t color) 
{
  SPI_START;
  //DC_DATA;
  CS_ACTIVE;

  writeSPI(color>>8); writeSPI(color);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void GC9A01A_AVR::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
  if(x<0 ||x>=_width || y<0 || y>=_height) return;
  setAddrWindow(x,y,x,y);

  writeSPI(color>>8); writeSPI(color);
  /*
  asm volatile
  (
    "out %[spdr],%[hi]\n"
    "rjmp .+0\n"  // wait 8*2+1 = 17 cycles
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "rjmp .+0\n"
    "nop\n"
    "out %[spdr],%[lo]\n"
    //"rjmp .+0\n"  // wait 6*2+1 = 13 cycles + sbiw + brne
    //"rjmp .+0\n"
    //"rjmp .+0\n"
    //"rjmp .+0\n"
    //"nop\n"
    :
    : [spdr] "I" (_SFR_IO_ADDR(SPDR)), [lo] "r" ((uint8_t)color), [hi] "r" ((uint8_t)(color>>8))
  );
  */
  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// full clipping
void GC9A01A_AVR::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
#if FASTLINE_CLIP==2 // clip to circle, 180 bytes more
  if(x<0 || x>=240) return;
  int y0=pgm_read_byte(&cornerPGM[x<120 ? x : 239-x]); // PGM
  int y1=239-y0;
  if(y>y1) return;
  if(y<y0) { h+=y-y0; y=y0; }
  if(h<=0) return;
  if(y+h>y1+1) h=y1-y+1;
#elif FASTLINE_CLIP==1 // cut corners only, 56 bytes more
  int y0=0,y1,e=70;
  if(x<e) y0=e-x; else if(x>239-e) y0=x-239+e;
  y1=239-y0;
  if(y>y1) return;
  if(y<y0) { h+=y-y0; y=y0; }
  if(h<=0) return;
  if(y+h>y1+1) h=y1+1-y; 
#else // full 240x240 frame
  if(x>=_width || y>=_height || h<=0) return;
  if(y+h>_height) h=_height-y;
  if(y<0) { h+=y; y=0; }
  if(h<=0) return;
#endif
  setAddrWindow(x, y, x, y+h-1);

  writeMulti(color,h);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// full clipping
void GC9A01A_AVR::drawFastHLine(int16_t x, int16_t y, int16_t w,  uint16_t color) 
{
#if FASTLINE_CLIP==2 // clip to circle, 180 bytes of code more
  if(y<0 || y>=240) return;
  int x0=pgm_read_byte(&cornerPGM[y<120 ? y : 239-y]); // PGM
  int x1=239-x0;
  if(x>x1) return;
  if(x<x0) { w+=x-x0; x=x0; }
  if(w<=0) return;
  if(x+w>x1+1) w=x1-x+1;
#elif FASTLINE_CLIP==1 // cut corners only, 56 bytes of code more
  int x0=0,x1,e=70;
  if(y<e) x0=e-y; else if(y>239-e) x0=y-239+e;
  x1=239-x0;
  if(x>x1) return;
  if(x<x0) { w+=x-x0; x=x0; }
  if(w<=0) return;
  if(x+w>x1+1) w=x1+1-x;
#else // full 240x240 frame
  if(x>=_width || y>=_height || w<=0) return;
  if(x+w>_width)  w=_width-x;
  if(x<0) { w+=x; x=0; }
  if(w<=0) return;
#endif
  setAddrWindow(x, y, x+w-1, y);

  writeMulti(color,w);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// full clipping
void GC9A01A_AVR::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  if(x+w>_width)  w=_width -x;
  if(y+h>_height) h=_height-y;
  if(x<0) { w+=x; x=0; }
  if(w<=0) return;
  if(y<0) { h+=y; y=0; }
  if(h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  writeMulti(color,w*h);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void GC9A01A_AVR::fillScreen(uint16_t color) 
{
  fillRect(0, 0,  _width, _height, color);
}

// ----------------------------------------------------------
// draws image from RAM, only basic clipping
void GC9A01A_AVR::drawImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *img16) 
{
  // all clipping should be on the application side
  //if(w<=0 || h<=0) return;  // left for compatibility
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  //if(x+w-1>=_width)  w=_width -x;
  //if(y+h-1>=_height) h=_height-y;
  setAddrWindow(x, y, x+w-1, y+h-1);

  copyMulti((uint8_t *)img16, w*h);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// draws image from flash (PROGMEM)
void GC9A01A_AVR::drawImageF(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *img16) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  uint32_t num = (uint32_t)w*h;
  uint16_t num16 = num>>3;
  uint8_t *img = (uint8_t *)img16;
  while(num16--) {
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
    writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2;
  }
  uint8_t num8 = num & 0x7;
  while(num8--) { writeSPI(pgm_read_byte(img+1)); writeSPI(pgm_read_byte(img+0)); img+=2; }

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t GC9A01A_AVR::Color565(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// ----------------------------------------------------------
void GC9A01A_AVR::invertDisplay(boolean mode) 
{
  writeCmd(mode ? GC9A01A_INVON : GC9A01A_INVOFF);
}

// ----------------------------------------------------------
// doesn't work?
void GC9A01A_AVR::partialDisplay(boolean mode) 
{
  writeCmd(mode ? GC9A01A_PTLON : GC9A01A_NORON);
}

// ----------------------------------------------------------
void GC9A01A_AVR::sleepDisplay(boolean mode) 
{
  writeCmd(mode ? GC9A01A_SLPIN : GC9A01A_SLPOUT);
  delay(5);
}

// ----------------------------------------------------------
void GC9A01A_AVR::enableDisplay(boolean mode) 
{
  writeCmd(mode ? GC9A01A_DISPON : GC9A01A_DISPOFF);
}

// ----------------------------------------------------------
void GC9A01A_AVR::idleDisplay(boolean mode) 
{
  writeCmd(mode ? GC9A01A_IDLEON : GC9A01A_IDLEOFF);
}

// ----------------------------------------------------------
void GC9A01A_AVR::resetDisplay() 
{
  writeCmd(GC9A01A_SWRESET);
  delay(5);
}

// ----------------------------------------------------------
void GC9A01A_AVR::setScrollArea(uint16_t tfa, uint16_t bfa) 
{
  uint16_t vsa = 240-tfa-bfa;
  writeCmd(GC9A01A_VSCRDEF);
  writeData16(tfa);
  writeData16(vsa);
  writeData16(bfa);
}

// ----------------------------------------------------------
void GC9A01A_AVR::setScroll(uint16_t vsp) 
{
  writeCmd(GC9A01A_VSCRSADD);
  writeData16(vsp);
}

// ----------------------------------------------------------
// doesn't work?
void GC9A01A_AVR::setPartArea(uint16_t sr, uint16_t er) 
{
  writeCmd(GC9A01A_PTLAR);
  writeData16(sr);
  writeData16(er);
}

// ----------------------------------------------------------
// doesn't work
void GC9A01A_AVR::setBrightness(uint8_t br) 
{
  //writeCmd(ST7789_WRCACE);
  //writeData(0xb1);  // 80,90,b0, or 00,01,02,03
  //writeCmd(ST7789_WRCABCMB);
  //writeData(120);

  //BCTRL=0x20, dd=0x08, bl=0x04
  int val = 0x04;
  //writeCmd(ST7789_WRCTRLD);
  writeData(val);
  //writeCmd(ST7789_WRDISBV);
  writeData(br);
}

// ----------------------------------------------------------
// 0 - off
// 1 - idle
// 2 - normal
// 4 - display off
void GC9A01A_AVR::powerSave(uint8_t mode) 
{
  /*
  if(mode==0) {
    writeCmd(ST7789_POWSAVE);
    writeData(0xec|3);
    writeCmd(ST7789_DLPOFFSAVE);
    writeData(0xff);
    return;
  }
  int is = (mode&1) ? 0 : 1;
  int ns = (mode&2) ? 0 : 2;
  writeCmd(ST7789_POWSAVE);
  writeData(0xec|ns|is);
  if(mode&4) {
    writeCmd(ST7789_DLPOFFSAVE);
    writeData(0xfe);
  }*/
}

// ------------------------------------------------
// Input a value 0 to 511 (85*6) to get a color value.
// The colours are a transition R - Y - G - C - B - M - R.
void GC9A01A_AVR::rgbWheel(int idx, uint8_t *_r, uint8_t *_g, uint8_t *_b)
{
  idx &= 0x1ff;
  if(idx < 85) { // R->Y  
    *_r = 255; *_g = idx * 3; *_b = 0;
    return;
  } else if(idx < 85*2) { // Y->G
    idx -= 85*1;
    *_r = 255 - idx * 3; *_g = 255; *_b = 0;
    return;
  } else if(idx < 85*3) { // G->C
    idx -= 85*2;
    *_r = 0; *_g = 255; *_b = idx * 3;
    return;  
  } else if(idx < 85*4) { // C->B
    idx -= 85*3;
    *_r = 0; *_g = 255 - idx * 3; *_b = 255;
    return;    
  } else if(idx < 85*5) { // B->M
    idx -= 85*4;
    *_r = idx * 3; *_g = 0; *_b = 255;
    return;    
  } else { // M->R
    idx -= 85*5;
    if(idx>85) idx=85;
    *_r = 255; *_g = 0; *_b = 255 - idx * 3;
   return;
  }
} 

uint16_t GC9A01A_AVR::rgbWheel(int idx)
{
  uint8_t r,g,b;
  rgbWheel(idx, &r,&g,&b);
  return RGBto565(r,g,b);
}

// ------------------------------------------------