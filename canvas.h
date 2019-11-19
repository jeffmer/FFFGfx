#ifndef _CANVAS_H
#define _CANVAS_H

#include <arduino.h>
#include <gfxfont.h>

/*
*  for simplicity and efficiency bits per pixel must be 1, 2, 4 or 8
*
*/

enum PixelBits {PIXELBITS1 = 1, PIXELBITS2 = 2, PIXELBITS4 = 4, PIXELBITS8 = 8};

class Canvas : public Print {
  public:
    virtual size_t write(uint8_t);
    
    Canvas(int width, int height, PixelBits bitsperpixel, const uint16_t *colormap);
    
    void setColor(uint8_t color) 
        {_color = color & _pixel_mask;}
    void setXY(int x, int y) 
        { _x = x; _y = y;}
    void setFont(const GFXfont* font) 
        {_font = font;}
    
    void drawPixel(int x, int y);
    void drawHLine(int x, int y, int w);  
    void drawVLine(int x, int y, int h); 
    void drawLine(int x0, int y0, int x1, int y1);
    void drawBox(int x, int y, int w, int h);
    void drawFilledBox(int x, int y, int w, int h);
    void drawCircle(int x0, int y0, int r);
    void drawFilledCircle(int x0, int y0, int r);
    void drawXBM(int x, int y, int w, int h, const uint8_t *bitmap);
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
    void drawFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
    size_t drawChar(int x, int y,  uint8_t c);
    
    int charWidth(char c);
    int strWidth(const char * s);
    
    void clear()    //no need to call explicitly, canvas is cleared after paint
        { memset(_canvas, 0x00, _width*_height/_pixperbyte);}
    
    uint16_t getScreenColor(int x,  int y);
    uint16_t getWidth(void){ return _width;}
    uint16_t getHeight(void){ return _height;}
    
protected:
    uint16_t _height;
    uint16_t _width;
    uint16_t _x;
    uint16_t _y;
    uint8_t _color;
    const GFXfont* _font;
    uint8_t _bits;
    uint8_t _pixel_mask;
    uint8_t _pixperbyte;
    const uint16_t *_map;   
    uint8_t *_canvas;  
};

#endif
