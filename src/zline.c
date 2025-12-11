#include <stdlib.h>
#include "zbuffer.h"

#define ZCMP(z, zpix) (!(zbdt) || z >= (zpix))

/* Dirty rectangle helper macros */
#if TGL_HAS(DIRTY_RECTANGLE)
#define TGL_MIN2(a, b) (((a) < (b)) ? (a) : (b))
#define TGL_MAX2(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* TODO: Implement point size. */
/* TODO: Implement blending for lines and points. */

void ZB_plot(ZBuffer *zb, ZBufferPoint *p)
{
    GLint zz, y, x;
    GLubyte zbdw = zb->depth_write;
    GLubyte zbdt = zb->depth_test;
    GLfloat zbps = zb->pointsize;

#if TGL_HAS(DIRTY_RECTANGLE)
    /* Mark dirty region for point (may have point size) */
    if (zbps <= 1.0f) {
        ZB_markDirty(zb, p->x, p->y, p->x, p->y);
    } else {
        GLfloat hzbps = zbps / 2.0f;
        GLint xmin = (GLint) ((GLfloat) p->x - hzbps);
        GLint xmax = (GLint) ((GLfloat) p->x + hzbps);
        GLint ymin = (GLint) ((GLfloat) p->y - hzbps);
        GLint ymax = (GLint) ((GLfloat) p->y + hzbps);
        ZB_markDirty(zb, xmin, ymin, xmax, ymax);
    }
#endif
    TGL_BLEND_VARS
    zz = p->z >> ZB_POINT_Z_FRAC_BITS;

    if (zbps == 1) {
        GLushort *pz;
        PIXEL *pp;
        pz = zb->zbuf + (p->y * zb->xsize + p->x);
        pp =
            (PIXEL *) ((GLbyte *) zb->pbuf + zb->linesize * p->y + p->x * PSZB);

        if (ZCMP(zz, *pz)) {
#if TGL_HAS(BLEND)
            if (!zb->enable_blend)
                *pp = RGB_TO_PIXEL(p->r, p->g, p->b);
            else
                TGL_BLEND_FUNC_RGB(p->r, p->g, p->b, (*pp))
#else
            *pp = RGB_TO_PIXEL(p->r, p->g, p->b);
#endif
            if (zbdw)
                *pz = zz;
        }
    } else {
        PIXEL col = RGB_TO_PIXEL(p->r, p->g, p->b);
        GLfloat hzbps = zbps / 2.0f;
        GLint bx = (GLfloat) p->x - hzbps;
        GLint ex = (GLfloat) p->x + hzbps;
        GLint by = (GLfloat) p->y - hzbps;
        GLint ey = (GLfloat) p->y + hzbps;
        bx = (bx < 0) ? 0 : bx;
        by = (by < 0) ? 0 : by;
        ex = (ex > zb->xsize) ? zb->xsize : ex;
        ey = (ey > zb->ysize) ? zb->ysize : ey;
        for (y = by; y < ey; y++)
            for (x = bx; x < ex; x++) {
                GLushort *pz = zb->zbuf + (y * zb->xsize + x);
                PIXEL *pp = (PIXEL *) ((GLbyte *) zb->pbuf + zb->linesize * y +
                                       x * PSZB);

                if (ZCMP(zz, *pz)) {
#if TGL_HAS(BLEND)
                    if (!zb->enable_blend)
                        *pp = col;
                    else
                        TGL_BLEND_FUNC_RGB(p->r, p->g, p->b, (*pp))
#else
                    *pp = col;
#endif
                    if (zbdw)
                        *pz = zz;
                }
            }
    }
}

#define INTERP_Z
static void ZB_line_flat_z(ZBuffer *zb,
                           ZBufferPoint *p1,
                           ZBufferPoint *p2,
                           GLint color)
{
    GLubyte zbdt = zb->depth_test;
    GLubyte zbdw = zb->depth_write;
#include "zline.h"
}

/* line with color GLinterpolation */
#define INTERP_Z
#define INTERP_RGB
static void ZB_line_interp_z(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2)
{
    GLubyte zbdt = zb->depth_test;
    GLubyte zbdw = zb->depth_write;
#include "zline.h"
}

/* no Z GLinterpolation */

static void ZB_line_flat(ZBuffer *zb,
                         ZBufferPoint *p1,
                         ZBufferPoint *p2,
                         GLint color)
{
#include "zline.h"
}

#define INTERP_RGB
static void ZB_line_interp(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2)
{
#include "zline.h"
}

void ZB_line_z(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2)
{
    GLint color1, color2;

#if TGL_HAS(DIRTY_RECTANGLE)
    /* Mark dirty region for line bounding box */
    GLint xmin = TGL_MIN2(p1->x, p2->x);
    GLint xmax = TGL_MAX2(p1->x, p2->x);
    GLint ymin = TGL_MIN2(p1->y, p2->y);
    GLint ymax = TGL_MAX2(p1->y, p2->y);
    ZB_markDirty(zb, xmin, ymin, xmax, ymax);
#endif

    color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
    color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

    /* choose if the line should have its color GLinterpolated or not */
    if (color1 == color2) {
        ZB_line_flat_z(zb, p1, p2, color1);
    } else {
        ZB_line_interp_z(zb, p1, p2);
    }
}

void ZB_line(ZBuffer *zb, ZBufferPoint *p1, ZBufferPoint *p2)
{
    GLint color1, color2;

#if TGL_HAS(DIRTY_RECTANGLE)
    /* Mark dirty region for line bounding box */
    GLint xmin = TGL_MIN2(p1->x, p2->x);
    GLint xmax = TGL_MAX2(p1->x, p2->x);
    GLint ymin = TGL_MIN2(p1->y, p2->y);
    GLint ymax = TGL_MAX2(p1->y, p2->y);
    ZB_markDirty(zb, xmin, ymin, xmax, ymax);
#endif

    color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
    color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

    /* choose if the line should have its color GLinterpolated or not */
    if (color1 == color2) {
        ZB_line_flat(zb, p1, p2, color1);
    } else {
        ZB_line_interp(zb, p1, p2);
    }
}
