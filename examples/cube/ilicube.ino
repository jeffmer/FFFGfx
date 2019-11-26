/*=
The MIT License (MIT)
Copyright (c) 2017 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please note: I am spending a lot of my free time in developing Software and Hardware
for these projects. Please consider supporting me by
a) Buying my hardware kits from https://blog.squix.org/shop
b) Send a donation: https://www.paypal.me/squix/5USD
c) Or using this affiliate link while shopping: https://www.banggood.com/?p=6R31122484684201508S

See more at https://blog.squix.org

Demo for the buffered graphics library. Renders a 3D cube
*/

/* adapted to work with Fast Flicker Free Graphics j.n.magee 19-nov-2019 */

#include <fff_canvas.h>
#include <fff_TFTSPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <fff_colors.h>
#include <user_settings.h>



// defines the colors usable in the paletted 16 color frame buffer

const uint16_t palette[] = {
                      FFFGFX_BLACK, // 0
                      FFFGFX_WHITE, // 1
                       FFFGFX_NAVY, // 2
                      FFFGFX_DARKCYAN, // 3
                      FFFGFX_DARKGREEN, // 4
                      FFFGFX_MAROON, // 5
                      FFFGFX_PURPLE, // 6
                      FFFGFX_OLIVE, // 7
                      FFFGFX_LIGHTGREY, // 8
                      FFFGFX_DARKGREY, // 9
                      FFFGFX_BLUE, // 10
                      FFFGFX_GREEN, // 11
                      FFFGFX_CYAN, // 12
                      FFFGFX_MAGENTA, // 13
                      FFFGFX_RED, // 14
                      FFFGFX_YELLOW}; // 15
/*
const uint16_t palette[] = {
                      FFFGFX_BLACK, // 0
                      FFFGFX_BLUE, // 1
                      FFFGFX_GREEN,   // 2
                      FFFGFX_RED}; // 3
*/
const int ncubes = 1;
const int CANVAS_WIDTH = 200;
const int CANVAS_HEIGHT = 200;
const int SHAPE_SIZE = 500;

fff_TFTSPI screen;
Canvas canvas(CANVAS_WIDTH, CANVAS_HEIGHT, PIXELBITS4, palette);
Canvas text(screen.width(),20, PIXELBITS1, palette);

// Used for fps measuring
uint16_t counter = 0;
long startMillis = millis();
uint16_t interval = 20;

// size / 2 of cube edge
float d = 15;
float px[] = { -d,  d,  d, -d, -d,  d,  d, -d };
float py[] = { -d, -d,  d,  d, -d, -d,  d,  d };
float pz[] = { -d, -d, -d, -d,  d,  d,  d,  d };

// define the triangles
// The order of the vertices MUST be CCW or the
// shoelace method won't work to detect visible edges
int  faces[12][3] = {
    {0,1,4},
    {1,5,4},
    {1,2,5},
    {2,6,5},
    {5,7,4},
    {6,7,5},
    {3,4,7},
    {4,3,0},
    {0,3,1},
    {1,3,2},
    {2,3,6},
    {6,3,7}
  };

// mapped coordinates on screen
float p2x[] = {
  0,0,0,0,0,0,0,0};
float p2y[] = {
  0,0,0,0,0,0,0,0};

// rotation angle in radians
float r[] = {
  0,0,0};


// Define how fast the cube rotates. Smaller numbers are faster.
// This is the number of ms between draws.
#define ROTATION_SPEED 0
String fps = "0fps";

void setup() {
  screen.init();
  text.setFont(&FreeMonoBold9pt7b);
  startMillis = millis();
}



/**
* Detected visible triangles. If calculated area > 0 the triangle
* is rendered facing towards the viewer, since the vertices are CCW.
* If the area is negative the triangle is CW and thus facing away from us.
*/
int shoelace(int x1, int y1, int x2, int y2, int x3, int y3) {
  // (x1y2 - y1x2) + (x2y3 - y2x3)
  return x1 * y2 - y1 * x2 + x2*y3 - y2*x3 + x3*y1 - y3*x1;
}

/**
* Rotates and renders the cube.
**/
void drawCube()
{
  double speed = 90;
  r[0]=r[0]+PI/speed; // Add a degree
  r[1]=r[1]+PI/speed; // Add a degree
  r[2]=r[2]+PI/speed; // Add a degree
  if (r[0] >= 360.0*PI/90.0) r[0] = 0;
  if (r[1] >= 360.0*PI/90.0) r[1] = 0;
  if (r[2] >= 360.0*PI/90.0) r[2] = 0;

  float ax[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  float ay[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  float az[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  // Calculate all vertices of the cube
  for (int i=0;i<8;i++)
  {
    float px2 = px[i];
    float py2 = cos(r[0])*py[i] - sin(r[0])*pz[i];
    float pz2 = sin(r[0])*py[i] + cos(r[0])*pz[i];

    float px3 = cos(r[1])*px2 + sin(r[1])*pz2;
    float py3 = py2;
    float pz3 = -sin(r[1])*px2 + cos(r[1])*pz2;

    ax[i] = cos(r[2])*px3 - sin(r[2])*py3;
    ay[i] = sin(r[2])*px3 + cos(r[2])*py3;
    az[i] = pz3-150;

    p2x[i] = CANVAS_WIDTH/2+ax[i]*SHAPE_SIZE/az[i];
    p2y[i] = CANVAS_HEIGHT/2+ay[i]*SHAPE_SIZE/az[i];
  }

  for (int i = 0; i < 12; i++) {

    if (shoelace(p2x[faces[i][0]],p2y[faces[i][0]],p2x[faces[i][1]],p2y[faces[i][1]],p2x[faces[i][2]],p2y[faces[i][2]]) > 0) {
      canvas.setColor((i/2) + 10);
      canvas.drawFilledTriangle(p2x[faces[i][0]],p2y[faces[i][0]],p2x[faces[i][1]],p2y[faces[i][1]],p2x[faces[i][2]],p2y[faces[i][2]]);
      if (i % 2) {
        int avX = 0;
        int avY = 0;
        for (int v = 0; v < 3; v++) {
          avX += p2x[faces[i][v]];
          avY += p2y[faces[i][v]];
        }
        avX = avX / 3;
        avY = avY / 3;
      }
    }
  }
 screen.paint((screen.width()-CANVAS_WIDTH)/2,(screen.height()-CANVAS_HEIGHT)/2 - 10,&canvas);
}

void loop() {

  drawCube();

  counter++;
  // only calculate the fps every <interval> iterations.
  if (counter % interval == 0) {
    long millisSinceUpdate = millis() - startMillis;
    fps = String(interval * 1000.0 / (millisSinceUpdate)) + "fps " + _FFF_CPU_NAME;
    startMillis = millis();
    text.setColor(1);
    text.setXY(10,12);
    text.print(fps);
    screen.paint(4,screen.height()-21,&text);
  }
}
