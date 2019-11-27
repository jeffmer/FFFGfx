#include "fff_canvas.h"
#include "fff_TFTSPI.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include "fff_colors.h"
#include "user_settings.h"

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

const int CANVAS_WIDTH = 144;
const int CANVAS_HEIGHT = 144;

fff_TFTSPI screen;
Canvas canvas1(CANVAS_WIDTH, CANVAS_HEIGHT, PIXELBITS1, palette);
Canvas canvas2(CANVAS_WIDTH, CANVAS_HEIGHT, PIXELBITS4, palette);
Canvas text(screen.width(),20, PIXELBITS1, palette);

// Used for fps measuring
uint16_t counter = 0;
long startMillis = millis();
uint16_t interval = 20;

float unitcube[8][3] = {
    {-1, -1, -1},
    { 1, -1, -1},
    { 1,  1, -1},
    {-1,  1, -1},
    {-1, -1,  1},
    { 1, -1,  1},
    { 1,  1,  1},
    {-1,  1,  1}};
    
const int edges[12][2]= 
    {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    
// The order of the triangle vertices must be CCW for shoelace method

const int  faces[12][3] = {
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
    

void scale(float s[3], float input[8][3], float output[8][3]){
    for (int i= 0; i < 8; i++) 
        for (int j= 0; j < 3; j++) output[i][j] = s[j] * input[i][j];
}

void rotateX(float theta, float input[8][3], float output[8][3]) {
   float sinTheta = sin(theta);
   float cosTheta = cos(theta);
   for (int i= 0; i < 8; i++) {
      float y = input[i][1];
      float z = input[i][2];
      output[i][0] = input[i][0];
      output[i][1] = y * cosTheta - z * sinTheta;
      output[i][2] = z * cosTheta + y * sinTheta;
   }
}

void rotateY(float theta,  float input[8][3], float output[8][3]) {
   float sinTheta = sin(theta);
   float cosTheta = cos(theta);
   for (int i= 0; i < 8; i++) {
      float x = input[i][0];
      float z = input[i][2];
      output[i][0] = x * cosTheta + z * sinTheta;
      output[i][1] = input[i][1];
      output[i][2] = z * cosTheta - x * sinTheta;
   }
}
    
void rotateZ(float theta, float input[8][3], float output[8][3]) {
   float sinTheta = sin(theta);
   float cosTheta = cos(theta);
   for (int i= 0; i < 8; i++) {
      float x = input[i][0];
      float y = input[i][1];
      output[i][0] = x * cosTheta - y * sinTheta;
      output[i][1] = y * cosTheta + x * sinTheta;
      output[i][2] = input[i][2];
   }
}

void rotate(float theta, float input[8][3], float output[8][3]) {
  rotateX(theta,input,output);
  rotateY(theta/2.0,output,output);
  rotateZ(theta,output,output); 
}

void drawCube(Canvas *c, float input[8][3]) {
    c->setColor(1);
    int cx = c->getWidth()/2; int cy = c->getHeight()/2; //draw cube in centre of canvas
    for (int i = 0; i<12; i++) {
        int p1 = edges[i][0];
        int p2 = edges[i][1];
        int x1 = cx + (int) input[p1][0];
        int y1 = cy + (int) input[p1][1];
        int x2 = cx + (int) input[p2][0];
        int y2 = cy + (int) input[p2][1];
        c->drawLine(x1,y1,x2,y2);
    }
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
void drawRenderedCube(Canvas *c, float input[8][3]) {
    c->setColor(1);
    int cx = c->getWidth()/2; int cy = c->getHeight()/2; //draw cube in centre of canvas
    for (int i = 0; i<12; i++) {
        int p1 = faces[i][0];
        int p2 = faces[i][1];
        int p3 = faces[i][2];
        int x1 = cx + (int) input[p1][0];
        int y1 = cy + (int) input[p1][1];
        int x2 = cx + (int) input[p2][0];
        int y2 = cy + (int) input[p2][1];
        int x3 = cx + (int) input[p3][0];
        int y3 = cy + (int) input[p3][1];
        c->setColor((i / 2) + 10);
        if (shoelace(x1,y1,x2,y2,x3,y3) > 1)
            c->drawFilledTriangle(x1,y1,x2,y2,x3,y3);
    }
}

float initCube[8][3];
float rotatedCube[8][3];
float sv[3] = {40,40,40};

void setup() {
  screen.init();
  text.setFont(&FreeMonoBold9pt7b);
  canvas1.setColor(1);
  startMillis = millis();
  scale(sv,unitcube,initCube);
}

float const incr = PI/90; // 2 degrees per interation
float r = 0;

char pbuf[10];

void loop() {
  r += incr;
  if (r >= 4 * PI) {
     r = 0; //reset after two revolutions.
     if (sv[2]>39) sv[2] = 5.0; else sv[2] = 40.0;
     scale(sv,unitcube,initCube);
  }    
  rotate(r, initCube, rotatedCube);
  drawCube(&canvas1, rotatedCube);
  drawRenderedCube(&canvas2, rotatedCube);
  screen.paint(8, (screen.height()-CANVAS_HEIGHT)/2 - 10,&canvas1);
  screen.paint(16+CANVAS_WIDTH, (screen.height()-CANVAS_HEIGHT)/2 - 10,&canvas2);
  counter++;
  // only calculate the fps every <interval> iterations.
  if (counter % interval == 0) {
    text.setColor(1);
    text.setXY(10,12);
    long millisSinceUpdate = millis() - startMillis;     
    startMillis = millis();
    text.print(dtostrf(interval * 1000.0 / (millisSinceUpdate),4,1,pbuf ));
    text.print(" fps ");
    text.print(_FFF_CPU_NAME);   
    screen.paint(4,screen.height()-21,&text);
  }
}
 
