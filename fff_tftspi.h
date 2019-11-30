#ifndef _FFF_TFTSPI_H
#define _FFF_TFTSPI_H

#include "fff_canvas.h"

class fff_TFTSPI {
    public:
        void init(uint16_t background = 0);
        void on(void);
        void off(void);
        void clear(uint16_t background = 0);
        void paint(int x, int y, Canvas *pane);
        int height(void);
        int width(void);
};

#endif
