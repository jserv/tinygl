// tgl_line.h

#ifndef __TGL_LINE_H
#define __TGL_LINE_H

#include "tgl.h"

void ZB_plot(ZBuffer *zb,ZBufferPoint *p);
void ZB_line(ZBuffer *zb,ZBufferPoint *p1,ZBufferPoint *p2);
void ZB_line_z(ZBuffer * zb, ZBufferPoint * p1, ZBufferPoint * p2);

#endif // __TGL_LINE_H
