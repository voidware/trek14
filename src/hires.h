
#ifndef __hires_h__
#define __hires_h__

#include "defs.h"

#define HIRES_X         0x80
#define HIRES_Y         0x81
#define HIRES_DATA      0x82
#define HIRES_CTL       0x83

#define HIRES_SCROLL_X  0x8c
#define HIRES_SCROLL_Y  0x8d


uchar checkgfx();
void hrOn();
void hrOff();
void hrCls(uchar c);
void hrPlot(int x, uchar y, uchar c);
void hrLine(int x1, uchar y1, int x2, uchar y2, uchar c);

#endif 
