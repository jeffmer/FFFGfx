
#include "fff_TFTSPI.h"
#include <SPI.h>
#include "user_settings.h"

//common commands

static uint8_t const SWRESET = 0x01; // software reset 
static const uint8_t SLPIN  = 0x10;
static const uint8_t SLPOUT = 0x11;
static uint8_t const INVREG = 0X20; // 20 invert off 21 invert on 
static uint8_t const GAMMASET = 0X26; // 20 invert off 21 invert on 
static uint8_t const CASET = 0x2A;  // Define column address
static uint8_t const RASET = 0x2B; // Define row address
static uint8_t const RAMWR = 0x2C; // Write to display RAM
static uint8_t const DISPOFF = 0x28; // display off
static uint8_t const DISPON  = 0x29; // display on
static uint8_t const MADCTL = 0X36; // memory access cintrol
static uint8_t const VSCRSADD = 0x37;
static uint8_t const PIXFMT = 0X3A; // pixel color format 

#ifdef IS_ST7735
static const uint8_t PROGMEM initcmd[] = {
  SWRESET, 0x80,
  SLPOUT  , 0x80,              // Exit Sleep
  MADCTL, 1, (_ROTATE<<5) | 0x08,       // Memory Access Control
  INVREG+_INVERT, 0,
  PIXFMT, 1, 0x05,
  DISPON  , 0x80,              // Display on
  0x00                         // End of list
};
#endif

#ifdef IS_ILI9341
static const uint8_t PROGMEM initcmd[] = {
  SWRESET, 0x80,
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  0xC0, 1, 0x23,             // Power control VRH[5:0]
  0xC1, 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  0xC5, 2, 0x3e, 0x28,       // VCM control
  0xC7, 1, 0x86,             // VCM control2
  MADCTL, 1, (_ROTATE<<5) | 0x08,       // Memory Access Control
  VSCRSADD, 1, 0x00,             // Vertical scroll zero
  PIXFMT, 1, 0x55,
  0xB1, 2, 0x00, 0x18,
  0xB6, 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  GAMMASET , 1, 0x01,             // Gamma curve selected
  0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
        0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
        0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  SLPOUT  , 0x80,                // Exit Sleep
  DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};
#endif

static SPISettings settings(40000000, MSBFIRST, SPI_MODE0);

#ifdef _SPI_WRITEBYTES_
static uint16_t _buf[_YSIZE];
#endif

#ifdef USE_DMA_FOR_M0
/****   DMA code for M0 only ****/
// NB uChip uses SERCOM1 for SPI,  Adafruit Feather uses SERCOM4 - adjust references below.
#include <Adafruit_ZeroDMA.h>
#include "utility/dma.h"

static Adafruit_ZeroDMA theDMA;
static ZeroDMAstatus    stat; // DMA status codes returned by some functions
static DmacDescriptor *desc; // DMA descriptor address (so we can change contents)
static uint16_t dbuf[2][_YSIZE];

static volatile bool transfer_flag = true; 

// Callback for end-of-DMA-transfer
static void dma_callback(Adafruit_ZeroDMA *dma) {
  transfer_flag = true;
}

static void dma_setup(){
  theDMA.setTrigger(USE_TRIGGER); //M0
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

// Send a command followed by n data bytes
static void Commandn(uint8_t c, int n, const uint8_t *d){
    digitalWrite(_DC,LOW);                          
    SPI.transfer(c);
    digitalWrite(_DC,HIGH);                         
    for(int i = 0; i<n; ++i) SPI.transfer(pgm_read_byte(d+i));  
}

static void initCommands(const uint8_t *addr){
    uint8_t cmd, x, n;
    while((cmd = pgm_read_byte(addr++)) > 0) {
        x = pgm_read_byte(addr++);
        n = x & 0x7F;
        Commandn(cmd, n, addr);
        addr += n;
        if(x & 0x80) delay(150);
    }
}

void fff_TFTSPI::init(uint16_t background) {
    pinMode(_CS, OUTPUT);
    pinMode(_DC, OUTPUT);
#ifdef _RESET
    pinMode(_RESET, OUTPUT);
    digitalWrite(_RESET, LOW);
    delay(100);
    digitalWrite(_RESET, HIGH);
    delay(200);
#endif
#ifdef _LED_PIN
    pinMode(_LED_PIN, OUTPUT);
    digitalWrite(_LED_PIN, HIGH);
#endif
    SPI.begin();
    SPI.beginTransaction(settings); 
    digitalWrite(_CS,LOW);                            
    initCommands(initcmd);   
    digitalWrite(_CS,HIGH); 
    SPI.endTransaction();
#ifdef USE_DMA_FOR_M0
    dma_setup();
#endif      
    clear(background);         
}


void fff_TFTSPI::on() {
  digitalWrite(_CS, LOW);                            
  Command(DISPON);
  delay(100);
  digitalWrite(_CS, HIGH);                            
}

void fff_TFTSPI::off() {
  digitalWrite(_CS, LOW);                            
  Command(DISPOFF);
  delay(100);
  digitalWrite(_CS, HIGH);                            
}

void fff_TFTSPI::clear(uint16_t background) {
  SPI.beginTransaction(settings); 
  digitalWrite(_CS, LOW);
  const int maxX = _XOFF + _XSIZE - 1;
  const int maxY = _YOFF + _YSIZE - 1;                         
  Command4(CASET, 0, _YOFF, maxY>>8, maxY);
  Command4(RASET, 0, _XOFF, maxX>>8, maxX);
  Command(RAMWR);
  #ifdef USE_DMA_FOR_M0
  for (int j=0; j<_YSIZE; j++) dbuf[0][j] = background>>8 | background<< 8;
  for (int i=0; i<=_XSIZE; i++) do_dma(0, _YSIZE*2);
  while(theDMA.isActive()) ; // finish last dma
  #elif defined _SPI_WRITEBYTES_
  for (int j=0; j<_YSIZE; j++) _buf[j] = background>>8 | background<< 8;
  for (int i=0; i<=_XSIZE; i++) SPI.writeBytes((byte *)&_buf, _YSIZE*2);
  #else 
  for (int i=0; i<_XSIZE; i++) {
    for (int j=0; j<_YSIZE; j++) {
      SPI.transfer16(background>>8 | background<< 8);
    }
  } 
  #endif
  digitalWrite(_CS, HIGH);
  SPI.endTransaction();                            
}

void fff_TFTSPI::paint (int x, int y, Canvas *pane) {
  x += _XOFF; 
  y += _YOFF;
  int w = pane->getWidth(); 
  int h = pane->getHeight();
  int x1 = x + w - 1; 
  int y1 = y + h - 1;
  SPI.beginTransaction(settings); 
  digitalWrite(_CS, LOW);                            
  Command4(CASET, y>>8, y, y1>>8, y1);
  Command4(RASET, x>>8, x, x1>>8, x1);
  Command(RAMWR);
#if defined ( USE_DMA_FOR_M0 )  
  for (int i=0; i<w; i++) {
    for (int j=0; j<h; j++) {
      uint16_t p = pane->getScreenColor(i,j);
      // fill buffer
      dbuf[i & 1][j] = p>>8 | p<< 8; 
    }
    do_dma(i & 1, h*2);
  }
  while(theDMA.isActive()) ; // finish last transfer
#elif defined _SPI_WRITEBYTES_ 
  for (int i=0; i<w; i++) {
    for (int j=0; j<h; j++) {
      uint16_t p = pane->getScreenColor(i,j);
      // fill buffer
      _buf[j] = p>>8 | p<< 8; 
    }
    SPI.writeBytes((byte *)&_buf, h * 2);
  } 
#else
  for (int i=0; i<w; i++) {
    for (int j=0; j<h; j++) {
      uint16_t p = pane->getScreenColor(i,j);
      SPI.transfer(p>>8);
      SPI.transfer(p); 
    }
  } 
#endif
  digitalWrite(_CS, HIGH);
  SPI.endTransaction();
  pane->clear();
}

int fff_TFTSPI::width (void) { return _XSIZE; };
int fff_TFTSPI::height (void) { return _YSIZE; };
