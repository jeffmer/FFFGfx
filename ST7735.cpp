/*
* Driver for ST7735 Color TFT. 
* The low level routines and initialisation code are from
* David Johnson-Davies's Tiny TFT Graphi_CS Library - see notice at end of file. 
* 
* j.n.magee 19-nov-2019
*
*/

#include "ST7735.h"
#include <SPI.h>
#include "user_settings.h"

static SPISettings settings(16000000, MSBFIRST, SPI_MODE0);

//st7735 commands
static uint8_t const CASET = 0x2A;  // Define column address
static uint8_t const RASET = 0x2B; // Define row address
static uint8_t const RAMWR = 0x2C; // Write to display RAM
static uint8_t const DISPON  = 0x29; // display on
static uint8_t const DISPOFF = 0x28; // display off


#ifdef USE_DMA_FOR_M0
/****   DMA code for M0 only ****/
// NB uChip uses SERCOM1 for SPI,  Adafruit Feather uses SERCOM4 - adjust references below.
#include <Adafruit_ZeroDMA.h>
#include "utility/dma.h"

static Adafruit_ZeroDMA theDMA;
static ZeroDMAstatus    stat; // DMA status codes returned by some functions
static DmacDescriptor *desc; // DMA descriptor address (so we can change contents)
static uint16_t dbuf[2][_YSIZE];

volatile bool transfer_flag = true; 

// Callback for end-of-DMA-transfer
static void dma_callback(Adafruit_ZeroDMA *dma) {
  transfer_flag = true;
}

static void dma_setup(){
  theDMA.setTrigger(SERCOM1_DMAC_ID_TX); //M0
  theDMA.setAction(DMA_TRIGGER_ACTON_BEAT);
  stat = theDMA.allocate();
  desc = theDMA.addDescriptor(
    dbuf[0],                    // move data from here
    (void *)(&USE_SERCOM->SPI.DATA.reg), // to here (M0)
    _YSIZE,                      // this many...
    DMA_BEAT_SIZE_BYTE,               // bytes/hword/words
    true,                             // increment source addr?
    false);                           // increment dest addr?
  theDMA.setCallback(dma_callback);
}

static void do_dma(int phase, int amount){
    while(theDMA.isActive()) ;
    SPI.endTransaction();
    theDMA.changeDescriptor(desc, 
      dbuf[phase], 
      (void *)(&USE_SERCOM->SPI.DATA.reg), 
      amount);       
    SPI.beginTransaction(settings);  
    transfer_flag = false;            
    stat = theDMA.startJob();
}

#endif

// Send a command to the display
static void Command (uint8_t c) {
    digitalWrite(_DC,LOW);                       
    SPI.transfer(c);
    digitalWrite(_DC,HIGH);                          
}

// Send a command followed by four data bytes
static void Command4 (uint8_t c, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
    digitalWrite(_DC,LOW);                          
    SPI.transfer(c);
    digitalWrite(_DC,HIGH);                         
    SPI.transfer(d1); 
    SPI.transfer(d2); 
    SPI.transfer(d3); 
    SPI.transfer(d4);
}
  
void ST7735::init() {
  pinMode(_CS, OUTPUT);
  pinMode(_DC, OUTPUT);
  digitalWrite(_RESET, HIGH);
  pinMode(_RESET, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(settings); 
  digitalWrite(_CS,LOW);                            
  Command(0x01);                           // Software _RESET
  delay(150);                              // delay 150 ms
  Command(0x11);                           // Out of sleep mode
  delay(500);                              // delay 500 ms
  Command(0x3A); SPI.transfer(0x05);       // Set color mode - 16-bit color
  Command(0x20+_INVERT);                    // Invert
  Command(0x36); SPI.transfer(_ROTATE<<5);  // Set orientation
  digitalWrite(_CS,HIGH); 
#ifdef USE_DMA_FOR_M0
  dma_setup();
#endif  
  clear();
  on();               
}

void ST7735::on() {
  digitalWrite(_CS, LOW);                            
  Command(DISPON);
  delay(100);
  digitalWrite(_CS, HIGH);                            
}

void ST7735::off() {
  digitalWrite(_CS, LOW);                            
  Command(DISPOFF);
  delay(100);
  digitalWrite(_CS, HIGH);                            
}

void ST7735::clear() {
  digitalWrite(_CS, LOW);                            
  Command4(CASET, _YOFF>>8, _YOFF, 0, _YOFF + _YSIZE - 1);
  Command4(RASET, _XOFF>>8, _XOFF, 0, _XOFF + _XSIZE - 1);
  Command(RAMWR);
  for (int i=0; i<_XSIZE; i++) {
    for (int j=0; j<_YSIZE; j++) {
      SPI.transfer16(0);
    }
  }
  digitalWrite(_CS, HIGH);                            
}

void ST7735::paint (int x, int y, Canvas pane) {
  int w = pane.getWidth(); 
  int h = pane.getHeight();
  digitalWrite(_CS, LOW);                            
  Command4(CASET, 0, y+_YOFF, 0, y+_YOFF+h-1);
  Command4(RASET, 0, x+_XOFF, 0, x+_XOFF+w-1);
  Command(RAMWR);
  for (int i=0; i<w; i++) {
    for (int j=0; j<h; j++) {
      uint16_t p = pane.getScreenColor(i,j);
#ifdef USE_DMA_FOR_M0
      // fill buffer
      dbuf[i & 1][j] = p>>8 | p<< 8;  
#else
      SPI.transfer(p>>8);
      SPI.transfer(p); 
#endif
    }
#ifdef USE_DMA_FOR_M0
    do_dma(i & 1, h*2);
#endif
  }
  digitalWrite(_CS, HIGH);
  pane.clear();
}

/* Tiny TFT Graphics Library - see http://www.technoblogy.com/show?L6I

   David Johnson-Davies - www.technoblogy.com - 13th June 2019
   ATtiny85 @ 8 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/
