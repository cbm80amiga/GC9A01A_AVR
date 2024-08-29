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

//-----------------------------------------------------------------------------

void fillTri(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) 
{
  int y;
  for(y=0;y<SCR_HT;y++) { raster[2*y+0] = SCR_WD+1; raster[2*y+1] = 0; }

  rasterize( x0, y0, x1, y1);
  rasterize( x1, y1, x2, y2);
  rasterize( x2, y2, x0, y0);

  for(y=0;y<SCR_HT;y++)
    if(raster[2*y+1]>raster[2*y+0]) lcd.drawFastHLine(raster[2*y+0],y,raster[2*y+1]-raster[2*y+0]+1,c);
}

//-----------------------------------------------------------------------------

void fillQuad( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) 
{
  int y;
  for(y=0;y<SCR_HT;y++) { raster[2*y+0] = SCR_WD+1; raster[2*y+1] = 0; }

  rasterize( x0, y0, x1, y1);
  rasterize( x1, y1, x2, y2);
  rasterize( x2, y2, x3, y3);
  rasterize( x3, y3, x0, y0);

  for(y=0;y<SCR_HT;y++)
    if(raster[2*y+1]>raster[2*y+0]) lcd.drawFastHLine(raster[2*y+0],y,raster[2*y+1]-raster[2*y+0]+1,c);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 141ms vs 153ms lcd.fillCircle r=10 x 100
void fillCircle(int x0, int y0, int r, uint16_t color)
{
  int  x  = 0;
  int  dx = 1;
  int  dy = r+r;
  int  p  = -(r>>1);

  lcd.drawFastHLine(x0 - r, y0, dy+1, color);

  while(x<r) {
    if(p>=0) {
      lcd.drawFastHLine(x0 - x, y0 + r, dx, color);
      lcd.drawFastHLine(x0 - x, y0 - r, dx, color);
      dy-=2;
      p-=dy;
      r--;
    }

    dx+=2;
    p+=dx;
    x++;
    lcd.drawFastHLine(x0 - r, y0 + x, dy+1, color);
    lcd.drawFastHLine(x0 - r, y0 - x, dy+1, color);
  }
}
//-----------------------------------------------------------------------------

