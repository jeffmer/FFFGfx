#FFFGfx - Fast Flicker Free Graphics

A graphics library targeted at M0 based Arduino systems such as the Arduino Zero, Adafruit Feather M0 and uChip. Currently supports ST7735 based displays (will support ILI9341).

[Two Cubes](cubes2dma.gif)

###Flicker

Typically Arduino graphics libraries write pixels directly to the display memories of colour ST7735 or ILI9341 based TFT displays. Consequently, when the display is updated, it is necessary to clear the screen before drawing the next one leading to the characteristic flicker.

The classic graphics technique to avoid this flicker is to use a framebuffer in main memory such that pixels are set in this buffer and then the framebuffer is written to the screen in one update operation - thus erasing the old screen and drawing the new one at the same time with no intermediate state of a cleared screen visible - the flicker.

However, each pixel in a colour display is defined by a 16 bit value meaning that a framebuffer for a 160x128 pixel display would require 40960 bytes which is larger than the 32K bytes of RAM available on the M0 based boards.

###Colour Map

To reduce the storage required to store the framebuffer, again, the classic graphics technique is to use a colour map such that each pixel in the framebuffer is described by a small number of bits and this value is used to index the colour map which is a table used to translate the framebuffer pixel values to the 16 bit screen values. For example, if we use a four bit value to define the colour of a pixel in memory, then this can be mapped to one of sixteen 16 bit screen colours. This technique is adopted by the [Minigfx](https://github.com/ThingPulse/minigrafx) library from which I have adapted the rotating cube example. However, the framebuffer can still take a large amount of the available memory. A 4 bit framebuffer for the 160x128 screen requires 10240 bytes, a third of the available memory. For a 320x240 ILI9341 display such as that use for the Weather Station this would be 38400 bytes.

###Canvas

The solution adopted in FFFGfx is to allow multiple smaller framebuffers. Each framebuffer can have a different number of bits per pixel and  can have its own colour map or share an existing map. Each framebuffer can be written to a different part of the screen and they can be overlaid. The `Canvas` class implements this idea. For example the display above has an 80 by 80 `Canvas` for each rotating cube and a 120 by 30 `Canvas` for the text. These are declared as shown below:

```
Canvas canvas(80, 80, PIXELBITS4, palette);
Canvas canvas2(80, 80, PIXELBITS4, palette);
Canvas text(160,20, PIXELBITS1, palette);
```
The two cube canvases use 4 bits per pixel and the text canvas uses 1 bit per pixel since it only needs the colours black and white. These are the first two colours defined in the shared colour map `palette`. The RAM required for all three framebuffers is:

```
(80 * 80)/2 + (80 * 80)/2 + (120 * 20)/8 = 3200 + 3200 + 300
                                         = 6700 bytes
```
The `Canvas` class includes the usual familiar drawing operations from the [Adafruit GFX LIbrary](https://github.com/adafruit/Adafruit-GFX-Library) library. The code from the example which draws to the `text` canvas is shown below:

```
 counter++;
  // only calculate the fps every <interval> iterations.
  if (counter % interval == 0) {
    long millisSinceUpdate = millis() - startMillis;
    fps = String(interval * 1000.0 / (millisSinceUpdate)) + "fps (dma)";
    startMillis = millis();
    text.setColor(1);  // White
    text.setXY(10,12);
    text.print(fps);
    screen.paint(0,100,text); // draws canvas on the screen
  }
```

###Screen update speed

From the code above, note that while the cube drawing canvases are updated every loop iteration, the `text` canvas is only updated every `interval` iterations. Canvases thus not only permit different parts of the display to have different colour maps, they also allow different update rates for different elements of the overall display. Consequently, we can achieve fast frame update rates on those parts of the display that need it. For example, if we only have one cube canvas, we can double the frame rate - see below:

[one cube](cubes1dma.gif)


###DMA - Direct Memory Access






	