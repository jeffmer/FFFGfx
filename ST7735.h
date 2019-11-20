#ifndef _ST7735_H
#define _ST7735_H

#include "canbas.h"

class ST7735 {
    public:
        void init(void);
        void on(void);
        void off(void);
        void clear(void);
        void paint(int x, int y, Canvas pane);
};

#endif
