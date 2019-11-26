#ifndef _USER_SETTINGS_H
#define _USER_SETTINGS_H

// Displays - uncomment the one you want to use

#define IS_ILI9341
//#define IS_ST7735


#ifdef IS_ST7735
// Display dimensions - uncomment the one you want to use
// Adafruit 0.96" 80x160 display
//int const _YSIZE = 80, _XSIZE = 160, _YOFF = 24, _XOFF = 0, _INVERT = 0, _ROTATE = 0;

//AliExpress 0.96" 80x160 display
//static int const _YSIZE = 80, _XSIZE = 160, _YOFF = 26, _XOFF = 1, _INVERT = 1, _ROTATE = 2;

// Adafruit 1.44" 128x128 display
//static int const _YSIZE = 128, _XSIZE = 128, _YOFF = 3, _XOFF = 2, _INVERT = 0, _ROTATE = 5;

// Adafruit 1.8" 128x160 display LANDSCAPE
static int const _YSIZE = 128, _XSIZE = 160, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 2;

// Adafruit 1.8" 128x160 display PORTRAIT
//static int const _YSIZE = 160, _XSIZE = 128, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 1;

// Adafruit 1.54" 240x240 display
//static int const _YSIZE = 240, _XSIZE = 240, _YOFF = 0, _XOFF = 0, _INVERT = 1, _ROTATE = 3;
#endif

#ifdef IS_ILI9341

//ILI9341 2.2" 240X320 DISPLAY PORTRAIT
//static int const _YSIZE = 320, _XSIZE = 240, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 3;

//ILI9341 2.2" 240X320 DISPLAY LANDSCAPE
static int const _YSIZE = 240, _XSIZE = 320, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 0;

//ILI9341 MSTACK 240X320 DISPLAY LANDSCAPE
//static int const _YSIZE = 240, _XSIZE = 320, _YOFF = 0, _XOFF = 0, _INVERT = 0, _ROTATE = 1;
#endif

//processor board definitions - select one
//#define _ATMEGA1284_
//#define _UCHIP_
#define _FEATHER_M0_
//#define _FEATHER_M4_
//#define _NODEMCU_ESP8266_
//#define _M5STACK_
//#define _TTGO_ESP32_WROVER_

#ifdef _ATMEGA1284_
    // Pins for SPI
    #define _DC 16
    #define _CS 15
#endif

#ifdef _UCHIP_
    #define _FFF_CPU_NAME "M0 (dma)"
    // SPI control pins
    #define _DC 2
    #define _CS 7
    #define _RESET 3
    // SERCOM for DMA to SPI
    #define USE_DMA_FOR_M0
    #define USE_SERCOM SERCOM1             
    #define USE_TRIGGER SERCOM1_DMAC_ID_TX  
#endif

#ifdef _FEATHER_M0_
    #define _FFF_CPU_NAME "M0 (dma)"
    // SPI control pins
    #define _DC 10
    #define _CS 9
    // SERCOM for DMA to SPI
    #define USE_DMA_FOR_M0
    #define USE_SERCOM SERCOM4              
    #define USE_TRIGGER SERCOM4_DMAC_ID_TX  
#endif

#ifdef _FEATHER_M4_
    #define _FFF_CPU_NAME "M4 (dma)"
    // SPI control pins
    #define _DC 10
    #define _CS 9
    // SERCOM for DMA to SPI
    #define USE_DMA_FOR_M0
    #define USE_SERCOM SERCOM1              
    #define USE_TRIGGER SERCOM1_DMAC_ID_TX  
#endif


#ifdef _NODEMCU_ESP8266_
    #define _FFF_CPU_NAME "ESP8266 (160mhz)"
    // Pins for SPI
    #define _DC D4
    #define _CS D1
    #define _RESET D0
    #define _SPI_WRITEBYTES_
#endif

#ifdef _M5STACK_
    #define _FFF_CPU_NAME "ESP32 MStack"
    // Pins for SPI
    #define _DC 27
    #define _CS 14
    #define _RESET 33
    #define _LED_PIN 32
    #define _SPI_WRITEBYTES_
#endif

#ifdef _TTGO_ESP32_WROVER_
    #define _FFF_CPU_NAME "ESP32 WROVER"
    // Pins for SPI
    #define _DC 27
    #define _CS 14
    #define _SPI_WRITEBYTES_
#endif

#endif
