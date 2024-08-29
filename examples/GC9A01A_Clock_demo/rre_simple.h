#include "rre_digits_arialb60v.h"
#include "rre_digits_arial60v.h"

RRE_Font *rFont;

int fg = 0xffff, bg = 0x0000;
uint8_t *rects,*rectsCh;
uint16_t recNum,r,num;
int xf=-1,yf=-1,wf,hf;
int chWdMax = 0;
int chWd=0;
int chHt=0;
int chHtAA=0;
int digitWdAA = 0;
int scrWd=SCR_WD,scrHt=SCR_HT;
int spacingX=1;

// ----------------

uint16_t colline[60]; // max font height=60

// ----------------

// mix color between rgb1 and rgb2
uint16_t mixCol(int r1, int g1, int b1, int r2, int g2, int b2, int tr)
{
  int r = r1+(long)(r2-r1)*tr/255;
  int g = g1+(long)(g2-g1)*tr/255;
  int b = b1+(long)(b2-b1)*tr/255;
  return RGBto565(r,g,b);
}

// ----------------
// gen colors from rgbf to rgbb
void genColors(uint16_t *cols, int rf,int gf,int bf, int rb,int gb,int bb, int maxCol)
{ 
  for(int i=0;i<=maxCol;i++) cols[i]=mixCol(rb,gb,bb, rf,gf,bf, i*255/maxCol);
}

// ----------------

void setFont(struct RRE_Font *f)
{
  rFont = f;
  chWdMax = 0;
  
  int digitWdMax=0;
  Serial.println("RRE Font "+String(rFont->wd)+"x"+String(rFont->ht));
  for(int i=rFont->firstCh; i<=rFont->lastCh; i++) {
    uint16_t recIdx = pgm_read_word(&(rFont->offs[i-rFont->firstCh]));
    uint16_t recNum = pgm_read_word(&(rFont->offs[i-rFont->firstCh+1]))-recIdx;
    int lastIdx = recNum-1+recIdx;
    uint8_t *rects = (uint8_t*)rFont->rects + lastIdx*3;
    int chWd = recNum>0 ? pgm_read_byte(&rects[0])+1 : 0;
    if(chWd>chWdMax) chWdMax=chWd;
    if(isdigit(i) && chWd>digitWdMax) digitWdMax=chWd;
    char txt[50];
    snprintf(txt,50,"char %2d/%2x '%c' %2dx%2d start=%4d recNum=%3d",i,i,i,chWd,rFont->ht,recIdx*3,recNum);
    Serial.println(txt);
  }
  Serial.println("chWdMax = "+String(chWdMax));
  Serial.println("digitWdMax = "+String(digitWdMax));
}

void setDigitWdAA(int w)
{
  digitWdAA=w;
}

// ----------------

void drawRRECharColumn(int x, int y, int column) 
{
  int yb,hb;
  yf=hf=0;
  //optimize if required column is following column
  if(column!=xf) {
    rects = rectsCh;
    num = recNum;
    r=0;
    while(r++<num && (xf=pgm_read_byte(rects+0))<column ) rects+=3;
  }

  while(xf==column && r<=num) {
    yb = yf+hf;
    yf = pgm_read_byte(rects+1);
    hf = pgm_read_byte(rects+2)+1;
    hb = yf-yb;
    lcd.drawFastVLine(x, y+yb, hb, bg);
    //lcd.drawFastVLine(x, y+yf, hf, fg);
    lcd.drawImage(x,y+yf,1,hf,colline+yf); 
    if(r++>=num) break;
    rects+=3;
    xf = pgm_read_byte(rects+0);
  }

  lcd.drawFastVLine(x, y+yf+hf, chHt-yf-hf, bg);
}

// -------------------------

int drawRREChar(int x, int y, char ch)
{
  int i,j, wd,wdL=0,wdR=0;
  chHt=chHtAA=rFont->ht;
  if(ch==' ') {
    wd = digitWdAA>0 ? digitWdAA : rFont->wd/3;
    lcd.fillRect(x,y,wd,chHtAA,bg);
    return wd;
  }

  if(ch<rFont->firstCh || ch>rFont->lastCh) return 0;
  uint16_t recIdx = pgm_read_word(&(rFont->offs[ch-rFont->firstCh]));
  recNum = pgm_read_word(&(rFont->offs[ch-rFont->firstCh+1]))-recIdx;
  int lastIdx = recNum-1+recIdx;
  rects = (uint8_t*)rFont->rects + lastIdx*3;
  chWd = pgm_read_byte(&rects[0])+1;
  rectsCh = rects = (uint8_t*)rFont->rects + recIdx*3;

  wd=chWd;

  if(digitWdAA>0 && digitWdAA>wd && isdigit(ch)) {
    wdL =(digitWdAA-wd)/2;
    wdR+=(digitWdAA-wd-wdL);
  }
  if(x+wd+wdL+wdR>scrWd) wdR = max(scrWd-x-wdL-wd, 0);
  if(x+wd+wdL+wdR>scrWd) wd  = max(scrWd-x-wdL, 0);
  if(x+wd+wdL+wdR>scrWd) wdL = max(scrWd-x, 0);
  lcd.fillRect(x,y,wdL,chHtAA,bg);
  x+=wdL;
  lcd.fillRect(x+wd,y,wdR,chHtAA,bg);
  for(i=0;i<wd;i++) drawRRECharColumn(x+i,y,i);
  return wd+wdR+wdL;
}

//-------------------
/*
int drawStringRRE(int xs, int ys, char *str)
{
  int x=xs,ch,w;
  while(*str) {
    ch = *str++;
    w = drawCharAA(x,ys,ch);
    x+=w+spacingX;
  }
  return x-spacingX;
}
*/
//-------------------


