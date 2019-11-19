/*  Canvas implements drawing on an n bit frame buffer where n= 1,2,4 or 8.
 *  The drawing primitives are taken from Adafruit Gfx (see legal notice at end of file). 
 *  The character drawing primitives use Adafruit proportional fonts, so the Gfx library must
 *  be installed to compile Canvas
 *
 *  j.n. magee 19-nov-2019
 *
 */

#include "canvas.h"
#include "font.h"

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif


static unsigned int pow2(uint8_t v){
    int res = 1;
    for (uint8_t i=0; i<v; ++i) res *= 2;
    return res;
} 
     
Canvas::Canvas( int width,  int height, PixelBits bitsperpixel, const uint16_t *colormap){
    _height = height; 
    _width = width; 
    _bits = bitsperpixel;
    _pixel_mask = pow2(_bits) - 1;
    _pixperbyte = 8/_bits;
    _map = colormap; 
    _canvas = new uint8_t[(width * height * _bits)/8];
    clear();
}

void Canvas::drawPixel(int x, int y) {
  if (x >= _width || y >= _height) return;
  int addr = (y*_width +x);
  uint8_t pos;
  if (_bits == 1) {
      addr /= 8; pos = x & 7;
      _canvas[addr] = (_canvas[addr] & ~(0x01<< pos)) | (_color << pos);
      return;
  }
  if (_bits == 2) {
      addr /= 4; pos = (x & 3) * 2;
      _canvas[addr] = (_canvas[addr] & ~(0x03<< pos)) | (_color << pos);
      return;
  }
  if (_bits == 4) {
      addr /= 2; pos = (x & 1) * 4;
      _canvas[addr] = (_canvas[addr] & ~(0x0F<< pos)) | (_color << pos);
      return;
  }
  if (_bits == 8) _canvas[addr] = _color;
}


uint16_t Canvas::getScreenColor(int x, int y) {
  int addr = (y*_width +x);
  uint8_t pos;
  if (_bits == 1) {
      addr /= 8; pos = x & 7;
      return _map[(_canvas[addr] & (0x01<< pos)) >> pos];
  }
  if (_bits == 2) {
      addr /= 4; pos = (x & 3) * 2;
      return _map[(_canvas[addr] & (0x03<< pos)) >> pos];
  }
  if (_bits == 4) {
      addr /= 2; pos = (x & 1) * 4;
      return _map[(_canvas[addr] & (0x0F<< pos)) >> pos];
  }
  if (_bits == 8) return _map[_canvas[addr]];
}

void Canvas::drawHLine(int x, int y, int w){
    for(int i=0; i<w; ++i) drawPixel(x+i,y);
}

void Canvas::drawVLine(int x, int y, int h){
    for (int i=0; i<h; ++i) drawPixel(x,y+i);
}

static void _swap_int(int a, int b) {int t = b; a = b; b = t;}

void Canvas::drawLine(int x1, int y1, int x2, int y2) {
    int tmp, x,y, dx, dy, err, ystep;
    int swapxy = 0;
  
    if ( x1 > x2 ) dx = x1-x2; else dx = x2-x1;
    if ( y1 > y2 ) dy = y1-y2; else dy = y2-y1;
  
    if ( dy > dx ) {
      swapxy = 1;
      tmp = dx; dx =dy; dy = tmp;
      tmp = x1; x1 =y1; y1 = tmp;
      tmp = x2; x2 =y2; y2 = tmp;
    } 
    if ( x1 > x2 ) {
      tmp = x1; x1 =x2; x2 = tmp;
      tmp = y1; y1 =y2; y2 = tmp;
    }
    err = dx >> 1;
    if ( y2 > y1 ) ystep = 1; else ystep = -1;
    y = y1;
  
    for( x = x1; x <= x2; x++ ) {
      if ( swapxy == 0 ) 
        drawPixel( x, y); 
      else 
        drawPixel(y, x); 
      err -= dy;
      if ( err < 0 ) {
        y += ystep;
        err += dx;
      }
    }
}

void Canvas::drawBox(int x, int y, int w, int h) {
    drawHLine(x,y,w);
    drawHLine(x,y+h-1,w);
    drawVLine(x,y,h);
    drawVLine(x+w-1,y,h);
}


void Canvas::drawFilledBox(int x, int y, int w, int h) {
    for (int i=y; i<y+h; i++) {
        drawHLine(x, i, w);
    }
}

void Canvas::drawCircle(int x0, int y0, int r) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    drawPixel(x0  , y0+r);
    drawPixel(x0  , y0-r);
    drawPixel(x0+r, y0  );
    drawPixel(x0-r, y0  );

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y);
        drawPixel(x0 - x, y0 + y);
        drawPixel(x0 + x, y0 - y);
        drawPixel(x0 - x, y0 - y);
        drawPixel(x0 + y, y0 + x);
        drawPixel(x0 - y, y0 + x);
        drawPixel(x0 + y, y0 - x);
        drawPixel(x0 - y, y0 - x);
    }
}

void Canvas::drawFilledCircle(int x0, int y0, int r) {
    int x = r;
    int y = 0;
    int xChange = 1 - (r << 1);
    int yChange = 0;
    int radiusError = 0;

    while (x >= y)
    {
        for (int i = x0 - x; i <= x0 + x; i++)
        {
            drawPixel(i, y0 + y);
            drawPixel(i, y0 - y);
        }
        for (int i = x0 - y; i <= x0 + y; i++)
        {
            drawPixel(i, y0 + x);
            drawPixel(i, y0 - x);
        }

        y++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0)
        {
            x--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}

size_t Canvas::drawChar(int x, int y,  unsigned char c) {
        if (!_font) {
            for (uint8_t i = 0; i < FONT_WIDTH; i++) {
                uint8_t data = pgm_read_byte_near(&font_bitmap[c - FONT_START ][i]);
                for ( uint8_t j = 0; j<FONT_HEIGHT; j++) 
                     if (data & (1<<j)) drawPixel(x+i,y+j);
            }
            return FONT_WIDTH + 1;
        }
        uint8_t first = (uint8_t)pgm_read_byte(&_font->first);
        uint8_t last = (uint8_t)pgm_read_byte(&_font->last);
        if (c<first || c>last) return 0;
        c -=first;
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&_font->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&_font->bitmap);
        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        for(yy=0; yy<h; yy++) {
            for(xx=0; xx<w; xx++) {
                if(!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80) {
                    drawPixel(x+xo+xx, y+yo+yy);
                } 
                bits <<= 1;
            }
        }
        return pgm_read_byte(&glyph->xAdvance);
}

int Canvas::charWidth(char c) {
     if (!_font) return FONT_WIDTH + 1; 
     c -= (uint8_t)pgm_read_byte(&_font->first);
     GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&_font->glyph))[c]);
     return pgm_read_byte(&glyph->xAdvance);
}

int Canvas::strWidth(const char * s) {
   int w =0;
   for(int i = 0; s[i]!=0; ++i) 
      w += charWidth(s[i]);
   return w;
}

size_t Canvas::write(unsigned char c) {
     size_t s = drawChar(_x, _y, c);
     _x += s;
     return s;
}
    
//assumes that horizontal lines are padded into an integral number of bytes
void Canvas::drawXBM(int x, int y, int w, int h, const uint8_t *bitmap){
    uint8_t data;
    uint8_t bytewidth = (w%8 == 0) ? w/8 : w/8+1;
    for (int j = 0; j<h; ++j){
        for (int i= 0; i<w; ++i) {
            uint8_t bitno = i%8;
            if (bitno == 0) data = pgm_read_byte_near(&bitmap[j*bytewidth+i/8]);
            if (data & (1<<bitno)) drawPixel(x+i,y+j);
        }
    }
}

void Canvas::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {
    drawLine(x0, y0, x1, y1);
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x0, y0);
}

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
            
void Canvas::drawFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        drawHLine(a, y0, b-a+1);
        return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        drawHLine(a, y, b-a+1);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        drawHLine(a, y, b-a+1);
    }
}

/*
Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
