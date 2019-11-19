// Display dimensions - uncomment the one you want to use

// Adafruit 0.96" 80x160 display
//int const _YSIZE = 80, _XSIZE = 160, _YOFF = 24, _XOFF = 0, _INVERT = 0, _ROTATE = 0;

//AliExpress 0.96" 80x160 display
//static int const _YSIZE = 80, _XSIZE = 160, _YOFF = 26, _XOFF = 1, _INVERT = 1, _ROTATE = 2;

// Adafruit 1.44" 128x128 display
//static int const _YSIZE = 128, _XSIZE = 128, _YOFF = 3, _XOFF = 2, _INVERT = 0, _ROTATE = 5;

// Adafruit 1.8" 128x160 display
static int const _YSIZE = 128, _XSIZE = 160, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 2;

// Adafruit 1.54" 240x240 display
//static int const _YSIZE = 240, _XSIZE = 240, _YOFF = 0, _XOFF = 0, _INVERT = 1, _ROTATE = 3;

// SPI pins  uChip is 2, 7, 3  Feather is 12, 10, 11

static int const _DC = 2;    // 12;
static int const _CS = 7;    // 10;
static int const _RESET = 3; //11;

// comment out the following definition if DMA not used

#define USE_DMA_FOR_M0

#define USE_SERCOM SERCOM1              //UChip, more usually SERCOM4 for M0
#define USE_TRIGGER SERCOM1_DMAC_ID_TX  //UChip, more usually SERCOM4_DMAC_ID_TX
