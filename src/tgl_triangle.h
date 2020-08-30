// tgl_triangle.h

#ifndef __TGL_TRIANGLE_H
#define __TGL_TRIANGLE_H

#include "tgl.h"

#define ZCMP(z,zpix) ((z) >= (zpix))
//typedef unsigned long long uiST;

void ZB_setTexture(ZBuffer *zb, void *image);
void ZB_fillTriangleFlat(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2,ZBufferPoint *p3);
void ZB_fillTriangleSmooth(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2, ZBufferPoint *p3);
void ZB_fillTriangleMapping(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2, ZBufferPoint *p3); // unused
void ZB_fillTriangleMappingPerspective(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2);
typedef void (*ZB_fillTriangleFunc)(ZBuffer *, ZBufferPoint *, ZBufferPoint *, ZBufferPoint *);

#endif

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
